// Fill out your copyright notice in the Description page of Project Settings.

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
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f; 

	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("mCameraSpringArm"));
	mCameraSpringArm->SetupAttachment(RootComponent);
	mCameraSpringArm->TargetArmLength = 1200.0f; // The camera follows at this distance behind the character	
	mCameraSpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
	mCameraSpringArm->bEnableCameraLag = false; // Do not allow camera to lag
	mCameraSpringArm->CameraLagSpeed = 15.0f;
	mCameraSpringArm->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set handling parameters
	Acceleration = 500.0f;
	TurnSpeed = 50.f;
	MaxSpeed = 6000.f;
	LandSpeed = 250.f;
	CurrentForwardSpeed = 500.f;
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
	// Get current actor rotation
	FRotator CurrentRotation = GetActorRotation();
	// For interpolation from extreme angles
	FRotator NewRotation;

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0, 0, 0);
	// Check if bird is flying too close too steeply or at obtuse angle downwards
	if (CurrentRotation.Pitch < 40.0f && CurrentRotation.Pitch > -40.0f) {
		DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	}
	else if (CurrentRotation.Pitch > 30.0f) {
		NewRotation = CurrentRotation;
		NewRotation.Pitch = 30.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, NewRotation, GetWorld()->GetDeltaSeconds(), 0.25f));
	}
	else if (CurrentRotation.Pitch < -30.0f) {
		NewRotation = CurrentRotation;
		NewRotation.Pitch = -30.0f;
		SetActorRotation(FMath::RInterpTo(CurrentRotation, NewRotation, GetWorld()->GetDeltaSeconds(), 0.25f));
	}
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);

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
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, LandSpeed, MaxSpeed);
}

void ABirdPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed));

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
	float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.0f) : (GetActorRotation().Roll * -2.f);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	//if ((Controller != NULL) && (Val != 0.0f))
	//{
	//	// find out which way is right
	//	const FRotator Rotation = Controller->GetControlRotation();
	//	const FRotator YawRotation(0, Rotation.Yaw, 0);

	//	// get right vector 
	//	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//	// add movement in that direction
	//	AddMovementInput(Direction, Val);
	//}
}