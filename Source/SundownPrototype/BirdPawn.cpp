// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#include "BirdPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"

// Sets default values
ABirdPawn::ABirdPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> BirdMesh;
		FConstructorStatics()
			: BirdMesh(TEXT("/Game/TheBird/TSTANIMBAKE.TSTANIMBAKE"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	mBoundSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Movement Sphere"));
	mBoundSphere->SetupAttachment(RootComponent);

	 // Create a particle system
	FireParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire Particles"));
	FireParticleSystem->SetupAttachment(RootComponent);
	FireParticleSystem->bAutoActivate = true;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (FireParticleAsset.Succeeded())
	{
		FireParticleSystem->SetTemplate(FireParticleAsset.Object);
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("mCameraSpringArm"));
	mCameraSpringArm->SetupAttachment(RootComponent);
	mCameraSpringArm->TargetArmLength = 2400.0f; // The camera follows at this distance behind the character	
	mCameraSpringArm->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	//// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set handling parameters
	Acceleration = 300.0f;
	TurnSpeed = 15.f;
	MaxSpeed = 2000.0f;
	MinSpeed = 1000.0f;
	CurrentForwardSpeed = 1000.0f;
	SplineDistance = 0.0f;
}

void ABirdPawn::BeginPlay()
{
	SplinePtr = SplineActor->GetComponentByClass(USplineComponent::StaticClass());
	Spline = Cast<USplineComponent>(SplinePtr);
	
	/*if (Spline)
	{
		SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(10000.0f, ESplineCoordinateSpace::World));
	}*/

	Super::BeginPlay();
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	// SPLINE MOVEMENT --------------------------------------------------
	SplineDistance = SplineDistance + SplineSpeed; // Increment distance along spline

	if (Spline) // Check to make sure reference is valid
	{
		if (SplineDistance < Spline->GetDistanceAlongSplineAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1))
		{
			mBoundSphere->SetWorldLocation(Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World));
			SetActorLocation(mBoundSphere->GetComponentLocation());
			//SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World)); // Set location to location at distance along spline
			//SetActorRotation(Spline->GetRotationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World)); // Aaaand rotation to rotation at distance along spline
		}
	}

	// SIDEWAYS MOVEMENT (WHILST ON SPLINE)

	//const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move bird forwards
	//AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation
	FRotator DeltaRotation(0, 0, 0);				// New rotation for updating rotation		
	FRotator InterpRotation;						// New rotation for interpolating to
	CurrentRotation = GetActorRotation();

	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds; // Update pitch
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;		// Update yaw
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;	// Update roll

	// Rotate bird
	AddActorLocalRotation(DeltaRotation);
	// Update camera
	CameraTick();
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ABirdPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
}

void ABirdPawn::CameraTick()
{
	// Update location first
	// CamMoveX will become the new camera X location (this is how zoomed into the character you are, pitch used for weighting)
	if (CurrentRotation.Pitch < 0.0f) {
		CamMoveX = FMath::FInterpTo(CameraLoc.X, CurrentRotation.Pitch*-10, GetWorld()->GetDeltaSeconds(), 1.0f);
	}

	// CamMoveY will become the new camera Y location (in the direction of Y = 0)
	if (CameraLoc.Y != 0.0f) {
		CamMoveY = CameraLoc.Y;
		CamMoveY = FMath::FInterpTo(CamMoveY, 0.0f, GetWorld()->GetDeltaSeconds(), 0.666f); // Interpolate for smooth camera movement
	}

	CameraRot = FRotator(0.0f, 0.0f, 0.0f);				// Camera rotation
	CameraLoc = FVector(CamMoveX, CamMoveY, 200.0f);	// Camera location
	CameraSca = FVector(1.0f, 1.0f, 1.0f);				// Camera scale

	// Setup transform to desired rotation, location, scale 
	FTransform CameraTransform(CameraRot, CameraLoc, CameraSca); 
	// Transform the camera
	mCamera->SetRelativeTransform(CameraTransform);
}

// Called to bind functionality to input
void ABirdPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &ABirdPawn::ThrustInput);
	PlayerInputComponent->BindAxis("MoveUp", this, &ABirdPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABirdPawn::MoveRightInput);
}

void ABirdPawn::ThrustInput(float Val)
{
	// Acceleration force
	float fAcc;
	if (CurrentForwardSpeed < 500.0)
	{
		fAcc = 0.5;
	}
	else {
		fAcc = -0.5f;
	}
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (fAcc * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ABirdPawn::MoveUpInput(float Val)
{
	CameraLoc = FVector(FMath::Clamp(CameraLoc.X + Val * 2.0f, 0.0f, 500.0f), CameraLoc.Y, CameraLoc.Z);

	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * 1.5f);

	// When steering, we decrease pitch slightly
	//TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.01f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ABirdPawn::MoveRightInput(float Val)
{
	CameraLoc = FVector(CameraLoc.X, FMath::Clamp(CameraLoc.Y + Val * 3.5f, -300.0f, 300.0f), CameraLoc.Z);

	float TargetYawSpeed;

	// Target yaw speed is based on input
	TargetYawSpeed = (Val * TurnSpeed * 2);

	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If turning, yaw value is used to influence roll
	// If not turning, roll to reverse current roll value.
	float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 1.0f) : (GetActorRotation().Roll * -2.f);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}