// Copyright 2024 Crows in a Crisis

#include "PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "PickupObject.h"
#include "EnhancedInputComponent.h"
#include "Engine/GameEngine.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Grapple with Black Magics
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = GetCapsuleComponent();

	FScriptDelegate delegate;//Attach collision sphere to onComponentHit func
	delegate.BindUFunction(this, "OnComponentHit");
	CollisionBox->OnComponentBeginOverlap.AddUnique(delegate);

	//Finish collision box setup
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	AttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Attach Point"));
	AttachPoint->SetupAttachment(RootComponent);
	AttachPoint->SetRelativeLocation(FVector(-50.f, 0.0f, 0.0f));

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Mapping Context for  Movement
	if (APlayerController* playerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(playerMappingContext, 0);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D movementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		//Add Movement
		movementVector.Normalize();
		AddMovementInput(FVector(1.0f,0.0f,0.0f), (movementVector.Y * 0.5f));
		AddMovementInput(FVector(0.0f, 1.0f, 0.0f), (movementVector.X * 0.5f));
	}
}

void APlayerCharacter::Grab(const FInputActionValue& Value)
{
	bool tryingToGrab = Value.Get<bool>();

	if (itemTouched && tryingToGrab)
	{
		APickupObject* item = Cast<APickupObject>(itemTouched);
		if (item->myPlayer == nullptr && !holdingObject)
		{
			item->myPlayerAttachPoint = AttachPoint;
			item->myPlayer = this;
			heldItem = item;
			holdingObject = true;
		}
	}
}

void APlayerCharacter::Drop(const FInputActionValue& Value)
{
	bool dropPressed = Value.Get<bool>();
	if (dropPressed)
	{
		APickupObject* item = Cast<APickupObject>(heldItem);
		if (holdingObject)
		{
			item->myPlayerAttachPoint = nullptr;
			item->myPlayer = nullptr;
			heldItem = nullptr;
			holdingObject = false;
			dropPressed = false;
			itemTouched = nullptr;
		}
	}
	else {

	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponenet = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponenet->BindAction(moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		EnhancedInputComponenet->BindAction(grabAction, ETriggerEvent::Started, this, &APlayerCharacter::Grab);

		EnhancedInputComponenet->BindAction(dropAction, ETriggerEvent::Started, this, &APlayerCharacter::Drop);
	}
}

//Collision With Another Collision Box (i guess it doesnt have to be a box but whatever)
void APlayerCharacter::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		if (OtherActor->ActorHasTag("Item"))
		{
			itemTouched = OtherActor;
		}
	}
	else {
		itemTouched = nullptr;
	}
}
