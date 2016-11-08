// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "monkeyCharacter.generated.h"

UCLASS(config=Game)
class AmonkeyCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AmonkeyCharacter();

	UFUNCTION(BlueprintCallable, Category = "SunShine")
		void CTick(float deltaTime);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterMesh, meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* phase0Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterMesh, meta = (AllowPrivateAccess = "true"))
		USkeletalMesh* phase1Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterMesh, meta = (AllowPrivateAccess = "true"))
		UAnimBlueprint* phase0Anim;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CharacterMesh, meta = (AllowPrivateAccess = "true"))
		UAnimBlueprint* phase1Anim;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int targetPhase = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int phase = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool transforming = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float morphTransSpeed = 3.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float m1 = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float m2 = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float m3 = 0.0f;

protected:

	inline void toPhase0() { if (!transforming) targetPhase = 0; }
	inline void toPhase1() { if (!transforming) targetPhase = 1; }

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

