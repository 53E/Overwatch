#pragma once

#include "CoreMinimal.h"
#include "OverwatchCharacter.h"
#include "DummyCharacter.generated.h"

/**
 * 훈련장 더미 캐릭터
 */
UCLASS()
class OVERWATCH_API ADummyCharacter : public AOverwatchCharacter
{
	GENERATED_BODY()
	
public:
	ADummyCharacter();

protected:
	virtual void BeginPlay() override;

	// 더미 자동 부활 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dummy")
	bool bAutoRevive;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dummy")
	float ReviveDelay;

	// 부활 타이머
	FTimerHandle ReviveTimerHandle;

public:
	// 죽음 처리 오버라이드

	virtual void Die(AActor* Killer = nullptr) override;

	
	// 부활 함수
	UFUNCTION(NetMulticast, Reliable,Category = "Dummy")
	void Revive();
	void Revive_Implementation();
	
	// 데미지 표시 (블루프린트에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Dummy")
	void OnShowDamage(float Damage, FVector Location);
	
	// 부활 이벤트 (블루프린트에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Dummy")
	void OnRevive();
	
	// Hit 오버라이드하여 데미지 표시
	virtual float Hit(float DamageAmount, AActor* DamageCauser) override;
	
};
