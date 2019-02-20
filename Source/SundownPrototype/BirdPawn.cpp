// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#include "BirdPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealMath.h"

// Sets default values
ABirdPawn::ABirdPawn()
{
	CollisionSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/EditorMeshes/EditorSphere.EditorSphere'"));
	if (SphereMeshAsset.Succeeded()) {
		CollisionSphere->SetStaticMesh(SphereMeshAsset.Object);
		CollisionSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CollisionSphere->SetWorldScale3D(FVector(1.0f));
		CollisionSphere->bVisible = false;
		CollisionSphere->bUseDefaultCollision = true;
	}

	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Bird Mesh"));
	BirdMesh->SetupAttachment(CollisionSphere);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BirdMeshAsset(TEXT("SkeletalMesh'/Game/NewBirdModel/BirdNewUv.BirdNewUv'"));
	if (BirdMeshAsset.Succeeded()) {
		BirdMesh->SetSkeletalMesh(BirdMeshAsset.Object);
		BirdMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		BirdMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));
		BirdMesh->SetWorldScale3D(FVector(1.0f));
	}

	 // Create a particle system
	FireParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire Particles"));
	FireParticleSystem->SetupAttachment(CollisionSphere);
	FireParticleSystem->bAutoActivate = true;
	FireParticleSystem->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (FireParticleAsset.Succeeded())
	{
		FireParticleSystem->SetTemplate(FireParticleAsset.Object);
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	mCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("mCameraSpringArm"));
	mCameraSpringArm->SetupAttachment(CollisionSphere);
	mCameraSpringArm->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	mCameraSpringArm->bEnableCameraLag = false;
	mCameraSpringArm->CameraLagSpeed = CamLag;

	//// Create a follow camera
	mCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	mCamera->SetupAttachment(mCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	mCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set handling parameters
	Acceleration = 1.0f;
	TurnSpeed = 45.0f;
	MaxSpeed = 1000.0f;
	MinSpeed = 0.0f;
	CurrentForwardSpeed = 0.0f;
}

void ABirdPawn::BeginPlay()
{
	Super::BeginPlay();

}

void ABirdPawn::Tick(float DeltaSeconds)
{
	// SPLINE MOVEMENT --------------------------------------------------
	if (OnSpline) // Check to make sure reference is valid
	{
		SetActorLocation(SplineBounds->GetComponentLocation()); // Set location to location at distance along spline
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), SplineBounds->GetComponentRotation() + FRotator(0.0f, -90.0f, 0.0f), DeltaSeconds, 5.0f)); // Aaaand rotation to rotation at distance along spline
	}
	else 
	{
		// MOVEMENT BOOLS
		if (move && CurrentForwardSpeed < MaxSpeed) {
			CurrentForwardSpeed += Acceleration;
		}
		else if (!move && CurrentForwardSpeed > 0.0f) {
			CurrentForwardSpeed -= Acceleration;
		}

		const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);
		// Move bird forwards
		AddActorLocalOffset(LocalMove, true);

		// Calculate change in rotation
		FRotator DeltaRotation(0, 0, 0);	// New rotation for updating rotation		

		DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds; // Update pitch
		DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds; // Update yaw
		DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds; //Update roll
															  
		AddActorLocalRotation(DeltaRotation);
	}

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ABirdPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	// Set move to false (stop Cinder from moving on hit)
	move = false; 
	CurrentForwardSpeed = 0.0f;
	// Vector to follow below (2.5f backwards)
	const FVector LocalMove = FVector(-2.5, 0.f, 0.f);
	// Move backwards to avoid calling NotifyHit() again
	AddActorLocalOffset(LocalMove, true); 

	if (Other->GetClass()->IsChildOf(SplineClassType)) {
		OnSpline = true;
		move = false;
		Spline = Cast<USplineComponent>(Hit.GetActor());
		UE_LOG(LogTemp, Warning, TEXT("SplineCyl hit! 1/2"));
		SplineBounds = Cast<UStaticMeshComponent>(Hit.GetComponent());
		UE_LOG(LogTemp, Warning, TEXT("SplineCyl hit! 2/2"));
	}

	// Deflect along the surface when we collide.
	//FRotator CurrentRotation = GetActorRotation();
	//SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.04f));
}

// Called to bind functionality to input
void ABirdPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAction("Move", IE_Released, this, &ABirdPawn::Move);
	PlayerInputComponent->BindAxis("MoveUp", this, &ABirdPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABirdPawn::MoveRightInput);
}

void ABirdPawn::Move() {
	if (move == false && OnSpline == false) {
		move = true;
	}
	else if (move == true) {
		move = false;
	}
}

void ABirdPawn::MoveUpInput(float Val)
{
	if (OnSpline) {
		FVector NewLocation = FVector(0.0f, 10.0f * Val, 0.0f);
		SetActorRelativeLocation(FVector(NewLocation));
	}
	else {
		// Target pitch speed is based in input
		float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

		// When steering, we decrease pitch slightly
		TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

		// Smoothly interpolate to target pitch speed
		CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
	}
}

void ABirdPawn::MoveRightInput(float Val)
{
	if (OnSpline) {
		FVector NewLocation = FVector(0.0f, 0.0f, 10.0f * Val);
		SetActorRelativeLocation(FVector(NewLocation));
	}
	else {
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
}
