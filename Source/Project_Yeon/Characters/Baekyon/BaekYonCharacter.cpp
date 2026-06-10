#include "BaekYonCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ABaekYonCharacter::ABaekYonCharacter()
{
	// ── 카메라 ───────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ── 이동 설정 ─────────────────────────────────────────────
	// 이동 방향으로 캐릭터가 회전 (카메라와 독립)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ABaekYonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ABaekYonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaekYonCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaekYonCharacter::Look);
		}
		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started, this, &ABaekYonCharacter::Dash);
		}
	}
}

// ── 이동 ─────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr || Axis.IsNearlyZero()) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.X);  // W/S → X축
	AddMovementInput(Right,   Axis.Y);  // D/A → Y축
}

void ABaekYonCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr) return;

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

// ── Dash ──────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Dash()
{
	if (!bCanDash) return;

	bCanDash = false;
	bIsInvincible = true;

	// 이동 중이면 입력 방향, 아니면 전방으로 Dash
	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector();
	}
	Dir.Z = 0.f;
	Dir.Normalize();

	// LaunchCharacter로 순간 추진 — 실제 이동 거리는 플레이테스트 후 DashImpulse로 조정
	LaunchCharacter(Dir * DashImpulse, true, false);

	GetWorldTimerManager().SetTimer(
		DashInvincibilityTimer, this, &ABaekYonCharacter::OnDashInvincibilityEnd,
		DashInvincibilityDuration, false);

	GetWorldTimerManager().SetTimer(
		DashCooldownTimer, this, &ABaekYonCharacter::OnDashCooldownEnd,
		DashCooldown, false);
}

void ABaekYonCharacter::ResetDashCooldown()
{
	GetWorldTimerManager().ClearTimer(DashCooldownTimer);
	bCanDash = true;
}

void ABaekYonCharacter::OnDashCooldownEnd()
{
	bCanDash = true;
}

void ABaekYonCharacter::OnDashInvincibilityEnd()
{
	bIsInvincible = false;
}
