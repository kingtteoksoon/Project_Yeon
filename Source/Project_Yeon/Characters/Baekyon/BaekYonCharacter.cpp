// Copyright Kim seok-hyun, Inc. All Rights Reserved.

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
	// MaxWalkSpeed를 매 프레임 보간(StartRun/StopRun → Tick)해야 하므로 필수.
	PrimaryActorTick.bCanEverTick = true;

	// ── 카메라 ───────────────────────────────────────────────
	// CameraBoom(SpringArm): 캐릭터에 붙어 카메라와의 거리/충돌을 관리하는 Arm.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.f;        // 캐릭터-카메라 거리(cm)
	CameraBoom->bUsePawnControlRotation = true; // 마우스/스틱 입력으로 Boom 회전
	CameraBoom->bEnableCameraLag = true;        // 카메라가 약간 늦게 따라오는 지연 효과
	CameraBoom->CameraLagSpeed = 10.f;          // 지연 속도(클수록 빨리 따라붙음)

	// FollowCamera: Boom 끝에 부착되는 실제 카메라. Boom 회전을 그대로 따라가면
	// 되므로 자체 회전 입력은 받지 않음(false).
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ── 이동 설정 ─────────────────────────────────────────────
	// 컨트롤러(카메라) 회전과 캐릭터 메시 회전을 분리.
	// → 카메라는 자유롭게 돌아가지만 캐릭터 모델은 즉시 따라 돌지 않음.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 이동 방향으로 캐릭터를 자동 회전시키고(bOrientRotationToMovement),
	// 최대 720°/s로 빠르게 방향 전환되도록 설정.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	// 초기 속도 세팅
	// TargetMaxSpeed: Tick에서 MaxWalkSpeed가 보간해서 따라갈 "목표값".
	// 평소엔 WalkSpeed, Run 입력 시 RunSpeed로 StartRun/StopRun에서 갱신됨.
	TargetMaxSpeed = WalkSpeed;

	// 스폰 직후 바로 정상 보행 속도로 시작(가속 워밍업 없음).
	// ※ 단, Tick에서 입력이 없으면 MaxWalkSpeed가 0으로 서서히 수렴하기 때문에,
	//    한참 가만히 있다가 다시 움직이면 다시 0→WalkSpeed로 ~0.4초간 가속되는
	//    "재출발 슬로우" 현상이 매번 발생함(의도한 연출인지 확인 필요).
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ── 점프 ─────────────────────────────────────────────────
	GetCharacterMovement()->JumpZVelocity = 500.f;            // 점프 초기 수직 속도
	GetCharacterMovement()->AirControl = 0.35f;               // 공중에서 방향 전환 가능 비율(0~1)
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f; // 공중에서 입력 없을 때 감속량
	JumpMaxCount = 1; // 2단 점프 등 다단 점프 비허용
}

void ABaekYonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 컨트롤러일 때만 Enhanced Input의 매핑 컨텍스트(키 바인딩 묶음)를 등록.
	// AI(이무기 등)나 컨트롤러 없는 폰은 이 블록을 그냥 건너뜀.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// DefaultMappingContext는 BP에서 에디터로 할당해야 함.
			// 비어있으면 아무 입력도 동작하지 않으므로 BP_BaekYon 설정 확인 필요.
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0); // 0 = 우선순위
			}
		}
	}
}

void ABaekYonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move/Look: 키를 누르고 있는 동안 매 프레임 Triggered 발생
		// → 이동은 RawInputAxis에 값만 저장하고, 실제 처리는 Tick에서 일괄 수행(아래 참고)
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaekYonCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaekYonCharacter::Look);
		}
		// Dash: 버튼을 누르는 "그 순간"(Started)에 한 번만 실행 → Dash() 내부에서 쿨다운 체크
		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started, this, &ABaekYonCharacter::Dash);
		}
		// Jump: 누르면 점프 시작, 떼면 상승 중단(짧게 누르면 낮게 점프)
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		// Run: 누르고 있는 동안 RunSpeed로, 떼면 WalkSpeed로 목표 속도(TargetMaxSpeed) 전환
		if (RunAction)
		{
			EIC->BindAction(RunAction, ETriggerEvent::Started, this, &ABaekYonCharacter::StartRun);
			EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &ABaekYonCharacter::StopRun);
		}
	}
}

// ── 매 프레임 가속도 및 감속 처리 (Tick) ───────────────────────────────────────

void ABaekYonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 입력이 완전히 없으면 목표 속도를 0으로 설정하여 감속 유도, 있으면 걷기/뛰기 상태 추종
	// NOTE: 여기서 CurrentTarget=0이 되면, "캐릭터 정지"가 아니라
	// "MaxWalkSpeed 자체가 0으로 수렴"한다는 점에 주의.
	// → 입력이 없을 때 MaxWalkSpeed가 0까지 떨어졌다가, 다음에 다시 움직이면
	//   0 → TargetMaxSpeed(WalkSpeed/RunSpeed)로 ~0.4초간 재가속해야 함(아래 2번 참고).
	float CurrentTarget = RawInputAxis.IsNearlyZero() ? 0.f : TargetMaxSpeed;

	// 2. FInterpTo로 MaxWalkSpeed를 DeltaTime에 맞춰 부드럽게 조정
	// FInterpTo(Cur, Target, dt, Speed) ≈ 매 프레임 (Target-Cur)의 (dt*Speed) 비율만큼 좁혀감.
	// AccelerationRate=8, 60fps(dt≈0.0166) 기준 한 프레임에 약 13.3%씩 목표값에 근접
	// → 0에서 200(WalkSpeed)까지 도달하는 데 대략 0.4~0.5초 소요.
	float NewSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, CurrentTarget, DeltaTime, AccelerationRate);
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	// 3. 입력이 있거나, 아직 속도가 남아 감속 중일 때 이동력을 계속 컴포넌트에 공급
	// FIXME: Controller가 nullptr인 상태(예: 디스폰/possess 해제 직후, 잔여 속도가
	// 10cm/s 이상 남아있는 경우)에서 이 분기에 진입하면 Controller->GetControlRotation()에서
	// 크래시. Move()는 Controller null 체크를 하지만 Tick은 이 값을 매 프레임 무조건 사용함.
	if (!RawInputAxis.IsNearlyZero() || GetCharacterMovement()->Velocity.Size() > 10.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// NOTE: Dash 중(bIsInvincible)에도 매 프레임 AddMovementInput이 호출되어
		// LaunchCharacter로 부여한 추진력이 CharacterMovementComponent의 가속/감속 처리와
		// 섞일 수 있음.
		// - 이동키를 누른 채 Dash → Acceleration이 생겨 Velocity가 MaxWalkSpeed
		//   (200~600) 쪽으로 빠르게 클램프되며 LaunchCharacter의 1280 추진력이
		//   1~2프레임 만에 깎여 실제 이동거리가 짧아질 수 있음.
		// - 이동키 없이 Dash → Acceleration=0, BrakingDeceleration(기본 2000)으로만
		//   감속되어 이론상 ~410cm로 GDD의 "Dash 400cm"와 비슷하게 나옴.
		// → Dash 거리/느낌이 기획 의도와 다르면 이 부분 확인 필요.
		AddMovementInput(Forward, RawInputAxis.Y); // W/S
		AddMovementInput(Right, RawInputAxis.X); // A/D
	}

	// 4. Enhanced Input 특성에 맞춰 프레임이 끝날 때 입력축 초기화
	// NOTE: 이 리셋이 안전하려면 Move()(Enhanced Input 처리)가 이 Tick보다 먼저
	// 실행된다는 틱 순서 보장이 필요함. PlayerController -> Pawn 순서로 보통 맞지만,
	// 틱 그룹/우선순위가 바뀌면 입력이 한 프레임 누락될 수 있음.
	RawInputAxis = FVector2D::ZeroVector;
}

void ABaekYonCharacter::StartRun()
{
	// 실제 속도 변경은 즉시 일어나지 않고, Tick의 FInterpTo가 이 목표값을 향해
	// MaxWalkSpeed를 매 프레임 보간하면서 따라감(부드러운 가속).
	TargetMaxSpeed = RunSpeed; // 런 키 누르고 있으면 목표 속도를 RunSpeed로 세팅
}

void ABaekYonCharacter::StopRun()
{
	TargetMaxSpeed = WalkSpeed; // 런 키 떼면 목표 속도를 WalkSpeed로 원복
}

// ── 이동 ─────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Move(const FInputActionValue& Value)
{
	// Controller가 없으면(빙의 해제 등) 입력을 무시 — 이동 방향 계산에 컨트롤러 회전이 필요하기 때문.
	if (Controller == nullptr) return;

	// 프레임마다 들어오는 입력값을 저장 (실제 이동 처리는 Tick에서 일괄 계산)
	// X = 좌우(A/D), Y = 전후(W/S) — 컨트롤 매핑(IA_Move) 설정에 따름
	RawInputAxis = Value.Get<FVector2D>();
}

void ABaekYonCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller == nullptr) return;

	// 마우스/스틱 입력을 그대로 컨트롤러 회전(Yaw/Pitch)에 더함.
	// 카메라는 bUsePawnControlRotation=true라 이 회전을 그대로 따라감.
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

// ── Dash ──────────────────────────────────────────────────────────────────────

void ABaekYonCharacter::Dash()
{
	// 쿨다운 중이면 무시 (콤보 3타 완주 시 ResetDashCooldown()으로 즉시 해제됨)
	if (!bCanDash) return;

	bCanDash = false;
	bIsInvincible = true; // DashInvincibilityDuration(0.18s) 동안 피격 무시 — 데미지 처리 쪽에서 IsInvincible() 체크 필요

	// 이동 중이면 입력 방향, 아니면 전방으로 Dash
	// GetLastMovementInputVector(): 가장 최근 AddMovementInput에 누적된 방향(매 프레임 소비 후 리셋됨)
	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector();
	}
	Dir.Z = 0.f;     // 수직 성분 제거 — Dash는 수평 이동만
	Dir.Normalize(); // 방향만 사용, 크기는 DashImpulse로 따로 적용

	// LaunchCharacter로 순간 추진 (bXYOverride=true: 기존 수평 속도를 무시하고 덮어씀,
	// bZOverride=false: 수직 속도는 유지 → 점프/낙하 중에도 자연스럽게 적용)
	LaunchCharacter(Dir * DashImpulse, true, false);

	// 무적 종료 타이머 (0.18초 후 OnDashInvincibilityEnd)
	GetWorldTimerManager().SetTimer(
		DashInvincibilityTimer, this, &ABaekYonCharacter::OnDashInvincibilityEnd,
		DashInvincibilityDuration, false);

	// 쿨다운 종료 타이머 (0.4초 후 OnDashCooldownEnd, 콤보로 더 일찍 끝나면 ResetDashCooldown이 대체)
	GetWorldTimerManager().SetTimer(
		DashCooldownTimer, this, &ABaekYonCharacter::OnDashCooldownEnd,
		DashCooldown, false);
}

// 약공격 3타 콤보 완주 시 Combat 쪽에서 호출 — 남은 쿨다운을 무시하고 즉시 Dash 가능 상태로 복귀
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