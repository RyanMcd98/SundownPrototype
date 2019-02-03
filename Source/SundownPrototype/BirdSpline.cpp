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
	// Create spline component
	MovementSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MovementSpline"));
	MovementSpline->SetupAttachment(RootComponent);
}

