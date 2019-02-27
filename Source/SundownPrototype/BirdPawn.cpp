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
	//mCameraSpringArm->CameraLagSpeed = CamLag;

	//// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	// Set handling parameters
	Acceleration = 1.0f;
}

void ABirdPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set relevant character movement properties
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	GetCharacterMovement()->AirControl = 100.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 90.0f);
	GetCharacterMovement()->MaxAcceleration = 600.0f;
	GetCharacterMovement()->MaxWalkSpeed = 4000.0f;
	//GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.0f);
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	// FLYING
	// Step one: Set liftNormalized

	// A) Calculate control inclination
	FVector controlForwardVec = UKismetMathLibrary::GetUpVector(FRotator(GetControlRotation()));
	controlInclination = FVector::DotProduct(controlForwardVec, GetActorForwardVector()); // control inclination ranges from -1 to 1 based on the rotational difference between camera up vector and actor forward vector

	// B) Get value from angle curve using control inclination
	float AngCurveVal = AngCurve->GetFloatValue(FMath::Acos(controlInclination) - 90.0f);

	// C) Get value from velocity curve
	FVector VelocityVec = GetCharacterMovement()->Velocity;
	VelocityVec.Z = FMath::Clamp<float>(VelocityVec.Z, -4000, 0); // Clamp Z within appropriate velocity as this is downwards movement
	float VelCurveVal = VelCurve->GetFloatValue(VelocityVec.Size());

	// D) Calculate lift normalized by multiplying the flight angle curve and velocity curve values
	liftNormalized = VelCurveVal * AngCurveVal;

	// Step two: set flyspeedHold

	// A) Calculate mapRangeClamped
	float mapRangeClamped;
	// First convert Z velocity value to be within the correct range
	FVector2D input = FVector2D(-500.0f, 0.0f);
	FVector2D output = FVector2D(1.5f, 0.0f);
	mapRangeClamped = FMath::GetMappedRangeValueClamped(input, output, GetCharacterMovement()->Velocity.Z);

	// B) FInterp flyspeedHold towards mapRangeClamped
	FMath::FInterpTo(flyspeedHold, mapRangeClamped, DeltaSeconds, FMath::Abs(controlInclination) + 0.5f);

	// C) Get direction to fly in (forward vector of the control rotation's Z component)
	FRotator flyDirection = FRotator(0.0f, 0.0f, GetControlRotation().GetComponentForAxis(EAxis::Z));
	FVector flyForwardVector = UKismetMathLibrary::GetForwardVector(flyDirection);
	
	// D) Add the movement input in the correct direction, using flyspeedHold as weighting to slow down over time if no input
	AddMovementInput(flyForwardVector, flyspeedHold);

	// Step three: GRAVITY?

	// A) Create force against gravity
	float x = GetCharacterMovement()->Mass * GravityConstant * liftNormalized;
	FVector force = FVector(0.0f, 0.0f, x);
	// B) Add force
	GetCharacterMovement()->AddForce(force);

	// Step four: set angle of velocity

	// A) Setup - from actor rotation and velocity
	FVector UpInverse = UKismetMathLibrary::GetUpVector(GetActorRotation()) - 1;
	FVector VelocityNormalized = GetCharacterMovement()->Velocity;
	VelocityNormalized.Normalize();

	// B) Perform dot product, set velocityAngle
	velocityAngle = FMath::Acos(FVector::DotProduct(VelocityNormalized, UpInverse) - 90);

	// Step five: Set flyingVelocity???
	float flyingVelocity = GetCharacterMovement()->Velocity.Size();

	// Step six: set rotation of actor

	// Calculate change in rotation
	FVector DirVelocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0.0f);
	FRotator DirRot = UKismetMathLibrary::MakeRotationFromAxes(FVector(DirVelocity), FVector(GetActorRightVector()), FVector(0.0f, 0.0f, 1.0f));
	DirRot = FRotator(0.0f, 0.0f, DirRot.GetComponentForAxis(EAxis::Z));
	SetActorRelativeRotation(DirRot);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
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

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.04f));
}

// Called to bind functionality to input
void ABirdPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions'
	PlayerInputComponent->BindAxis("MoveUp", this, &ABirdPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABirdPawn::MoveRightInput);
	PlayerInputComponent->BindAxis("PitchInput", this, &ABirdPawn::PitchInput);
	PlayerInputComponent->BindAxis("YawInput", this, &ABirdPawn::YawInput);
}

void ABirdPawn::PitchInput(float Val) {
	AddControllerPitchInput(Val);
}

void ABirdPawn::YawInput(float Val) {
	AddControllerYawInput(Val);
}

void ABirdPawn::MoveUpInput(float Val)
{
	if (OnSpline) {
			SetActorLocation(FVector(SplineBounds->GetComponentLocation()));
	}
	else {
		//AddMovementInput(GetActorForwardVector(), Acceleration * Val);
	}
}

void ABirdPawn::MoveRightInput(float Val)
{
	if (OnSpline) {
		SetActorLocation(FVector(SplineBounds->GetComponentLocation()));
	}
	else {
		// Target yaw speed is based on input
		float TargetYawSpeed = (Val * TurnSpeed);

		// Smoothly interpolate to target yaw speed
		CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
		//AddMovementInput(GetActorRightVector(), Acceleration * Val);
	}
}
