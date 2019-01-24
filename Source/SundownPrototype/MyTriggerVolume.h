#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "BirdPawn.h"
#include "GameFramework/Actor.h"
#include "MyTriggerVolume.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class SUNDOWNPROTOTYPE_API AMyTriggerVolume : public ATriggerVolume
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// constructor sets default values for this actor's properties
	AMyTriggerVolume();

	// overlap begin function
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	// overlap end function
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	/*Blueprint Reference of bird pawn class*/
	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ABirdPawn> SpawnBirdPawnBP;

};