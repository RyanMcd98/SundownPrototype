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

	 // Create a particle system
	FireParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire Particles"));
	FireParticleSystem->SetupAttachment(RootComponent);
	FireParticleSystem->bAutoActivate = true;
	FireParticleSystem->SetRelativeLocation(FVector(-250.0f, 0.0f, 20.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (FireParticleAsset.Succeeded())
	{
		FireParticleSystem->SetTemplate(FireParticleAsset.Object);
	}

	// Create a particle system
	FairyParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fairy Particles"));
	FairyParticleSystem->SetupAttachment(RootComponent);
	FairyParticleSystem->bAutoActivate = true;
	FairyParticleSystem->SetRelativeLocation(FVector(-250.0f, 250.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FairyParticleAsset(TEXT("/Game/Fairy/FairyParticle.FairyParticle"));
	if (FairyParticleAsset.Succeeded())
	{
		FairyParticleSystem->SetTemplate(FairyParticleAsset.Object);
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
	Acceleration = 250.0f;
	TurnSpeed = 25.f;
	MaxSpeed = 3000.f;
	MinSpeed = 1000.0f;
	CurrentForwardSpeed = 1500.f;
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
	
	// Move bird forwards
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation
	FRotator DeltaRotation(0, 0, 0);

	FRotator CurrentRotation = GetActorRotation();	// Current rotation of bird
	FRotator MaxRotation;							// New rotation for updating rotation

	// Check if bird is flying too close too steeply or at obtuse angle downwards
	if (CurrentRotation.Pitch < 80.0f && CurrentRotation.Pitch > -85.0f) {
		DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	}
	if (CurrentRotation.Pitch > 0.0f) {
		MaxRotation = CurrentRotation;
		MaxRotation.Pitch = 0.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, MaxRotation, GetWorld()->GetDeltaSeconds(), 0.666f)); // Interpolate to 
	}
	if (CurrentRotation.Pitch < 0.0f) {
		MaxRotation = CurrentRotation;
		MaxRotation.Pitch = 0.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, MaxRotation, GetWorld()->GetDeltaSeconds(), 0.666f));
	}

	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds; // Update yaw

	// Check if bird is banking too steeply
	if (CurrentRotation.Roll < 60.0f && CurrentRotation.Roll > -60.0f) {
		DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;
	}
	if (CurrentRotation.Roll > 30.0f) {
		MaxRotation = CurrentRotation;
		MaxRotation.Roll = 30.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, MaxRotation, GetWorld()->GetDeltaSeconds(), 2.0f));
	}
	if (CurrentRotation.Roll < -30.0f) {
		MaxRotation = CurrentRotation;
		MaxRotation.Roll = -30.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, MaxRotation, GetWorld()->GetDeltaSeconds(), 2.0f));
	}

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
	// CenterY will become the new camera Y location (in the direction of Y = 0)
	float CenterY;

	if (CameraLoc.Y > 5.0) { 
		CenterY = CameraLoc.Y - 5.0f; // 5 closer to Y = 0
	}
	else if (CameraLoc.Y < -5.0) {
		CenterY = CameraLoc.Y + 5.0f; // -5 closer to Y = 0
	}

	CameraLoc = FVector(-500.0f, CenterY, 600.0f);		// Camera location
	CameraRot = FRotator(0.0f, 0.0f, 0.0f);				// Camera rotation
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
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * 0.5f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * 0.5);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ABirdPawn::MoveRightInput(float Val)
{
	CameraLoc = FVector(CameraLoc.X, FMath::Clamp(CameraLoc.Y + Val * 7.5f, -500.0f, 500.0f), CameraLoc.Z);

	float TargetYawSpeed;

	// Target yaw speed is based on input
	TargetYawSpeed = (Val * TurnSpeed);

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