#include "BaekYonCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ABaekYonCharacter::ABaekYonCharacter()
{
	// ── Tick 활성화 ───────────────────────────────────────────
	PrimaryActorTick.bCanEverTick = true;

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
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	
	// 초기 속도 세팅
	TargetMaxSpeed = WalkSpeed; 
	GetCharacterMovement()->MaxWalkSpeed = 0.f; // 완전히 멈춘 상태에서 시작 (혹은 WalkSpeed)

	// ── 점프 ─────────────────────────────────────────────────
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

// ── 매 프레임 가속도 및 감속 처리 (Tick) ───────────────────────────────────────

void ABaekYonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 입력이 완전히 없으면 목표 속도를 0으로 설정하여 감속 유도, 있으면 걷기/뛰기 상태 추종
	float CurrentTarget = RawInputAxis.IsNearlyZero() ? 0.f : TargetMaxSpeed;

	// 2. FInterpTo로 MaxWalkSpeed를 DeltaTime에 맞춰 부드럽게 조정
	float NewSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, CurrentTarget, DeltaTime, AccelerationRate);
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	// 3. 입력이 있거나, 아직 속도가 남아 감속 중일 때 이동력을 계속 컴포넌트에 공급
	if (!RawInputAxis.IsNearlyZero() || GetCharacterMovement()->Velocity.Size() > 10.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, RawInputAxis.Y); // W/S
		AddMovementInput(Right,   RawInputAxis.X); // A/D
	}

	// 4. Enhanced Input 특성에 맞춰 프레임이 끝날 때 입력축 초기화
	RawInputAxis = FVector2D::ZeroVector;
}

void ABaekYonCharacter::StartRun()
{
	TargetMaxSpeed = RunSpeed; // 런 키 누르고 있으면 목표 속도를 RunSpeed로 세팅
}

void ABaekYonCharacter::StopRun()
{
	TargetMaxSpeed = WalkSpeed; // 런 키 떼면 목표 속도를 WalkSpeed로 원복
}

// ── 이동 ─────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	// 프레임마다 들어오는 입력값을 저장 (실제 이동 처리는 Tick에서 일괄 계산)
	RawInputAxis = Value.Get<FVector2D>();
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

	// LaunchCharacter로 순간 추진
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
