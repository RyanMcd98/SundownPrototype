#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "BirdPawn.h"
#include "GameFramework/Actor.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"
#include "GameFramework/Pawn.h"
#include "MyTriggerVolume.generated.h"

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

	UFUNCTION()
	void SitOnBrazier();

	//Reference to player
	TArray<AActor*> Pawns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	TSubclassOf<class AActor>  PawnClassType;
	APawn* Pawn;

	//Level sequence player used to play fade out
	UPROPERTY()
	ULevelSequencePlayer* SequencePlayer;

	//Actor used to play fade out
	UPROPERTY()
	ALevelSequenceActor* SequenceActor;

	//Level sequence asset played when pawn enters trigger box, can be set in trigger box details 
	UPROPERTY(EditAnywhere, Category = SequenceToPlay)
	class ULevelSequence* FadeOut;

	//New location which the player is set to when pawn enters trigger box, can be set in trigger box details
	UPROPERTY(EditAnywhere, Category = NewLocation)
	FVector Location;

	//New rotation which the player is set to when pawn enters trigger box, can be set in trigger box details
	UPROPERTY(EditAnywhere, Category = NewRotation)
	FRotator Rotation;

	//Set location timer handle
	UPROPERTY()
	FTimerHandle TimerHandle;
};