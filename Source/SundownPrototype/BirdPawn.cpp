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
	mCamera->bUsePawnControlRotation = true;

	// Set handling parameters
	Acceleration = 1.0f;
}

void ABirdPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set relevant character movement properties
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	GetCharacterMovement()->AirControl = 100.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 5.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 90.0f);
	GetCharacterMovement()->MaxAcceleration = 600.0f;
}

void ABirdPawn::Tick(float DeltaSeconds)
{
	// FLYING
	// Step one: Set liftNormalized

	// A) Calculate control inclination
	FVector controlUpVec = UKismetMathLibrary::GetUpVector(FRotator(GetControlRotation()));
	controlInclination = FVector::DotProduct(controlUpVec, GetActorForwardVector()); // control inclination ranges from -1 to 1 based on the rotational difference between camera up vector and actor forward vector

	// B) Get value from angle curve using control inclination
	float AngCurveVal = AngCurve->GetFloatValue(FMath::Acos(controlInclination) - 90.0f);

	// C) Get value from velocity curve
	FVector VelocityVec = GetCharacterMovement()->Velocity;
	VelocityVec.Z = FMath::Clamp<float>(VelocityVec.Z, -4000, 0); // Clamp Z within appropriate velocity as this deal with downwards movement
	float VelCurveVal = VelCurve->GetFloatValue(VelocityVec.Size());

	// D) Calculate lift normalized by multiplying the flight angle curve and velocity curve values
	liftNormalized = VelCurveVal * AngCurveVal;
	//UE_LOG(LogTemp,Warning,TEXT("Lift normalized: %f"), liftNormalized);

	// Step two: GRAVITY!

	// A) Create force against gravity
	float x = GetCharacterMovement()->Mass * GravityConstant * liftNormalized;
	FVector force = FVector(0.0f, 0.0f, x);
	// B) Add force
	GetCharacterMovement()->AddForce(force);

	// Step three: set flyspeedHold

	// A) Calculate mapRangeClamped
	float mapRangeClamped;
	// First convert Z velocity value to be within the correct range
	FVector2D input = FVector2D(-500.0f, 0.0f);
	FVector2D output = FVector2D(1.5f, 0.0f);
	mapRangeClamped = FMath::GetMappedRangeValueClamped(input, output, GetCharacterMovement()->Velocity.Z);

	// B) FInterp flyspeedHold towards mapRangeClamped
	FMath::FInterpTo(flyspeedHold, mapRangeClamped, DeltaSeconds, FMath::Abs(controlInclination) + 0.5f);

	// C) Get direction to fly in
	FVector flyForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());

	// D) Add the movement input in the correct direction, using flyspeedHold as weighting
	AddMovementInput(flyForwardVector, flyspeedHold);

	// ^ ^ ^ DONE ^ ^ ^ /////////////////////////////////////////////////////////////
	
	//UE_LOG(LogTemp,Warning,TEXT("FlyDirection: %s"), *flyDirection.ToString());
	//UE_LOG(LogTemp,Warning,TEXT("MovementVector: %s"), *flyForwardVector.ToString());
	//UE_LOG(LogTemp,Warning,TEXT("FlySpeedHold: %f"), flyspeedHold);
	UE_LOG(LogTemp,Warning,TEXT("Control rotation: %s"), *GetControlRotation().ToString());

	// Calculate change in yaw rotation
	FVector DirVelocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0.0f);
	FRotator DirRot = UKismetMathLibrary::MakeRotationFromAxes(DirVelocity, GetActorRightVector(), FVector(0.0f, 0.0f, 1.0f));
	FRotator NewDirRot = FRotator(0.0f, DirRot.Yaw, 0.0f);
	SetActorRelativeRotation(NewDirRot);

	// Set velocity
	float ZVel = FMath::FInterpTo(GetCharacterMovement()->Velocity.Z, (controlInclination * -980 * FMath::Abs(controlInclination)), DeltaSeconds, 2);
	FVector newVel = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, ZVel);
	GetCharacterMovement()->Velocity.Set(newVel.X, newVel.Y, newVel.Z);

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
}

// Called to bind functionality to input
void ABirdPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions'
	PlayerInputComponent->BindAxis("MoveUp", this, &ABirdPawn::MoveUpInput);
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
		
	}
}
