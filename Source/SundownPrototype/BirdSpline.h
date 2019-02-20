// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Classes/Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "BirdSpline.generated.h"

UCLASS()
class SUNDOWNPROTOTYPE_API ABirdSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABirdSpline();

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(EditAnywhere, Category = SplineSettings)
	USplineComponent* MovementSpline;

	/** Starting cylinder to attach to spline */
	UPROPERTY(EditAnywhere, Category = SplineSettings)
	UStaticMeshComponent* StartCylinder;

	/** Reference to player */
	TArray<AActor*> Pawns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
		TSubclassOf<class AActor>  PawnClassType;
	APawn* Pawn;

protected:

	/** Override tick */
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds);
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	/** End overrides */

private:
	bool SplineStarted = false; // Whether spline is being travelled along based on collision with the StartCylinder
	float SplineDistance = 0.0f; // Distance along spline
};
