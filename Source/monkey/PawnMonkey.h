// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "PawnMonkey.generated.h"

UCLASS()
class MONKEY_API APawnMonkey : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnMonkey();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UParticleSystemComponent* OurParticleSystem;
	USphereComponent* SphereComponent;

	class UPawnMonkeyMovementComponent* OurMovementComponent;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void ParticleToggle();
};