// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareDevCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "NightmareDevGameMode.h"
#include "NightmareEnemySpawner.h"
#include "NightmareFlowGameMode.h"
#include "NightmareInventoryComponent.h"
#include "NightmareItemInstance.h"
#include "NightmareMatchComponent.h"
#include "NightmarePickupActor.h"
#include "NightmarePlayerEffectComponent.h"
#include "NightmareProjectileActor.h"
#include "NightmareSpawnScheduler.h"
#include "NightmareStaminaComponent.h"
#include "UObject/ConstructorHelpers.h"

ANightmareDevCharacter::ANightmareDevCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CollectRadius = 200.0f;
	bInvertLookY = false;
	SelectedInventorySlot = 0;
	ProjectileClass = ANightmareProjectileActor::StaticClass();
	ProjectileSpeed = 6000.0f;
	ProjectileDamage = 5.0f;
	ProjectileTrajectory = ENightmareProjectileTrajectory::StraightLine;
	ProjectileHalfExtents = FVector(40.0f, 8.0f, 8.0f);
	ProjectileSpawnForwardOffset = 120.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	GrayboxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxBody"));
	GrayboxBody->SetupAttachment(GetCapsuleComponent());
	GrayboxBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxBody->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
	GrayboxBody->SetRelativeScale3D(FVector(0.55f, 0.35f, 0.9f));
	if (CubeMesh.Succeeded())
	{
		GrayboxBody->SetStaticMesh(CubeMesh.Object);
	}

	GrayboxHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrayboxHead"));
	GrayboxHead->SetupAttachment(GrayboxBody);
	GrayboxHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrayboxHead->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	GrayboxHead->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.55f));
	if (CubeMesh.Succeeded())
	{
		GrayboxHead->SetStaticMesh(CubeMesh.Object);
	}

	Stamina = CreateDefaultSubobject<UNightmareStaminaComponent>(TEXT("NightmareStamina"));
	Inventory = CreateDefaultSubobject<UNightmareInventoryComponent>(TEXT("NightmareInventory"));
	Match = CreateDefaultSubobject<UNightmareMatchComponent>(TEXT("NightmareMatch"));
	PlayerEffects = CreateDefaultSubobject<UNightmarePlayerEffectComponent>(TEXT("NightmarePlayerEffects"));
}

float ANightmareDevCharacter::ComputeLookPitchInput(float MouseY, bool bInvertY)
{
	return bInvertY ? MouseY : -MouseY;
}

bool ANightmareDevCharacter::IsGameplaySimulationActive() const
{
	if (const ANightmareFlowGameMode* FlowGM = GetWorld() ? GetWorld()->GetAuthGameMode<ANightmareFlowGameMode>() : nullptr)
	{
		return FlowGM->IsGameplayActive();
	}
	return true;
}

void ANightmareDevCharacter::ResetForNewRound()
{
	if (Stamina)
	{
		Stamina->ResetStamina();
	}
	if (Match)
	{
		Match->ResetMatch();
	}
	if (Inventory)
	{
		Inventory->ClearAll();
	}
	if (PlayerEffects)
	{
		PlayerEffects->ClearAllEffects(GetCharacterMovement());
	}
	SelectedInventorySlot = 0;
	ClampSelectedInventorySlot();
}

void ANightmareDevCharacter::EnsureInputAssets()
{
	if (MoveAction && LookAction && JumpAction && CollectAction && UseSelectedSlotAction && SelectSlot0Action
		&& SelectSlot1Action && SelectSlot2Action && FireAction && MappingContext)
	{
		return;
	}

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_DevMove"), RF_Transient);
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = NewObject<UInputAction>(this, TEXT("IA_DevLook"), RF_Transient);
	LookAction->ValueType = EInputActionValueType::Axis2D;

	JumpAction = NewObject<UInputAction>(this, TEXT("IA_DevJump"), RF_Transient);
	JumpAction->ValueType = EInputActionValueType::Boolean;

	CollectAction = NewObject<UInputAction>(this, TEXT("IA_DevCollect"), RF_Transient);
	CollectAction->ValueType = EInputActionValueType::Boolean;

	UseSelectedSlotAction = NewObject<UInputAction>(this, TEXT("IA_DevUseSelectedSlot"), RF_Transient);
	UseSelectedSlotAction->ValueType = EInputActionValueType::Boolean;

	SelectSlot0Action = NewObject<UInputAction>(this, TEXT("IA_DevSelectSlot0"), RF_Transient);
	SelectSlot0Action->ValueType = EInputActionValueType::Boolean;

	SelectSlot1Action = NewObject<UInputAction>(this, TEXT("IA_DevSelectSlot1"), RF_Transient);
	SelectSlot1Action->ValueType = EInputActionValueType::Boolean;

	SelectSlot2Action = NewObject<UInputAction>(this, TEXT("IA_DevSelectSlot2"), RF_Transient);
	SelectSlot2Action->ValueType = EInputActionValueType::Boolean;

	FireAction = NewObject<UInputAction>(this, TEXT("IA_DevFire"), RF_Transient);
	FireAction->ValueType = EInputActionValueType::Boolean;

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Dev"), RF_Transient);

	auto AddAxis2DKey = [this](UInputAction* Action, const FKey& Key, bool bNegate, bool bSwizzleToY)
	{
		FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(Action, Key);
		if (bSwizzleToY)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(MappingContext);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(MappingContext);
			Mapping.Modifiers.Add(Negate);
		}
	};

	AddAxis2DKey(MoveAction, EKeys::W, false, true);
	AddAxis2DKey(MoveAction, EKeys::S, true, true);
	AddAxis2DKey(MoveAction, EKeys::D, false, false);
	AddAxis2DKey(MoveAction, EKeys::A, true, false);

	MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	MappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	MappingContext->MapKey(CollectAction, EKeys::E);
	MappingContext->MapKey(UseSelectedSlotAction, EKeys::F);
	MappingContext->MapKey(SelectSlot0Action, EKeys::One);
	MappingContext->MapKey(SelectSlot1Action, EKeys::Two);
	MappingContext->MapKey(SelectSlot2Action, EKeys::Three);
	MappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
}

void ANightmareDevCharacter::AddMappingContext()
{
	EnsureInputAssets();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void ANightmareDevCharacter::SyncProjectileDefaultsFromBaselines()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		ProjectileSpeed = ANightmareProjectileActor::ComputeDefaultSpeed(Movement->MaxWalkSpeed);
	}
	if (Stamina)
	{
		ProjectileDamage = ANightmareProjectileActor::ComputeDefaultDamage(Stamina->GetMaxStamina());
	}
}

void ANightmareDevCharacter::BeginPlay()
{
	Super::BeginPlay();
	EnsureInputAssets();
	ClampSelectedInventorySlot();
	if (PlayerEffects)
	{
		PlayerEffects->CaptureBaselineFromMovement(GetCharacterMovement());
	}
	SyncProjectileDefaultsFromBaselines();
}

void ANightmareDevCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddMappingContext();
}

void ANightmareDevCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsGameplaySimulationActive())
	{
		if (Stamina)
		{
			Stamina->TickStamina(DeltaSeconds);
		}
		if (Match)
		{
			Match->TickMatch(DeltaSeconds, Stamina);
		}
		if (PlayerEffects)
		{
			PlayerEffects->TickEffects(DeltaSeconds, GetCharacterMovement());
		}
	}

	ClampSelectedInventorySlot();
	DrawDevStatusHud();
}

void ANightmareDevCharacter::ClampSelectedInventorySlot()
{
	const int32 SlotCount = Inventory ? Inventory->GetSlotCount() : 1;
	SelectedInventorySlot = FMath::Clamp(SelectedInventorySlot, 0, FMath::Max(0, SlotCount - 1));
}

void ANightmareDevCharacter::SelectInventorySlot(int32 SlotIndex)
{
	SelectedInventorySlot = SlotIndex;
	ClampSelectedInventorySlot();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			202,
			1.0f,
			FColor::Cyan,
			FString::Printf(TEXT("Selected slot %d"), SelectedInventorySlot));
	}
}

void ANightmareDevCharacter::SelectSlot0() { SelectInventorySlot(0); }
void ANightmareDevCharacter::SelectSlot1() { SelectInventorySlot(1); }
void ANightmareDevCharacter::SelectSlot2() { SelectInventorySlot(2); }

void ANightmareDevCharacter::StartJump() { Jump(); }
void ANightmareDevCharacter::StopJump() { StopJumping(); }

void ANightmareDevCharacter::TryFireProjectile()
{
	if (!IsGameplaySimulationActive())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !FollowCamera || !ProjectileClass)
	{
		return;
	}

	const FRotator AimRot = FollowCamera->GetComponentRotation();
	const FVector Direction = AimRot.Vector();
	const FVector SpawnLoc = FollowCamera->GetComponentLocation() + Direction * ProjectileSpawnForwardOffset;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANightmareProjectileActor* Projectile = World->SpawnActor<ANightmareProjectileActor>(
		ProjectileClass,
		SpawnLoc,
		AimRot,
		Params);
	if (!Projectile)
	{
		return;
	}

	Projectile->ConfigureProjectile(
		Direction,
		ProjectileSpeed,
		ProjectileDamage,
		ProjectileTrajectory,
		ProjectileHalfExtents);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			203,
			0.5f,
			FColor::White,
			FString::Printf(TEXT("Fire! spd=%.0f dmg=%.0f"), ProjectileSpeed, ProjectileDamage));
	}
}

void ANightmareDevCharacter::DrawDevStatusHud() const
{
	if (!GEngine)
	{
		return;
	}

	const float StaminaNow = Stamina ? Stamina->GetCurrentStamina() : 0.0f;
	const float StaminaMax = Stamina ? Stamina->GetMaxStamina() : 0.0f;
	GEngine->AddOnScreenDebugMessage(
		100,
		0.0f,
		FColor::Green,
		FString::Printf(TEXT("Stamina: %.1f / %.1f"), StaminaNow, StaminaMax));

	FString InvLine = FString::Printf(TEXT("Inventory (sel=%d):"), SelectedInventorySlot);
	if (Inventory)
	{
		for (int32 Index = 0; Index < Inventory->GetSlotCount(); ++Index)
		{
			const bool bSelected = (Index == SelectedInventorySlot);
			const UNightmareItemInstance* Item = Inventory->GetSlotItem(Index);
			if (Item)
			{
				InvLine += FString::Printf(
					TEXT(" %s[%d]%s x%d%s"),
					bSelected ? TEXT(">") : TEXT(" "),
					Index,
					*Item->GetItemId().ToString(),
					Item->GetRemainingUses(),
					bSelected ? TEXT("<") : TEXT(""));
			}
			else
			{
				InvLine += FString::Printf(
					TEXT(" %s[%d]-%s"),
					bSelected ? TEXT(">") : TEXT(" "),
					Index,
					bSelected ? TEXT("<") : TEXT(""));
			}
		}
	}
	GEngine->AddOnScreenDebugMessage(101, 0.0f, FColor::Cyan, InvLine);

	FString EffectLine = TEXT("Effects: none");
	if (PlayerEffects)
	{
		EffectLine = FString::Printf(
			TEXT("Effects: speed=%.1fs jump=%.1fs"),
			PlayerEffects->GetSpeedTimeRemaining(),
			PlayerEffects->GetJumpTimeRemaining());
	}
	GEngine->AddOnScreenDebugMessage(104, 0.0f, FColor::Magenta, EffectLine);

	FString MatchLine = TEXT("Match: ?");
	if (Match)
	{
		switch (Match->GetMatchState())
		{
		case ENightmareMatchState::Playing:
			MatchLine = FString::Printf(TEXT("Match: Playing  t=%.1fs"), Match->GetElapsedSeconds());
			break;
		case ENightmareMatchState::Won:
			MatchLine = TEXT("Match: WON");
			break;
		case ENightmareMatchState::Failed:
			MatchLine = TEXT("Match: FAILED");
			break;
		}
	}
	GEngine->AddOnScreenDebugMessage(102, 0.0f, FColor::Yellow, MatchLine);

	FString EnemyLine = TEXT("Enemies: (no spawner)");
	if (const ANightmareDevGameMode* DevGM = Cast<ANightmareDevGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr))
	{
		if (const ANightmareEnemySpawner* Spawner = DevGM->GetEnemySpawner())
		{
			const float NextIn = Spawner->GetScheduler() ? Spawner->GetScheduler()->GetTimeUntilNextSpawn() : -1.0f;
			EnemyLine = FString::Printf(
				TEXT("Enemies: alive=%d  next=%.1fs"),
				Spawner->GetAliveEnemyCount(),
				NextIn);
		}
	}
	GEngine->AddOnScreenDebugMessage(105, 0.0f, FColor::Orange, EnemyLine);

	GEngine->AddOnScreenDebugMessage(
		103,
		0.0f,
		FColor::White,
		FString::Printf(
			TEXT("WASD Move  Space Jump  LMB Fire(%.0f/%.0f)  E Collect  1/2/3 Select  F Use  |  Touch enemy=hit"),
			ProjectileSpeed,
			ProjectileDamage));
}

void ANightmareDevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	EnsureInputAssets();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("NightmareDevCharacter expects EnhancedInputComponent (check DefaultInput.ini)."));
		return;
	}

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANightmareDevCharacter::Move);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANightmareDevCharacter::Look);
	EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ANightmareDevCharacter::StartJump);
	EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ANightmareDevCharacter::StopJump);
	EIC->BindAction(CollectAction, ETriggerEvent::Started, this, &ANightmareDevCharacter::TryCollectNearbyPickups);
	EIC->BindAction(UseSelectedSlotAction, ETriggerEvent::Started, this, &ANightmareDevCharacter::TryUseSelectedInventorySlot);
	EIC->BindAction(SelectSlot0Action, ETriggerEvent::Started, this, &ANightmareDevCharacter::SelectSlot0);
	EIC->BindAction(SelectSlot1Action, ETriggerEvent::Started, this, &ANightmareDevCharacter::SelectSlot1);
	EIC->BindAction(SelectSlot2Action, ETriggerEvent::Started, this, &ANightmareDevCharacter::SelectSlot2);
	EIC->BindAction(FireAction, ETriggerEvent::Started, this, &ANightmareDevCharacter::TryFireProjectile);
}

void ANightmareDevCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr)
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void ANightmareDevCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(ComputeLookPitchInput(Axis.Y, bInvertLookY));
}

void ANightmareDevCharacter::TryCollectNearbyPickups()
{
	if (!Inventory || !GetWorld())
	{
		return;
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANightmarePickupActor::StaticClass(), Found);
	const FVector MyLocation = GetActorLocation();
	const float RadiusSq = CollectRadius * CollectRadius;
	int32 NearbyUncollected = 0;
	int32 CollectedCount = 0;
	int32 RejectedFull = 0;
	int32 RejectedTouchOnly = 0;

	for (AActor* Actor : Found)
	{
		ANightmarePickupActor* Pickup = Cast<ANightmarePickupActor>(Actor);
		if (!Pickup || Pickup->IsCollected())
		{
			continue;
		}
		if (FVector::DistSquared(MyLocation, Pickup->GetActorLocation()) > RadiusSq)
		{
			continue;
		}

		++NearbyUncollected;
		if (Pickup->GetItemDef().InteractMode != ENightmareItemInteractMode::HoldToUse)
		{
			++RejectedTouchOnly;
			continue;
		}
		if (Inventory->IsFull())
		{
			++RejectedFull;
			continue;
		}
		if (Pickup->TryCollectInto(Inventory))
		{
			++CollectedCount;
		}
		else if (Inventory->IsFull())
		{
			++RejectedFull;
		}
	}

	if (GEngine)
	{
		if (CollectedCount > 0)
		{
			GEngine->AddOnScreenDebugMessage(
				200,
				2.0f,
				FColor::Green,
				FString::Printf(TEXT("Collected %d item(s)"), CollectedCount));
		}
		else if (RejectedFull > 0 || (NearbyUncollected > 0 && Inventory->IsFull()))
		{
			GEngine->AddOnScreenDebugMessage(200, 1.5f, FColor::Orange, TEXT("Inventory full — cannot collect"));
		}
		else if (RejectedTouchOnly > 0 && CollectedCount == 0)
		{
			GEngine->AddOnScreenDebugMessage(200, 1.5f, FColor::Orange, TEXT("Nearby item is TouchInstant — walk into it"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(200, 1.5f, FColor::Orange, TEXT("Nothing to collect"));
		}
	}
}

void ANightmareDevCharacter::TryUseSelectedInventorySlot()
{
	if (!Inventory || !Stamina)
	{
		return;
	}

	ClampSelectedInventorySlot();
	const int32 SlotIndex = SelectedInventorySlot;
	const float StaminaBefore = Stamina->GetCurrentStamina();
	const bool bUsed = Inventory->TryUseSlot(SlotIndex, Stamina, PlayerEffects);
	if (GEngine)
	{
		if (bUsed)
		{
			const float Delta = Stamina->GetCurrentStamina() - StaminaBefore;
			GEngine->AddOnScreenDebugMessage(
				201,
				2.0f,
				FColor::Green,
				FString::Printf(
					TEXT("Used slot%d  stamina %+0.1f -> %.1f"),
					SlotIndex,
					Delta,
					Stamina->GetCurrentStamina()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(
				201,
				1.5f,
				FColor::Orange,
				FString::Printf(TEXT("Use slot%d failed (empty?)"), SlotIndex));
		}
	}
}
