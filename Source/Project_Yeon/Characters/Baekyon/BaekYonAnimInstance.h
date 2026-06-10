#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaekYonAnimInstance.generated.h"

class ABaekYonCharacter;
class UCharacterMovementComponent;

UCLASS()
class PROJECT_YEON_API UBaekYonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ABaekYonCharacter> BaekYon;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// 수평 속력 — 로코모션 블렌드(Idle/Walk/Run)의 입력값
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.f;

	// 이동 입력이 있고 실제로 움직이는 중 — Idle ↔ Locomotion 전환 조건
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bShouldMove = false;

	// 공중 상태 — 점프/낙하 블렌드 조건
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = false;
};
