#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BirdPawn.generated.h"

UCLASS()
class SUNDOWNPROTOTYPE_API ABirdPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABirdPawn();

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

protected:

	/** Bound to the control rotation pitch (camera too) */
	void PitchInput(float Val);

	/** Bound to the control rotation yaw (camera too) */
	void YawInput(float Val);

	// Begin AActor overrides
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

private:

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

	// This is used when calculating the inclination of the character
	float InclinationAmount;
	// This is used to control the lift of the bird
	float LiftAmount;
	// This is the flyspeedHold variable for handling movement (default value is 1.0f)
	float SpeedHoldAmount = 1.0f;

	// Calculate flight function
	void CalculateFlight(float DeltaSeconds);

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraArm() const { return mCameraSpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return mCamera; }

	bool allowedToFly = true;
};
