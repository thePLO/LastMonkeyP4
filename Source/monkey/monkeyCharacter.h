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

	UFUNCTION(BlueprintCallable, Category = "SunShine")	void CTick(float deltaTime);
	UFUNCTION(BlueprintCallable, Category = "SunShine")	float getPhaseRel(int a);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int targetPhase = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int phase = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool transforming = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool attaking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float attTimer = 0.0f; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float attCd = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float morphTransSpeed = 5.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float recoveryRate = 1.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float m1 = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float m2 = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float m3 = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool doubleJumping = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int airJumps = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int jumpCounter = 0;

	UPROPERTY(EditAnywhere)	float acceleration[4];
	UPROPERTY(EditAnywhere)	float maxSpeed[4];
	UPROPERTY(EditAnywhere)	float jumpSpeed[4];
	UPROPERTY(EditAnywhere)	float doubleJumpSpeed[4];
	UPROPERTY(EditAnywhere)	float gliding[4];
	UPROPERTY(VisibleAnywhere) float morph1[4];
	UPROPERTY(VisibleAnywhere) float morph2[4];
	UPROPERTY(VisibleAnywhere) float phaseTimes[4];
	UPROPERTY(VisibleAnywhere) float phaseTimers[4];
	UPROPERTY(VisibleAnywhere) int inputState[3];

protected:
	void dJump(); 
	void dStopJumping();
	void Attack();
	void modInput(int index, bool state);
	inline void toPhase0() { targetPhase = 0; }
	inline void toPhase1() { modInput(1, true); }
	inline void outPhase1() { modInput(1, false);}
	inline void toPhase2() { modInput(2, true);}
	inline void outPhase2() { modInput(2, false); }

	void MoveForward(float Value);
	void MoveRight(float Value);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

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

