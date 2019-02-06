// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#include "BirdPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Vector.h"

// Sets default values
ABirdPawn::ABirdPawn()
{
	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	 // Create a particle system
	FireParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire Particles"));
	FireParticleSystem->SetupAttachment(RootComponent);
	FireParticleSystem->bAutoActivate = true;
	FireParticleSystem->SetRelativeLocation(FVector(0.0f, 0.0f, 00.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (FireParticleAsset.Succeeded())
	{
		FireParticleSystem->SetTemplate(FireParticleAsset.Object);
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("mCameraSpringArm"));
	mCameraSpringArm->SetupAttachment(RootComponent);
	mCameraSpringArm->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	mCameraSpringArm->bEnableCameraLag = false;
	mCameraSpringArm->CameraLagSpeed = CamLag;

	//// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create the collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);	

	// Set handling parameters
	Acceleration = 500.0f;
	TurnSpeed = 45.0f;
	MaxSpeed = 6000.0f;
	MinSpeed = 3000.0f;
	CurrentForwardSpeed = 3000.0f;
	SplineDistance = 0.0f;
}

void ABirdPawn::BeginPlay()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), SplineClassType, Splines);
	Spline = Cast<USplineComponent>(Splines[0]->GetComponentByClass(USplineComponent::StaticClass()));

	if (Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline found!"));
		SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World));
	}
	
	Super::BeginPlay();
}

//void ABirdPawn::Collide()
//{
//	float weighting = 0;
//
//	CollisionSphere->GetOverlappingActors(OverlappingActors);
//	float push = 0.0f;
//
//	if (OverlappingActors.Num() > 0) {
//		UE_LOG(LogTemp, Warning, TEXT("Overlap!"));
//		for (int i = 0; i < OverlappingActors.Num(); i++) {
//			FVector normalVec = OverlappingActors[i]->GetActorLocation - GetActorLocation();
//			normalVec.GetSafeNormal(1.0f);
//
//			// Deflect along the surface when we collide.
//			FRotator CurrentRotation = GetActorRotation();
//			SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), normalVec.ToOrientationQuat(), 0.025f));
//		}
//	}
//
//}

void ABirdPawn::Tick(float DeltaSeconds)
{
	// SPLINE MOVEMENT --------------------------------------------------
	SplineDistance = SplineDistance + SplineSpeed; // Increment distance along spline

	if (Spline) // Check to make sure reference is valid
	{
		if (SplineDistance < Spline->GetDistanceAlongSplineAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1))
		{
			//mBoundSphere->SetWorldLocation(Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World));
			//SetActorLocation(mBoundSphere->GetComponentLocation());
			SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World)); // Set location to location at distance along spline
			SetActorRotation(Spline->GetRotationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World)); // Aaaand rotation to rotation at distance along spline
		}
	}

	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
	
	// Move bird forwards
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation
	FRotator DeltaRotation(0, 0, 0);				// New rotation for updating rotation		

	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds; // Update pitch
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds; // Update yaw
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds; //Update roll

	// Rotate bird
	AddActorLocalRotation(DeltaRotation);
	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ABirdPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit!"));
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
}

void ABirdPawn::NotifyActorBeginOverlap(class AActor* other) 
{
	CurrentForwardSpeed = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("Overlap!"));
	Super::NotifyActorBeginOverlap(other);
	
	FVector normalVec = (other->GetActorLocation() - this->GetActorLocation());
	normalVec.Normalize();
	
	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), normalVec.ToOrientationQuat(), 0.025f));
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
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ABirdPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ABirdPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetYawSpeed = (Val * TurnSpeed);

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
