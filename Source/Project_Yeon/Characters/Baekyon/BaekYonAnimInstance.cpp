#include "BaekYonAnimInstance.h"

#include "BaekYonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBaekYonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BaekYon = Cast<ABaekYonCharacter>(TryGetPawnOwner());
	if (BaekYon)
	{
		MovementComponent = BaekYon->GetCharacterMovement();
	}
}

void UBaekYonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BaekYon == nullptr)
	{
		BaekYon = Cast<ABaekYonCharacter>(TryGetPawnOwner());
		if (BaekYon)
		{
			MovementComponent = BaekYon->GetCharacterMovement();
		}
	}

	if (MovementComponent == nullptr) return;

	const FVector Velocity = MovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();

	bShouldMove = GroundSpeed > 3.f
		&& !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

	bIsFalling = MovementComponent->IsFalling();
}
