// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/SplineComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "BirdPawn.generated.h"

UCLASS()
class SUNDOWNPROTOTYPE_API ABirdPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABirdPawn();

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* BirdMesh;
	/** Spline component for bird to follow can be set by designers **/
	UPROPERTY(Category = Spline, EditAnywhere)
		AActor* SplineActor;
		UActorComponent* SplinePtr;
		USplineComponent* Spline;
	/** Camera components */
	UPROPERTY(EditAnywhere)
		USpringArmComponent* mCameraSpringArm;
	UPROPERTY(EditAnywhere)
		UCameraComponent* mCamera;
	UPROPERTY(EditAnywhere)
		UParticleSystemComponent* FireParticleSystem;

protected:

	/** Bound to the thrust axis */
	void ThrustInput(float Val);

	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	// Begin AActor overrides
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	void CameraTick();

private:

	/** How quickly forward speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
		float Acceleration;

	/** How quickly pawn can steer */
	UPROPERTY(Category = Plane, EditAnywhere)
		float TurnSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere)
		float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Yaw, EditAnywhere)
		float MinSpeed;

	/** Fairy light intensity rate (energy) */
	UPROPERTY(Category = Fairy, EditAnywhere)
		float FairyEnergy;

	/** Current forward speed */
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

	/** Camera Movement Floats */
	float CamMoveX = 0.0f;
	float CamMoveY = 0.0f;

	/** Current rotation */
	FRotator CurrentRotation;

	/** Camera FObjects */
	FVector CameraLoc;
	FRotator CameraRot;
	FVector CameraSca;

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns BirdMesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetBirdMesh() const { return BirdMesh; }
	/** Returns CameraSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraArm() const { return mCameraSpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return mCamera; }
};
