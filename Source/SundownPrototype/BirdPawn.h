// Developed by Gary Whittle and Scott Douglas, based on Unreal's Flight Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "BirdPawn.generated.h"

UCLASS()
class SUNDOWNPROTOTYPE_API ABirdPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABirdPawn();

	// Soft collision sphere
	//UPROPERTY(EditAnywhere, Category = Collision)
	//	UStaticMeshComponent* CollisionSphere;

	//// Skeletal mesh for bird
	//UPROPERTY(EditAnywhere)
	//	USkeletalMeshComponent* BirdMesh;

	// Spline reference variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spline)
		TSubclassOf<class AActor>  SplineClassType;
	USplineComponent* Spline; // the spline
	UStaticMeshComponent* SplineBounds; // the spline bounds

	// Camera components
	UPROPERTY(EditAnywhere)
		USpringArmComponent* mCameraSpringArm;
	UPROPERTY(EditAnywhere)
		UCameraComponent* mCamera;

	//Bird particle system
	/*UPROPERTY(EditAnywhere)
		UParticleSystemComponent* FireParticleSystem;*/

protected:

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

	/** Move is set to false by default */
	bool move = false;

	/** Spline movement bool, false by default */
	bool OnSpline = false;

	// FLYING MOVEMENT
	/** Flight Velocity Lift Multiplier Curve */
	UPROPERTY(EditAnywhere, Category = Flight)
		UCurveFloat* VelCurve;

	/** Flight Angle Lift Multiplier Curve */
	UPROPERTY(EditAnywhere, Category = Flight)
		UCurveFloat* AngCurve;

	/** How quickly forward speed changes */
	UPROPERTY(EditDefaultsOnly, Category = Flight)
		float Acceleration;

	/** The force of gravity */
	UPROPERTY(EditDefaultsOnly, Category = Flight)
		float GravityConstant;

	/** The angle of actor's velocity */
	float velocityAngle = 0.0f;

	// This is used when calculating the inclination of the character
	float controlInclination;

	// This is used to control the lift of the bird
	float liftNormalized;

	// This is the flyspeedHold variable for handling gliding (default value is 1.0f)
	float flyspeedHold = 1.0f;






	/** How quickly pawn can steer */
	UPROPERTY(Category = Movement, EditAnywhere)
		float TurnSpeed;

	///** Max forward speed */
	//UPROPERTY(Category = Movement, EditAnywhere)
	//	float MaxSpeed;

	///** Min forward speed */
	//UPROPERTY(Category = Movement, EditAnywhere)
	//	float MinSpeed;

	///** Camera lag speed (how quickly camera moves towards target socket */
	//UPROPERTY(Category = Camera, EditAnywhere)
	//	float CamLag;

	///** Current forward speed */
	//UPROPERTY(Category = Movement, EditAnywhere)
	//	float CurrentForwardSpeed;

	//** Current yaw speed */
	float CurrentYawSpeed = 0.0f;

	///** Current pitch speed */
	//float CurrentPitchSpeed;

	///** Current roll speed */
	//float CurrentRollSpeed;

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void PitchInput(float Val);
	void YawInput(float Val);

public:
	/** Returns CameraSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraArm() const { return mCameraSpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return mCamera; }
};
