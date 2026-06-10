#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaekYonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PROJECT_YEON_API ABaekYonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaekYonCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ── 카메라 ───────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ── 입력 에셋 (BP_BaekYon에서 할당) ─────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;

	// ── 이동 ─────────────────────────────────────────────────
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// 기본=걷기(WalkSpeed). RunAction 홀드 중 뛰기(RunSpeed)
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0"))
	float WalkSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0"))
	float RunSpeed = 600.f;

	void StartRun();
	void StopRun();

	// ── Dash ─────────────────────────────────────────────────

	// GDD 7.3: 400cm, 쿨다운 0.4초, 무적 0.18초
	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashImpulse = 1280.f; // 400cm 도달 근사값 — 플레이테스트 후 확정

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashCooldown = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashInvincibilityDuration = 0.18f;

	void Dash();

public:
	// 약공격 3타 완주 시 외부(콤보 시스템)에서 호출 — GDD 7.3
	UFUNCTION(BlueprintCallable, Category = "Dash")
	void ResetDashCooldown();

	UFUNCTION(BlueprintPure, Category = "Dash")
	bool CanDash() const { return bCanDash; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInvincible() const { return bIsInvincible; }

private:
	bool bCanDash = true;
	bool bIsInvincible = false;

	FTimerHandle DashCooldownTimer;
	FTimerHandle DashInvincibilityTimer;

	void OnDashCooldownEnd();
	void OnDashInvincibilityEnd();
};
