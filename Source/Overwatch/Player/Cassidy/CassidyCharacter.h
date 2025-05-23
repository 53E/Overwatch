#pragma once

#include "CoreMinimal.h"
#include "../OverwatchCharacter.h"
#include "InputActionValue.h"
#include "CassidyCharacter.generated.h"

class UAnimMontage;
class USoundBase;
class UParticleSystem;
class UInputAction;
class UNiagaraSystem;
class UMaterialInterface;


/**
 * 캐서디 캐릭터 클래스
 * 오버워치 스타일의 1인칭 캐릭터
 */
UCLASS()
class OVERWATCH_API ACassidyCharacter : public AOverwatchCharacter
{
	GENERATED_BODY()

public:
	// 생성자 및 초기화 메서드
	ACassidyCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** 총 발사 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FireAnimation;

	/** 재장전 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnimation;

	/** 총소리 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;
	
	/** 재장전 소리 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;

	/** 구르기 소리 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* DodgeSound;

	/** 총구 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* MuzzleFlash;

	/** 탄피 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ShellEject;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* BulletTracer;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* BlueCircleDecal;
	
	/** 최대 탄창 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;

	/** 현재 탄창 수 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;

	/** 재장전 중인지 여부 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsReloading;

	/** 팬 파이어 중인지 여부 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsFanFiring;

	/** 구르기 중인지 여부 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bIsDodging;

	/** 총격 간 딜레이 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;

	/** 연발 사이의 딜레이 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FanFireRate;

	/** 마지막 발사 시간 */
	float LastFireTime;

	/** 재장전 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadTime;

	/** 무기 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float WeaponDamage;

	/** 무기 사거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float WeaponRange;

	/** 팬 파이어 최대 발산 각도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float FanFireMaxSpreadAngle;

	/** 반동 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilRate;
	
	/** 팬 파이어 반동 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float FanFireRecoilRate;

	/**입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FanFireAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* HighnoonAction;

	/** 섬광탄 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* FlashbangExplosionEffect;

	/** 섬광탄 소리 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FlashbangSound;

	/** 섬광탄 쿨타임 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float FlashbangCooldown;

	/** 섬광탄 쿨타임 상태 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bFlashbangOnCooldown;

	/** 섬광탄 폭발 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float FlashbangRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float HighnoonRadius;

	/** 구르기 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float DodgeSpeed;

	/** 구르기 쿨타임 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float DodgeCooldown;

	/** 구르기 쿨타임 상태 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bDodgingOnCooldown;

	/** 궁극기 게이지 관련 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ultimate")
	float UltimateCharge;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float MaxUltimateCharge;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float UltimateChargePerSecond;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float UltimateChargePerHit;

	/** Highnoon 활성화 상태 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ultimate")
	bool bIsHighnoon;
	
	/** Highnoon 지속 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float HighnoonDuration;
	
	/** Highnoon 타겟팅 시간 (원이 작아지는 시간) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float HighnoonTargetingTime;
	
	/** Highnoon 최대 데미지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float HighnoonMaxDamage;
	
	/** Highnoon 타겟들 */
	UPROPERTY(Replicated)
	TArray<AActor*> HighnoonTargets;
	
	/** 각 타겟의 락온 진행도 */
	TMap<AActor*, float> TargetLockProgress;
	
	/** Highnoon 발사 가능 여부 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ultimate")
	bool bCanFireHighnoon;
	
	/** Highnoon 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* HighnoonActivateSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* HighnoonFireSound;
	
	
	/** 섬광탄 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FlashbangAction;

protected:
	// 액션 입력 처리 함수
	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
	void FanFire(const FInputActionValue& Value);
	void StartReload(const FInputActionValue& Value);
	void ThrowFlashbang(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);
	void StartHighnoon(const FInputActionValue& Value);

	// 실제 총 발사 로직
	void FireWeapon();
	void FireBullet(bool bIsFanFireShot = false);

	// Highnoon 관련 함수들
	void ActivateHighnoon();
	void UpdateHighnoon(float DeltaTime);
	void FireHighnoon();
	void EndHighnoon();
	void ChargeUltimate(float Amount);
	
	// 클라이언트 RPC
	UFUNCTION(Client, Reliable)
	void ClientShowHighnoonUI(const TArray<AActor*>& Targets);
	void ClientShowHighnoonUI_Implementation(const TArray<AActor*>& Targets);
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateHighnoonUI(float Progress);
	void ClientUpdateHighnoonUI_Implementation(float Progress);
	
	UFUNCTION(Client, Reliable)
	void ClientHideHighnoonUI();
	void ClientHideHighnoonUI_Implementation();
	
	// 서버 RPC 구현
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerActivateHighnoon();
	bool ServerActivateHighnoon_Validate();
	void ServerActivateHighnoon_Implementation();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireHighnoon();
	bool ServerFireHighnoon_Validate();
	void ServerFireHighnoon_Implementation();
	
	// 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHighnoonEffects(bool bActivate);
	void MulticastPlayHighnoonEffects_Implementation(bool bActivate);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireBullet(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction, bool bIsFanFireShot);
	bool ServerFireBullet_Validate(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction, bool bIsFanFireShot);
	void ServerFireBullet_Implementation(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction, bool bIsFanFireShot);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();
	bool ServerStartReload_Validate();
	void ServerStartReload_Implementation();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerThrowFlashbang(const FVector_NetQuantize& Location);
	bool ServerThrowFlashbang_Validate(const FVector_NetQuantize& Location);
	void ServerThrowFlashbang_Implementation(const FVector_NetQuantize& Location);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDodge();
	bool ServerDodge_Validate();
	void ServerDodge_Implementation();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFanFire();
	bool ServerFanFire_Validate();
	void ServerFanFire_Implementation();
	
	// 멀티캐스트 RPC 구현 (효과 동기화)
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFireEffects(bool bIsFanFireShot, const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction);
	void MulticastPlayFireEffects_Implementation(bool bIsFanFireShot, const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayReloadEffects();
	void MulticastPlayReloadEffects_Implementation();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFlashbangEffects(const FVector_NetQuantize& Location);
	void MulticastPlayFlashbangEffects_Implementation(const FVector_NetQuantize& Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDodgeEffects();
	void MulticastPlayDodgeEffects_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayTracerEffect(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& EndLocation,bool bHit);
	void MulticastPlayTracerEffect_Implementation(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& EndLocation, bool bHit);
	
	// 구르기 종료 효과를 위한 멀티캐스트 RPC 추가
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDodgeResetEffects();
	void MulticastPlayDodgeResetEffects_Implementation();
	
	// 히트 처리
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerProcessHit(AActor* HitActor, const FVector_NetQuantize& HitLocation);
	bool ServerProcessHit_Validate(AActor* HitActor, const FVector_NetQuantize& HitLocation);
	void ServerProcessHit_Implementation(AActor* HitActor, const FVector_NetQuantize& HitLocation);

	void Flashbang();
	
	
	// 타이머 콜백 함수
	UFUNCTION()
	void FinishReload();
	
	UFUNCTION()
	void FinishDodge();
	
	UFUNCTION()
	void FinishFlashbangCooldown();

	// 블루프린트 구현 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Dodge")
	void DownCameraOnDodge();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dodge")
	void DodgeMontage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	void Recoil(bool bIsFanFire);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void FanFireUI(float time);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void StopFanFireUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Fire")
	void FireSoundEcho();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate")
	void ShowHighnoonTargetUI(AActor* Target, float InitialSize);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate")
	void UpdateHighnoonTargetUI(AActor* Target, float Progress);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate")
	void HideHighnoonTargetUI(AActor* Target);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate")
	void OnHighnoonReady();
	

private:
	// 타이머 핸들
	FTimerHandle TimerHandle_Reload;
	FTimerHandle TimerHandle_FlashbangCooldown;
	FTimerHandle TimerHandle_HighnoonDuration;
	FTimerHandle TimerHandle_HighnoonUpdate;
	
	//Faanfire 핸들러 배열
	TArray<FTimerHandle> FanFireTimerHandles;
	
	// 섬광탄 위치
	FVector FlashbangLocation;
	
	// Highnoon 시작 시간
	float HighnoonStartTime;
};
