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
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AmonkeyCharacter::CTick(float deltaTime) {
	if (targetPhase == phase) {
		transforming = false;
		if (phase == 0) {
			GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
			GetCharacterMovement()->JumpZVelocity = 1200.f;
			GetCharacterMovement()->AirControl = 0.8f;
			GetCharacterMovement()->MaxAcceleration = 2048.0f;

		}
		else {
			GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
			GetCharacterMovement()->JumpZVelocity = 600.f;
			GetCharacterMovement()->AirControl = 2.0f;
			GetCharacterMovement()->MaxAcceleration = 1600.0f;
		}
	}
	else {
		transforming = true;
		if (targetPhase == 0) {
			if (m1 - deltaTime * morphTransSpeed > 0.0f) m1 -= deltaTime * morphTransSpeed;
			else m1 = 0.0f;
			/*if (m2 - deltaTime * morphTransSpeed > 0.0f) m2 -= deltaTime * morphTransSpeed;
			else m2 = 0.0f;
			if (m3 - deltaTime * morphTransSpeed > 0.0f) m3 -= deltaTime * morphTransSpeed;
			else m3 = 0.0f;*/
			if (m1 == 0.0f && m2 == 0.0f && m3 == 0.0f) {
				phase = 0;
				//GetMesh()->SetSkeletalMesh(phase0Mesh);
			}
		}
		else {
			if (m1 + deltaTime * morphTransSpeed < 1.0f) m1 += deltaTime * morphTransSpeed;
			else m1 = 1.0f;
			/*if (m2 - deltaTime * morphTransSpeed > 0.0f) m2 -= deltaTime * morphTransSpeed;
			else m2 = 0.0f;
			if (m3 - deltaTime * morphTransSpeed > 0.0f) m3 -= deltaTime * morphTransSpeed;
			else m3 = 0.0f;*/
			if (m1 == 1.0f && m2 == 0.0f && m3 == 0.0f) {
				phase = 1;
				//GetMesh()->SetSkeletalMesh(phase1Mesh);
			}
		}
		//GetMesh()->SetMorphTarget("capMorph", m1);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AmonkeyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("Phase0", IE_Pressed, this, &AmonkeyCharacter::toPhase0);
	PlayerInputComponent->BindAction("Phase1", IE_Pressed, this, &AmonkeyCharacter::toPhase1);

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
