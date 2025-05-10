#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CassidyCharacter.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class UInputComponent;
class UAnimMontage;
class USoundBase;
class UParticleSystem;
class UInputMappingContext;
class UInputAction;

/**
 * 캐서디 캐릭터 클래스
 * 오버워치 스타일의 1인칭 캐릭터
 */
UCLASS()
class OVERWATCH_API ACassidyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// 생성자 및 초기화 메서드
	ACassidyCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	/** 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	/** 1인칭 메시 (총) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* FPGunMesh;

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

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* DodgeSound;

	/** 총구 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* MuzzleFlash;

	/** 탄피 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ShellEject;

	/** 최대 탄창 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;

	/** 현재 탄창 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;

	/** 재장전 중인지 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsReloading;

	/** 팬 파이어 중인지 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsFanFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
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
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Weapon")
	float FanFireMaxSpreadAngle;

	//반동
	UPROPERTY(EditAnywhere , BlueprintReadWrite,Category = "Recoil")
	float RecoilRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float FanFireRecoilRate;

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

	/** 발사 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	/** 연사 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FanFireAction;

	/** 재장전 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* DodgeAction;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bFlashbangOnCooldown;

	/** 섬광탄 폭발 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float FlashbangRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float DodgeSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float DodgeCooldown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
	bool bDodgingOnCooldown;

	// 입력 액션에 추가:
	/** 섬광탄 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* FlashbangAction;

	// 함수 선언 추가:
	void ThrowFlashbang(const FInputActionValue& Value);
	void Flashbang();

	// 타이머 핸들 및 완료 함수:
	FTimerHandle TimerHandle_FlashbangCooldown;

	FVector FlashbangLocation;

	UFUNCTION()
	void FinishFlashbangCooldown();

	UFUNCTION()
	void Dodge();

	UFUNCTION()
	void FinishDodge();


protected:
	// Enhanced Input 입력 처리 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
	void FanFire(const FInputActionValue& Value);
	void StartReload(const FInputActionValue& Value);


	// 실제 총 발사 로직
	void FireWeapon();
	void FireBullet(bool bIsFanFireShot = false);
	
	// 재장전 완료
	UFUNCTION()
	void FinishReload();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dodge")
	void DownCameraOnDodge();

	UFUNCTION(BlueprintImplementableEvent , Category = "Camera")
	void Recoil(bool bIsFanFire);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void FanFireUI(float time);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void StopFanFireUI();

private:
	// 타이머 핸들
	FTimerHandle TimerHandle_Reload;

	float MaxWalkSpeed;


};
