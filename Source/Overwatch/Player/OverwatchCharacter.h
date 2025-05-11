#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OverwatchCharacter.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class UInputComponent;
class UAnimMontage;
class USoundBase;
class UInputMappingContext;
class UInputAction;

/**
 * 오버워치 캐릭터 베이스 클래스
 * 모든 오버워치 캐릭터의 기본 기능 제공
 */
UCLASS()
class OVERWATCH_API AOverwatchCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자 및 초기화 메서드
	AOverwatchCharacter();
	virtual void BeginPlay() override;
	// 네트워크 속성 복제 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	float Hit(float DamageAmount, AActor* DamageCauser);

	/** 캐릭터 사망 처리 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Die();
	
	/** 체력 회복 처리 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Heal(float HealAmount);

protected:
	/** 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	/** 1인칭 무기 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* FPWeaponMesh;
	
	/** 3인칭 무기 메시 (다른 플레이어에게 보이는 메시) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* TPWeaponMesh;

	/** 현재 체력 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentHealth;

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;
	
	/** 아머 (방어력) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Armor;
	
	/** 쉴드 (재생 가능한 보호막) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Shield;
	
	/** 최대 쉴드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxShield;
	
	/** 쉴드 재생 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float ShieldRegenRate;
	
	/** 쉴드 재생 시작 전 딜레이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float ShieldRegenDelay;
	
	/** 사망 상태 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bIsDead;

	/** 기본 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseWalkSpeed;

	/** Enhanced Input 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	/** 이동 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** 시점 이동 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	/** 점프 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// 쉴드 재생 타이머
	FTimerHandle TimerHandle_ShieldRegen;

protected:
	// 체력 및 쉴드 관련 함수
	void StartShieldRegen();
	void RegenShield();
	
	// Enhanced Input 입력 처리 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:
	// 체력 및 상태 액세서
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetArmor() const { return Armor; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE bool IsDead() const { return bIsDead; }
	
	// 데미지 이벤트 (블루프린트에서 구현 가능)
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDamaged(float DamageAmount, AActor* DamageCauser);
	
	// 사망 이벤트 (블루프린트에서 구현 가능)
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDeath();
};
