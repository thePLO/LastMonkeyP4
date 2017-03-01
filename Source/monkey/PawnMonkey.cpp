// Fill out your copyright notice in the Description page of Project Settings.

#include "monkey.h"
#include "PawnMonkey.h"
#include "PawnMonkeyMovementComponent.h"

// Sets default values
APawnMonkey::APawnMonkey()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(40.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->BodyInstance.bLockXRotation = true;
	SphereComponent->BodyInstance.bLockYRotation = true;
	SphereComponent->SetAngularDamping(0.1f);
	/*SphereComponent->BodyInstance.SetCollisdonProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetAngularDamping(0.1f);
	SphereComponent->SetLinearDamping(0.1f);
	
	SphereComponent->BodyInstance.MassScale = 3.5f;
	SphereComponent->BodyInstance.MaxAngularVelocity = 0.0f;
	SphereComponent->SetNotifyRigidBodyCollision(true);*/

	// Create and position a mesh component so we can see where our sphere is
	UStaticMeshComponent* SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	SphereVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/Geometry/Meshes/1M_Cube2.1M_Cube2"));
	SphereVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereVisualAsset.Succeeded())
	{
		SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
		SphereVisual->SetWorldScale3D(FVector(0.8f));
	}

	// Create a particle system that we can activate or deactivate
	OurParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MovementParticles"));
	OurParticleSystem->SetupAttachment(SphereVisual);
	OurParticleSystem->bAutoActivate = false;
	OurParticleSystem->SetRelativeLocation(FVector(-20.0f, 0.0f, 20.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (ParticleAsset.Succeeded())
	{
		OurParticleSystem->SetTemplate(ParticleAsset.Object);
	}

	// Use a spring arm to give the camera smooth, natural-feeling motion.
	USpringArmComponent* SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->RelativeRotation = FRotator(-45.f, 0.f, 0.f);
	//SpringArm->bAbsoluteRotation = true;
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;

	// Create a camera and attach to our spring arm
	UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// Take control of the default player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create an instance of our movement component, and tell it to update our root component.
	
	OurMovementComponent = CreateDefaultSubobject<UPawnMonkeyMovementComponent>(TEXT("CustomMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;
}

// Called when the game starts or when spawned
void APawnMonkey::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APawnMonkey::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APawnMonkey::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &APawnMonkey::ParticleToggle);

	InputComponent->BindAxis("MoveForward", this, &APawnMonkey::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APawnMonkey::MoveRight);
	InputComponent->BindAxis("Turn", this, &APawnMonkey::Turn);

}


UPawnMovementComponent* APawnMonkey::GetMovementComponent() const
{
	return OurMovementComponent;
}

void APawnMonkey::MoveForward(float AxisValue)
{
	/*if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorForwardVector() * AxisValue);
	}*/
	//SphereComponent->AddForce(GetActorForwardVector() * AxisValue * 200.0f);
	SphereComponent->AddImpulse(GetActorForwardVector() * AxisValue * 200.0f);
	//const FVector Torque = FVector(-1.f * AxisValue * 500000.0f, 0.f, 0.f);
	//SphereComponent->AddTorque(Torque);
	//FQuat myActorQuat = SphereComponent->GetComponentQuat();
	//SphereComponent->AddTorque(myActorQuat.RotateVector(FVector(10.0f*AxisValue, 0.0f, 0.0f)), NAME_None, true);
	
}

void APawnMonkey::MoveRight(float AxisValue)
{
	if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorRightVector() * AxisValue);
	}
}

void APawnMonkey::Turn(float AxisValue)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += AxisValue;
	SetActorRotation(NewRotation);
}

void APawnMonkey::ParticleToggle()
{
	if (OurParticleSystem && OurParticleSystem->Template)
	{
		OurParticleSystem->ToggleActive();
	}
	SphereComponent->AddImpulse(GetActorUpVector() * 20000.0f);
}