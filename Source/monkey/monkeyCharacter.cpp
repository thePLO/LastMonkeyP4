#include "monkey.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "monkeyCharacter.h"

AmonkeyCharacter::AmonkeyCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure default character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.8f;

	//BOLA
	GetCharacterMovement()->GroundFriction = 0.8f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//set values for different phases
	for (int i = 0; i < 4; i++) {
		phaseTimes[i] = 4.0f;
		phaseTimers[i] = 4.0f;
	}

	acceleration[0] = 900.0f; maxSpeed[0] = 400.0f; jumpSpeed[0] = 650.0f; doubleJumpSpeed[0] = 650.0f; gliding[0] = 0.0f; //default
	acceleration[1] = 900.0f; maxSpeed[1] = 250.0f; jumpSpeed[1] = 450.0f; doubleJumpSpeed[1] = 0.0f;   gliding[1] =-50.0f;//head
	acceleration[2] = 500.0f; maxSpeed[2] = 250.0f; jumpSpeed[2] = 450.0f; doubleJumpSpeed[2] = 450.0f; gliding[2] = 0.0f; //arms
	acceleration[3] = 500.0f; maxSpeed[3] = 600.0f; jumpSpeed[3] = 450.0f; doubleJumpSpeed[3] = 450.0f; gliding[3] = 0.0f; //feet

	//-----head------------arms---------
	morph1[0] = 0.0f; morph2[0] = 0.0f; //default
	morph1[1] = 1.0f; morph2[1] = 0.0f; //head
	morph1[2] = 0.0f; morph2[2] = 1.0f; //arms
	morph1[3] = 0.0f; morph2[3] = 0.0f; //feet

	GetCharacterMovement()->AirControl = 1.0;
}

float AmonkeyCharacter::getPhaseRel(int a) {
	return  phaseTimers[a]/phaseTimes[a];
}

void AmonkeyCharacter::CTick(float deltaTime) {
	FQuat quat = FQuat
	(
		FVector(1,0,0),
		0.5f
	);
	
	//phase timers update
	for (int i = 1; i < 4; i++) {
		if (phase == i) {
			if (phaseTimers[i] - deltaTime > 0.0f) phaseTimers[i] -= deltaTime;
			else {
				phaseTimers[i] = 0.0f;
				targetPhase = 0;
			}
		}
		else {
			if (phaseTimers[i] + deltaTime*recoveryRate < phaseTimes[i]) phaseTimers[i] += deltaTime*recoveryRate;
			else {
				phaseTimers[i] = phaseTimes[i];
			}
		}
	}
	//update morphs or stats phase
	if (targetPhase == phase && (m1== morph1[phase]) && m2 == morph2[targetPhase]) {
		transforming = false;
		GetCharacterMovement()->MaxAcceleration = acceleration[phase];
		GetCharacterMovement()->MaxWalkSpeed = maxSpeed[phase];
		GetCharacterMovement()->JumpZVelocity = jumpSpeed[phase];
		if(GetCharacterMovement()->Velocity.Z<0.0f && gliding[phase] != 0.0f) GetCharacterMovement()->Velocity.Z = gliding[phase];
	} else {
		transforming = true;
		if (m1 > morph1[targetPhase]) {
			if (m1 - deltaTime * morphTransSpeed > morph1[targetPhase]) m1 -= deltaTime * morphTransSpeed;
			else m1 = morph1[targetPhase];
		}else if (m1 < morph1[targetPhase]) {
			if (m1 + deltaTime * morphTransSpeed < morph1[targetPhase]) m1 += deltaTime * morphTransSpeed;
			else m1 = morph1[targetPhase];
		}

		if (m2 > morph2[targetPhase]) {
			if (m2 - deltaTime * morphTransSpeed > morph2[targetPhase]) m2 -= deltaTime * morphTransSpeed;
			else m2 = morph2[targetPhase];
		}
		else if (m2 < morph2[targetPhase]) {
			if (m2 + deltaTime * morphTransSpeed < morph2[targetPhase]) m2 += deltaTime * morphTransSpeed;
			else m2 = morph2[targetPhase];
		}

		if (m1 == morph1[targetPhase] && m2 == morph2[targetPhase]/* && m3 == morph3[targetPhase]*/) {
			phase = targetPhase;
			if(phase==1) GetCharacterMovement()->Velocity.Z = 0.0f;
		}
	}
	//updata attack vars
	if (attaking) {
		attTimer -= deltaTime;
		if (attTimer <= 0) attaking = false;
	}
	if (attCd > 0) attCd -= deltaTime;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AmonkeyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent){

	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AmonkeyCharacter::dJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AmonkeyCharacter::dStopJumping);
	
	PlayerInputComponent->BindAction("AttackR", IE_Pressed, this, &AmonkeyCharacter::AttackR);
	PlayerInputComponent->BindAction("AttackL", IE_Pressed, this, &AmonkeyCharacter::AttackL);

	//PlayerInputComponent->BindAction("Phase0", IE_Pressed, this, &AmonkeyCharacter::toPhase0);

	PlayerInputComponent->BindAction("Head", IE_Pressed, this, &AmonkeyCharacter::Head);
	PlayerInputComponent->BindAction("Head", IE_Released, this, &AmonkeyCharacter::outHead);

	PlayerInputComponent->BindAction("Ball", IE_Pressed, this, &AmonkeyCharacter::Ball);
	PlayerInputComponent->BindAction("Ball", IE_Released, this, &AmonkeyCharacter::outBall);

	PlayerInputComponent->BindAxis("MoveForward", this, &AmonkeyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AmonkeyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AmonkeyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AmonkeyCharacter::LookUpAtRate);
}

void AmonkeyCharacter::dJump() {
	if (GetCharacterMovement()->IsMovingOnGround()) {
		jumpCounter = 0;
		ACharacter::Jump();
	}
	else if (!ACharacter::bPressedJump && jumpCounter<airJumps && doubleJumpSpeed[phase] != 0.0f) {
		GetCharacterMovement()->Velocity.Z = 0.0f;
		GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, doubleJumpSpeed[phase]));
		jumpCounter++;
		doubleJumping = true;
	}
}

void AmonkeyCharacter::dStopJumping() {
	&ACharacter::StopJumping;
	doubleJumping = false;
}

void AmonkeyCharacter::AttackR() {
	if (phase == 2 && !attaking && attCd <= 0.0f) {
		attaking = true;
		attTimer = 0.3f; //attack duration
		attCd = 0.5f; //time till next attack
	}
}
void AmonkeyCharacter::AttackL() {
	if (phase == 2 && !attaking && attCd <= 0.0f) {
		attaking = true;
		attTimer = 0.3f; //attack duration
		attCd = 0.5f; //time till next attack
	}
}

void AmonkeyCharacter::modInput(int index, bool state) {
	if(state) inputState[index - 1] = 1;
	else inputState[index - 1] = 0;

	if (state) targetPhase = index;
	else {
		if (phase == index) targetPhase = 0;//return def if release actual
	}
}

void AmonkeyCharacter::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AmonkeyCharacter::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AmonkeyCharacter::MoveForward(float Value) {
	if ((Controller != NULL) && (Value != 0.0f)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AmonkeyCharacter::MoveRight(float Value) {
	if ( (Controller != NULL) && (Value != 0.0f) ) {
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}