// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#include "BirdPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "UnrealMath.h"

// Sets default values
ABirdPawn::ABirdPawn()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("mCameraSpringArm"));
	mCameraSpringArm->SetupAttachment(RootComponent);
	mCameraSpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	mCameraSpringArm->bEnableCameraLag = false;

	//// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = true;

	// Set handling parameters
	Acceleration = 10.0f;
}

void ABirdPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set relevant character movement properties
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
	GetCharacterMovement()->MaxAcceleration = 600.0f;
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	CalculateFlight(DeltaSeconds);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ABirdPawn::CalculateFlight(float DeltaSeconds) {
	// Step 1: Calculate LiftAmount
	// A) Calculate control inclination
	FVector controlUpVec = UKismetMathLibrary::GetUpVector(FRotator(GetControlRotation()));
	// control inclination ranges from -1 to 1 based on the rotational difference between camera up vector and actor forward vector
	InclinationAmount = FVector::DotProduct(controlUpVec, GetActorForwardVector()); 
	// B) Get value from angle curve using Inclination Amount (takeaway 90 degrees to get the correct angle)
	float AngCurveVal = AngCurve->GetFloatValue(FMath::Acos(InclinationAmount) - 90.0f);
	// C) Get value from velocity curve
	FVector VelocityVec = GetCharacterMovement()->Velocity;
	// Clamp Z within appropriate velocity as this deal with downwards movement - change the negative value to allow higher "down" velocity amounts to influence LiftAmount
	VelocityVec.Z = FMath::Clamp<float>(VelocityVec.Z, -2000, 0); 
	float VelCurveVal = VelCurve->GetFloatValue(VelocityVec.Size());
	// E) Calculate lift normalized by multiplying the flight angle curve and velocity curve values
	LiftAmount = VelCurveVal * AngCurveVal;

	// Step 2: GRAVITY!
	// A) Create force against gravity
	float x = GetCharacterMovement()->Mass * GravityConstant * LiftAmount;
	FVector force = FVector(0.0f, 0.0f, x);
	// B) Add force
	GetCharacterMovement()->AddForce(force);

	// Step 3: Add movement input in the correct direction (based on SpeedHoldAmount and controlrotation forward vector)
	// A) Calculate mapRangeClamped
	float mapRangeClamped;
	// First convert Z velocity value to be within the correct range
	FVector2D input = FVector2D(-500.0f, 0.0f);
	FVector2D output = FVector2D(1.5f, 0.0f);
	// mapRangeClamped represents the difference in the Z velocity of the character, but clamped within reasonable values to be used for SpeedHoldAmount)
	mapRangeClamped = FMath::GetMappedRangeValueClamped(input, output, GetCharacterMovement()->Velocity.Z);
	// B) FInterp SpeedHoldAmount towards mapRangeClamped
	FMath::FInterpTo(SpeedHoldAmount, mapRangeClamped, DeltaSeconds, FMath::Abs(InclinationAmount) + 0.5f);
	// C) Get direction to fly in
	FVector flyForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	// D) Add the movement input in the correct direction, using flyspeedHold as weighting
	AddMovementInput(flyForwardVector, SpeedHoldAmount);

	// Step 4: Rotate the character based on the yaw value only
	// A) Calculate change in yaw rotation
	FVector DirVelocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0.0f); //XY Movement FVector
	FRotator DirRot = UKismetMathLibrary::MakeRotationFromAxes(DirVelocity, UKismetMathLibrary::GetRightVector(GetControlRotation()), FVector(0.0f, 0.0f, 1.0f)); // Create rotator from control rotation and XY movement
	FRotator NewDirRot = FRotator(0.0f, DirRot.Yaw, 0.0f); // Create FRotator with just the Yaw
	SetActorRelativeRotation(NewDirRot); // Set relative rotation - X and Z rotation won't change

	// Step 5: Apply Z velocity to character
	// A) Set Z velocity based on the InclinationAmount (character steepness)
	float ZVel = FMath::FInterpTo(GetCharacterMovement()->Velocity.Z, (InclinationAmount * -980 * FMath::Abs(InclinationAmount)), DeltaSeconds, 4);
	FVector newVel = FVector(0.0f, 0.0f, ZVel);
	GetCharacterMovement()->Velocity.SetComponentForAxis(EAxis::Z, newVel.Z);
}

void ABirdPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (Other->GetClass()->IsChildOf(SplineClassType)) {
		OnSpline = true;
		Spline = Cast<USplineComponent>(Hit.GetActor());
		UE_LOG(LogTemp, Warning, TEXT("SplineCyl hit! 1/2"));
		SplineBounds = Cast<UStaticMeshComponent>(Hit.GetComponent());
		UE_LOG(LogTemp, Warning, TEXT("SplineCyl hit! 2/2"));
	}
}

// Called to bind functionality to input
void ABirdPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("PitchInput", this, &ABirdPawn::PitchInput);
	PlayerInputComponent->BindAxis("YawInput", this, &ABirdPawn::YawInput);
}

void ABirdPawn::PitchInput(float Val) {
	AddControllerPitchInput(Val);
}

void ABirdPawn::YawInput(float Val) {
	AddControllerYawInput(Val);
}
