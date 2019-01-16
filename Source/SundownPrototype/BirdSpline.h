// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Classes/Components/SplineComponent.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
