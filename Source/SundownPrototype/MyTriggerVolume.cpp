#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)
#define printFString(text, fstring) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(text), fstring))

#include "MyTriggerVolume.h"
// include draw debug helpers header file
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

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

}

void AMyTriggerVolume::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		// print to screen using above defined method when actor enters trigger volume
		print("Overlap Begin");
		printFString("Other Actor = %s", *OtherActor->GetName());

		FVector Location(this->GetActorLocation().X, this->GetActorLocation().Y + 3000, this->GetActorLocation().Z);
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AMyTriggerVolume>(Location, Rotation, SpawnInfo);

		//Actual Spawn. The following function returns a reference to the spawned actor
		ABirdPawn* MyBirdPawnBPRef = GetWorld()->SpawnActor<ABirdPawn>(SpawnBirdPawnBP, GetTransform(), SpawnInfo);
	}
}

void AMyTriggerVolume::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		// print to screen using above defined method when actor leaves trigger volume
		print("Overlap Ended");
		printFString("%s has left the Trigger Volume", *OtherActor->GetName());
	}
}