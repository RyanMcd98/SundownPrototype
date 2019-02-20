// Sundown Movement Spline

/** 
TO-DO...

PART 1

Create the spline itself (the spline shape)
Create tools so that designers can place/edit points on the spline
Get bird to follow spline, experiment with changing bird speed
Setup a movement bounds system and integrate into the movement spline w/ speed control

**/

#include "BirdSpline.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values
ABirdSpline::ABirdSpline()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create spline component
	MovementSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MovementSpline"));
	MovementSpline->SetupAttachment(RootComponent);
	
	StartCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Collision Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("StaticMesh'/Engine/EngineMeshes/Cylinder.Cylinder'"));
	if (CylinderMeshAsset.Succeeded()) {
		StartCylinder->SetStaticMesh(CylinderMeshAsset.Object);
		StartCylinder->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StartCylinder->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		StartCylinder->SetWorldScale3D(FVector(1.0f));
		StartCylinder->SetMobility(EComponentMobility::Movable);
		StartCylinder->bVisible = false;
		StartCylinder->bCastDynamicShadow = false;
	}
}

void ABirdSpline::BeginPlay() {
	Super::BeginPlay();
	// Find the player pawn
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), PawnClassType, Pawns);
	Pawn = Cast<APawn>(Pawns[0]);

	if (Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn found!"));
	}
}

void ABirdSpline::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if ((SplineStarted) && (SplineDistance < MovementSpline->GetDistanceAlongSplineAtSplinePoint(MovementSpline->GetNumberOfSplinePoints() - 1))) {
		SplineDistance += 10;
		//UE_LOG(LogTemp, Warning, TEXT("Spline should be moving!"));
		StartCylinder->SetWorldLocation(MovementSpline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World), false);
		StartCylinder->SetRelativeRotation(MovementSpline->GetRotationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World), false);
		}
}

void ABirdSpline::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit){
	UE_LOG(LogTemp, Warning, TEXT("Spline cylinder hit!"));
	if (SplineStarted != true) {
		SplineStarted = true;
		UE_LOG(LogTemp, Warning, TEXT("Spline should start!"));
		StartCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StartCylinder->SetCollisionProfileName(TEXT("OverlapAll"));
	}
}