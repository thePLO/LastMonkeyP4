// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "monkey.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "monkeyCharacter.h"

//////////////////////////////////////////////////////////////////////////
// AmonkeyCharacter

AmonkeyCharacter::AmonkeyCharacter()
{
	/*static ConstructorHelpers::FObjectFinder<USkeletalMesh> phase0MeshAsset(TEXT("/Game/character/default/default"));
	phase0Mesh = CreateDefaultSubobject<USkeletalMesh>(TEXT("Mesh0"));
	phase0Mesh->
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> phase1MeshAsset(TEXT("/Game/character/head/head"));
	phase1Mesh = CreateDefaultSubobject<USkeletalMesh>(TEXT("Mesh1"));*/
	
	//phase0Mesh = LoadObject<USkeletalMesh>(NULL, TEXT("/Game/character/default/default"), NULL, LOAD_None, NULL);
	//phase1Mesh = LoadObject<USkeletalMesh>(NULL, TEXT("/Game/character/head/head"), NULL, LOAD_None, NULL);
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.8f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	acceleration[0] = 900.0f; maxSpeed[0] = 400.0f; jumpSpeed[0] = 650.0f; doubleJumpSpeed[0] = 650.0f; gliding[0] = 0.0f;
	acceleration[1] = 900.0f; maxSpeed[1] = 250.0f; jumpSpeed[1] = 450.0f; doubleJumpSpeed[1] = 0.0f; gliding[1] = -50.0f;
	acceleration[2] = 500.0f; maxSpeed[2] = 250.0f; jumpSpeed[2] = 450.0f; doubleJumpSpeed[2] = 450.0f; gliding[2] = 0.0f;
	acceleration[3] = 500.0f; maxSpeed[3] = 600.0f; jumpSpeed[3] = 450.0f; doubleJumpSpeed[3] = 450.0f; gliding[3] = 0.0f;

	morph1[0] = 0.0f; morph1[1] = 1.0f; morph1[2] = 0.0f; morph1[3] = 0.0f; //cap
	morph2[0] = 0.0f; morph2[1] = 0.0f; morph2[2] = 1.0f; morph2[3] = 0.0f; //braços

	GetCharacterMovement()->AirControl = 1.0;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

float AmonkeyCharacter::getPhaseTimer(int a) {
	return phaseTimers[a];
}
float AmonkeyCharacter::getPhaseTimes(int a) {
	return phaseTimes[a];
}

void AmonkeyCharacter::CTick(float deltaTime) {
	if (GetCharacterMovement()->IsMovingOnGround()) jumpCounter = 0;
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

	if (targetPhase == phase && (m1== morph1[phase])) {
		transforming = false;
		GetCharacterMovement()->MaxAcceleration = acceleration[phase];
		GetCharacterMovement()->MaxWalkSpeed = maxSpeed[phase];
		GetCharacterMovement()->JumpZVelocity = jumpSpeed[phase];
		if(GetCharacterMovement()->Velocity.Z<0.0f && gliding[phase] != 0.0f) GetCharacterMovement()->Velocity.Z = gliding[phase];
	}
	else {
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
}

//////////////////////////////////////////////////////////////////////////
// Input

void AmonkeyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AmonkeyCharacter::dJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AmonkeyCharacter::dStopJumping);
	
	PlayerInputComponent->BindAction("Phase0", IE_Pressed, this, &AmonkeyCharacter::toPhase0);
	PlayerInputComponent->BindAction("Phase1", IE_Pressed, this, &AmonkeyCharacter::toPhase1);
	PlayerInputComponent->BindAction("Phase1", IE_Released, this, &AmonkeyCharacter::toPhase0);
	PlayerInputComponent->BindAction("Phase2", IE_Pressed, this, &AmonkeyCharacter::toPhase2);
	PlayerInputComponent->BindAction("Phase2", IE_Released, this, &AmonkeyCharacter::toPhase0);

	PlayerInputComponent->BindAxis("MoveForward", this, &AmonkeyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AmonkeyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AmonkeyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AmonkeyCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AmonkeyCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AmonkeyCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AmonkeyCharacter::OnResetVR);
}

void AmonkeyCharacter::dJump() {
	if (GetCharacterMovement()->IsMovingOnGround()) {
		ACharacter::Jump();
	}
	else if (!ACharacter::bPressedJump && !doubleJumping && jumpCounter<airJumps && doubleJumpSpeed[phase] != 0.0f) {
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

void AmonkeyCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AmonkeyCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void AmonkeyCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void AmonkeyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AmonkeyCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AmonkeyCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AmonkeyCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
