#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)
#define printFString(text, fstring) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(text), fstring))

#include "MyTriggerVolume.h"
// include draw debug helpers header file
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMyTriggerVolume::AMyTriggerVolume()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &AMyTriggerVolume::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AMyTriggerVolume::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AMyTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugBox(GetWorld(), GetActorLocation(), GetActorScale() * 100, FColor::Cyan, true, -1, 0, 5);

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), PawnClassType, Pawns);
	Pawn = Cast<APawn>(Pawns[0]);

	if (Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn found!"));
	}
}

void AMyTriggerVolume::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (OtherActor && (OtherActor->IsA(APawn::StaticClass())))
	{
		// print to screen using above defined method when actor enters trigger volume
		print("Overlap Begin");
		printFString("Other Actor = %s", *OtherActor->GetName());

		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOut, FMovieSceneSequencePlaybackSettings(), SequenceActor);

		if (SequencePlayer)
		{
			SequencePlayer->Play();
		}

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyTriggerVolume::SitOnBrazier, 3.0f, false);
	}
}

void AMyTriggerVolume::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (OtherActor && (OtherActor->IsA(APawn::StaticClass())))
	{
		// print to screen using above defined method when actor leaves trigger volume
		print("Overlap Ended");
		printFString("%s has left the Trigger Volume", *OtherActor->GetName());
	}
}

void AMyTriggerVolume::SitOnBrazier()
{
	Pawn->SetActorLocation(Location);
	Pawn->SetActorRotation(Rotation);
	GetWorldTimerManager().ClearTimer(TimerHandle);
}
