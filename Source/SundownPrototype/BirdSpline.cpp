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

// Sets default values
ABirdSpline::ABirdSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	// Create spline component
	MovementSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MovementSpline"));
	MovementSpline->SetupAttachment(RootComponent);

	MovementSpline->bSplineHasBeenEdited = true;

}

// Called when the game starts or when spawned
void ABirdSpline::BeginPlay()
{
	//Super::BeginPlay();

	
}

// Called every frame
void ABirdSpline::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

}

