#include "BaekYonCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ABaekYonCharacter::ABaekYonCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; 

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
	JumpMaxCount = 1;
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
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (RunAction)
		{
			EIC->BindAction(RunAction, ETriggerEvent::Started,   this, &ABaekYonCharacter::StartRun);
			EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &ABaekYonCharacter::StopRun);
		}
	}
}

void ABaekYonCharacter::StartRun()
{
	GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(MaxWalkSpeed, RunSpeed, DeltaTime, InterpSpeed);
}

void ABaekYonCharacter::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(MaxWalkSpeed, WalkSpeed, DeltaTime, InterpSpeed);
}

// ── 이동 ─────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Move(const FInputActionValue& Value float DeltaTime)
{
	Super::Tick(DeltaTime);
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr || Axis.IsNearlyZero()) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y); 
	AddMovementInput(Right,   Axis.X);
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

	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector();
	}
	Dir.Z = 0.f;
	Dir.Normalize();

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
