#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeathComponent.generated.h"

/**
 * 캐릭터 죽음 처리를 위한 컴포넌트
 * 래그돌 물리와 죽음 효과를 관리
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OVERWATCH_API UDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeathComponent();

protected:
	virtual void BeginPlay() override;

	// 래그돌 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FName RootBoneName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float ImpulseStrength;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	bool bApplyImpulse;

public:
	// 래그돌 활성화
	UFUNCTION(NetMulticast, Reliable,Category = "Death")
	void ActivateRagdoll(ACharacter* Character, FVector ImpulseDirection = FVector::ZeroVector);
	void ActivateRagdoll_Implementation(ACharacter* Character, FVector ImpulseDirection = FVector::ZeroVector);
	
	// 래그돌 비활성화
	UFUNCTION(NetMulticast, Reliable,Category = "Death")
	void DeactivateRagdoll(ACharacter* Character);
	void DeactivateRagdoll_Implementation(ACharacter* Character);
	
	// 죽음 처리
	UFUNCTION(BlueprintCallable, Category = "Death")
	void HandleDeath(ACharacter* Character, AActor* Killer = nullptr);
};
