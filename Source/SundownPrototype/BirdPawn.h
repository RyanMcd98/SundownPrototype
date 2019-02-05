// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "BirdPawn.generated.h"

UCLASS()
class SUNDOWNPROTOTYPE_API ABirdPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABirdPawn();

	// Spline system
	TArray<AActor*> Splines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spline)
		TSubclassOf<class AActor>  SplineClassType;
	USplineComponent* Spline;

	// Soft collision sphere
	UPROPERTY(EditAnywhere, Category = Collision)
		class USphereComponent* CollisionSphere;

	// Camera components
	UPROPERTY(EditAnywhere)
		USpringArmComponent* mCameraSpringArm;
	UPROPERTY(EditAnywhere)
		UCameraComponent* mCamera;

	//Bird particle system
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
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

private:

	/** Spline speed intensity */
	UPROPERTY(Category = Spline, EditAnywhere)
		float SplineSpeed;

	/** How quickly forward speed changes */
	UPROPERTY(Category = Movement, EditAnywhere)
		float Acceleration;

	/** How quickly pawn can steer */
	UPROPERTY(Category = Movement, EditAnywhere)
		float TurnSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Movement, EditAnywhere)
		float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Movement, EditAnywhere)
		float MinSpeed;

	/** Camera lag speed (how quickly camera moves towards target socket */
	UPROPERTY(Category = Camera, EditAnywhere)
		float CamLag;

	/** Current forward speed */
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

	/** Overlapping actors array */
	TArray<AActor*> OverlappingActors;

	// Spline distance variable for handling spline movement
	float SplineDistance;

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraArm() const { return mCameraSpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return mCamera; }
};
