#include "Trigger.h"
// include draw debug helpers header file
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

ATrigger::ATrigger()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &ATrigger::OnOverlapBegin);

	//Setup collision box
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);

	//Setup brazier mesh
	BrazierMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Brazier Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BrazierAsset(TEXT("StaticMesh'/Game/Assets/Brazier01.Brazier01'"));
	if (BrazierAsset.Succeeded())
	{
		BrazierMesh->SetStaticMesh(BrazierAsset.Object);
		BrazierMesh->SetupAttachment(RootComponent);
	}

	//Setup sequence
	static ConstructorHelpers::FObjectFinder<ULevelSequence> SequenceAsset(TEXT("LevelSequence'/Game/Sequences/FadeOut.FadeOut'"));
	if (SequenceAsset.Succeeded())
	{
		FadeOut = SequenceAsset.Object;
	}

	NewLocation = FVector(BrazierMesh->GetComponentLocation().X, BrazierMesh->GetComponentLocation().Y, BrazierMesh->GetComponentLocation().Z + 100);
	NewRotation = BrazierMesh->GetComponentRotation();
}

// Called when the game starts or when spawned
void ATrigger::BeginPlay()
{
	Super::BeginPlay();
}

//Called when character overlaps with collision box
void ATrigger::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
	if (OtherActor && (OtherActor->IsA(ACharacter::StaticClass())))
	{
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOut, FMovieSceneSequencePlaybackSettings(), SequenceActor);

		if (SequencePlayer)
		{
			SequencePlayer->Play();
		}

		// Cinder = Cast<ACharacter>(OtherActor);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATrigger::SitOnBrazier, 3.0f, false);
	}
}

void ATrigger::SitOnBrazier()
{
	//Cinder->SetActorLocation(NewLocation);
	//Cinder->SetActorRotation(NewRotation);
	Cinder->SetActorLocationAndRotation(NewLocation, NewRotation, false, 0, ETeleportType::None);
	GetWorldTimerManager().ClearTimer(TimerHandle);
}