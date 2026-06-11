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

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

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

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0"))
	float WalkSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0"))
	float RunSpeed = 600.f;

	void StartRun();
	void StopRun();

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashImpulse = 1280.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashCooldown = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0"))
	float DashInvincibilityDuration = 0.18f;

	void Dash();

public:
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

	FVector2D RawInputAxis = FVector2D::ZeroVector; 
	float TargetMaxSpeed = 200.f;                   

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.1"))
	float AccelerationRate = 8.0f;   
};