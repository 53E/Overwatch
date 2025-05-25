#include "CassidyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
// Enhanced Input 관련 헤더
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
// 무작위 값 생성을 위한 헤더
#include "Math/UnrealMathUtility.h"
// 네트워크 관련 헤더
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "PeaceKeeper.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInterface.h"
#include "Components/DecalComponent.h"
#include "../OverwatchCharacter.h"

// 생성자
ACassidyCharacter::ACassidyCharacter()
{
	// 매 프레임 Tick() 호출 설정
	PrimaryActorTick.bCanEverTick = true;
    
    // 네트워크 복제 활성화
    bReplicates = true;
    SetReplicates(true);
    SetReplicateMovement(true);

	// 캐서디 특성에 맞게 기본 스탯 설정
	MaxHealth = 225.0f;  // 캐서디의 체력
	Armor = 0.0f;        // 아머 없음
	MaxShield = 0.0f;    // 쉴드 없음
	ShieldRegenRate = 0.0f;
	
	// 무기 및 능력 변수 초기화
	MaxAmmo = 6;           // 캐서디는 6발 리볼버
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
	bIsFanFiring = false;
	bIsDodging = false;
	bIsHighnoon = false;
	FireRate = 0.5f;      // 0.5초마다 발사 가능
	FanFireRate = 0.15f;  // 연사 발사 속도
	LastFireTime = 0.0f;
	ReloadTime = 1.5f;    // 재장전 시간 1.5초
	WeaponDamage = 70.0f; // 기본 데미지
	WeaponRange = 5000.0f; // 사거리
	FanFireMaxSpreadAngle = 4.0f; // 팬 파이어 최대 발산 각도 (도 단위)

	// 반동 설정
	RecoilRate = 1.15f;
	FanFireRecoilRate = 1.75f;

	// 능력 쿨타임 설정
	DodgeSpeed = 2100.0f;
	DodgeCooldown = 2.0f;
	FlashbangCooldown = 2.0f;
	
	bFlashbangOnCooldown = false;
	bDodgingOnCooldown = false;
	FlashbangRadius = 130.0f; // 폭발 범위
	HighnoonRadius = 10000.0f;
	
	// 궁극기 관련 설정
	UltimateCharge = 0.0f;
	MaxUltimateCharge = 100.0f;
	UltimateChargePerSecond = 100.0f; // 초당 1% 충전
	UltimateChargePerHit = 5.0f; // 명중 시 5% 충전
	bIsHighnoon = false;
	bCanFireHighnoon = false;
	HighnoonDuration = 6.0f; // 6초 지속
	HighnoonTargetingTime = 3.0f; // 원이 작아지는 시간
	HighnoonMaxDamage = 999.0f; // 최대 데미지
	
	// 무기 메시 설정 - 부모 클래스의 FPWeaponMesh를 사용
	FPWeaponMesh->SetRelativeLocation(FVector(20.0f, 10.0f, -10.0f));
	FPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	
}

// 복제할 변수 등록
void ACassidyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 복제할 변수들 등록
    DOREPLIFETIME(ACassidyCharacter, CurrentAmmo);
    DOREPLIFETIME(ACassidyCharacter, bIsReloading);
    DOREPLIFETIME(ACassidyCharacter, bIsFanFiring);
    DOREPLIFETIME(ACassidyCharacter, bIsDodging);
    DOREPLIFETIME(ACassidyCharacter, bFlashbangOnCooldown);
    DOREPLIFETIME(ACassidyCharacter, bDodgingOnCooldown);
    DOREPLIFETIME(ACassidyCharacter, UltimateCharge);
    DOREPLIFETIME(ACassidyCharacter, bIsHighnoon);
    DOREPLIFETIME(ACassidyCharacter, bCanFireHighnoon);
    DOREPLIFETIME(ACassidyCharacter, HighnoonTargets);
}

// 게임 시작시 호출
void ACassidyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 탄창 설정
	CurrentAmmo = MaxAmmo;
}

// 매 프레임 호출
void ACassidyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 서버에서만 처리
	if (GetLocalRole() == ROLE_Authority)
	{
		// 살아있고 Highnoon이 아닐 때만 궁극기 충전
		if (!IsDead() && !bIsHighnoon && UltimateCharge < MaxUltimateCharge)
		{
			ChargeUltimate(UltimateChargePerSecond * DeltaTime);
		}
		
		// Highnoon 활성화 중
		if (bIsHighnoon)
		{
			UpdateHighnoon(DeltaTime);
		}
	}
}

// 입력 바인딩
void ACassidyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 부모 클래스의 입력 바인딩을 먼저 호출 (이동, 점프 등)
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 로컬 플레이어만 입력 처리
    if (!IsLocallyControlled())
        return;

	// Enhanced Input Component로 캐스팅
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (EnhancedInputComponent)
	{
		// 캐서디 특화 입력 바인딩
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ACassidyCharacter::StopFire);
		EnhancedInputComponent->BindAction(FanFireAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::FanFire);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::StartReload);
		EnhancedInputComponent->BindAction(FlashbangAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::ThrowFlashbang);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::Dodge);
		EnhancedInputComponent->BindAction(HighnoonAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::StartHighnoon);
	}
}

// 발사 시작 (Enhanced Input)
void ACassidyCharacter::StartFire(const FInputActionValue& Value)
{
	if (!IsDead())
	{
		// Highnoon 중일 때 처리
		if (bIsHighnoon && bCanFireHighnoon)
		{
			FireHighnoon();
		}
		else if (!bIsHighnoon)
		{
			FireWeapon();
		}
	}
}

// 발사 중단 (Enhanced Input)
void ACassidyCharacter::StopFire(const FInputActionValue& Value)
{
	// 필요시 연속 발사 중단 코드 추가
}

// 우클릭 연사 (Enhanced Input)
void ACassidyCharacter::FanFire(const FInputActionValue& Value)
{
	// 사망, 재장전 중, 탄약 부족, 팬 파이어 중, 구르기 중, Highnoon 중이면 발사 불가
	if (IsDead() || bIsReloading || CurrentAmmo <= 0 || bIsFanFiring || bIsDodging || bIsHighnoon)
	{
		return;
	}

    // 서버 RPC 호출
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerFanFire();
        if (IsLocallyControlled())
        {
            FanFireUI((CurrentAmmo + 5) * FanFireRate);
        }
    	return;
    }
    
    // 서버 
	bIsFanFiring = true;

	
	// 남은 총알만큼 연사
	int32 bulletsToFire = CurrentAmmo;

	// 나머지 총알 타이머로 발사
	for (FTimerHandle& Handle : FanFireTimerHandles)
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}
	FanFireTimerHandles.Empty();
	
	// 즉시 첫 발 발사
	//UE_LOG(LogTemp, Log, TEXT("FanFire: 발사 번호 1"));
	FireBullet(true);
	
	// 나머지 총알 타이머로 발사
	for (int32 i = 1; i < bulletsToFire; i++)
	{
		FTimerHandle TimerHandle;
		int32 bulletIndex = i;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this, bulletIndex]() { 
				UE_LOG(LogTemp, Log, TEXT("FanFire: 발사 번호 %d"), bulletIndex+1);
				FireBullet(true); 
			},
			FanFireRate * i,
			false
		);
		FanFireTimerHandles.Add(TimerHandle);
	}
	
	// 팬 파이어 종료 타이머 설정
	FTimerHandle TimerHandle_FinishFanFire;
	GetWorld()->GetTimerManager().SetTimer(
	TimerHandle_FinishFanFire,
	[this]() {
	 bIsFanFiring = false;
	  if (CurrentAmmo <= 0)
			{
				StartReload(FInputActionValue());
			}
		},
		FanFireRate * bulletsToFire,
		false
	);
}

// 서버에서 FanFire 실행
bool ACassidyCharacter::ServerFanFire_Validate()
{
    return true;
}

void ACassidyCharacter::ServerFanFire_Implementation()
{
    if (!IsDead() && !bIsReloading && CurrentAmmo > 0 && !bIsFanFiring && !bIsDodging)
    {
        FanFire(FInputActionValue());
    }
}

// 재장전 시작 (Enhanced Input)
void ACassidyCharacter::StartReload(const FInputActionValue& Value)
{
	// 사망, 최대 탄약, 재장전 중, 팬 파이어 중, Highnoon 중이면 리턴
	if (IsDead() || CurrentAmmo == MaxAmmo || bIsReloading || bIsFanFiring || bIsHighnoon)
	{
		return;
	}

    // 서버 RPC 호출
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerStartReload();
        return;
    }

	bIsReloading = true;
	UE_LOG(LogTemp, Log, TEXT("장전 시작"));

    // 모든 클라이언트에 효과 전파
    MulticastPlayReloadEffects();

	// 재장전 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Reload, 
		this, 
		&ACassidyCharacter::FinishReload, 
		ReloadTime, 
		false
	);
}

// 서버에서 재장전 실행
bool ACassidyCharacter::ServerStartReload_Validate()
{
    return true;
}

void ACassidyCharacter::ServerStartReload_Implementation()
{
    if (!IsDead() && CurrentAmmo < MaxAmmo && !bIsReloading && !bIsFanFiring)
    {
        StartReload(FInputActionValue());
    }
}

// 모든 클라이언트에 재장전 효과 전파
void ACassidyCharacter::MulticastPlayReloadEffects_Implementation()
{
    // 재장전 애니메이션 재생
    if (ReloadAnimation)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(ReloadAnimation, 1.0f);
        }
    }

    // 재장전 사운드 재생
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
    }
}

// 재장전 완료
void ACassidyCharacter::FinishReload()
{
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
	UE_LOG(LogTemp, Log, TEXT("장전 완료"));
}

// 무기 발사 처리
void ACassidyCharacter::FireWeapon()
{
	// 사망, 팬 파이어 중, 재장전 중, 총알 부족, Highnoon 중이면 발사 불가
	if (IsDead() || bIsFanFiring || bIsReloading || CurrentAmmo <= 0 || bIsHighnoon)
	{
		// 총알이 없고 재장전 중이 아니면 자동 재장전
		if (CurrentAmmo <= 0 && !bIsReloading && !bIsFanFiring && !bIsHighnoon)
		{
			StartReload(FInputActionValue());
		}
		return;
	}

	// 발사 가능 여부 확인 (발사 속도 고려)
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastFireTime) < FireRate)
	{
		return;
	}

	LastFireTime = CurrentTime;
	
	// 실제 발사 처리 (일반 발사)
	FireBullet(false);
}

// 총알 발사 로직
void ACassidyCharacter::FireBullet(bool bIsFanFireShot)
{
    // 로컬 클라이언트일 경우 (서버가 아닌 경우)
    if (1)//GetLocalRole() < ROLE_Authority)
    {
        // 카메라 정보 가져오기
        FVector CameraLocation;
        FRotator CameraRotation;
        GetActorEyesViewPoint(CameraLocation, CameraRotation);
        
        FVector TraceStart = CameraLocation;
        FVector TraceDirection = CameraRotation.Vector();
    	
        // 서버에 RPC 호출
        ServerFireBullet(TraceStart, TraceDirection, bIsFanFireShot);
    	
        
        return;
    }
	
	
    // 서버 측 코드 
	

}

// 궁극기 충전 함수
void ACassidyCharacter::ChargeUltimate(float Amount)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		float OldCharge = UltimateCharge;
		UltimateCharge = FMath::Clamp(UltimateCharge + Amount, 0.0f, MaxUltimateCharge);
		
		// 궁극기가 충전 완료되면 알림
		if (OldCharge < MaxUltimateCharge && UltimateCharge >= MaxUltimateCharge)
		{
			if (IsLocallyControlled())
			{
				OnHighnoonReady();
			}
		}
	}
}

// Highnoon 활성화 함수
void ACassidyCharacter::ActivateHighnoon()
{
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Log, TEXT("Activating Highnoon..."));
		ServerActivateHighnoon();
	}
}

// Highnoon 업데이트 (서버에서만)
void ACassidyCharacter::UpdateHighnoon(float DeltaTime)
{
	if (!bIsHighnoon || GetLocalRole() != ROLE_Authority)
		return;
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - HighnoonStartTime;
	float LockProgress = FMath::Clamp(ElapsedTime / HighnoonTargetingTime, 0.0f, 1.0f);
	
	// 모든 타겟의 락온 진행도 업데이트
	bool bAllTargetsLocked = true;
	for (AActor* Target : HighnoonTargets)
	{
		if (Target && !Target->IsPendingKillPending())
		{
			TargetLockProgress.Add(Target, LockProgress);
			if (LockProgress < 1.0f)
			{
				bAllTargetsLocked = false;
			}
		}
	}
	
	// 클라이언트에 UI 업데이트 알림 do once
	ClientUpdateHighnoonUI(LockProgress);
	
	// 모든 타겟이 락온되면 발사 가능
	if (bAllTargetsLocked && !bCanFireHighnoon)
	{
		bCanFireHighnoon = true;
		UE_LOG(LogTemp, Log, TEXT("Highnoon ready to fire!"));
	}
}

// Highnoon 발사
void ACassidyCharacter::FireHighnoon()
{
	if (!bIsHighnoon || !bCanFireHighnoon)
		return;
	
	if (IsLocallyControlled())
	{
		ServerFireHighnoon();
	}
}

// Highnoon 종료
void ACassidyCharacter::EndHighnoon()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsHighnoon = false;
		bCanFireHighnoon = false;
		HighnoonTargets.Empty();
		TargetLockProgress.Empty();
		UltimateCharge = 0.0f; // 궁극기 사용 후 초기화
		
		// 타이머 해제
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HighnoonDuration);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HighnoonUpdate);
		
		// 효과 중단
		MulticastPlayHighnoonEffects(false);
		
		// UI 숨기기
		ClientHideHighnoonUI();
	}
}

// 클라이언트 UI RPC
void ACassidyCharacter::ClientShowHighnoonUI_Implementation(const TArray<AActor*>& Targets)
{
	for (AActor* Target : Targets)
	{
		if (Target)
		{
			ShowHighnoonTargetUI(Target, 200.0f); // 초기 크기 200
		}
	}
}

void ACassidyCharacter::ClientUpdateHighnoonUI_Implementation(float Progress)
{
	for (AActor* Target : HighnoonTargets)
	{
		if (Target)
		{
			UpdateHighnoonTargetUI(Target, Progress);
		}
	}
}

void ACassidyCharacter::ClientHideHighnoonUI_Implementation()
{
	for (AActor* Target : HighnoonTargets)
	{
		if (Target)
		{
			HideHighnoonTargetUI(Target);
		}
	}
}

// 서버 RPC
bool ACassidyCharacter::ServerActivateHighnoon_Validate()
{
	return true;
}

void ACassidyCharacter::ServerActivateHighnoon_Implementation()
{
	// 궁극기 사용 가능 여부 확인
	if (!IsDead() && UltimateCharge >= MaxUltimateCharge && !bIsHighnoon)
	{
		bIsHighnoon = true;
		bCanFireHighnoon = false;
		HighnoonStartTime = GetWorld()->GetTimeSeconds();
		
		// 현재 위치에서 범위 내 적 찾기
		FVector Location = GetActorLocation();
		TArray<FOverlapResult> OverlapResults;
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(HighnoonRadius);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		
		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			Location,
			FQuat::Identity,
			ECC_Visibility,
			CollisionShape,
			QueryParams
		);

		// 시야에 있는 적만 타겟으로 추가
		for (const FOverlapResult& Result : OverlapResults)
		{
			AOverwatchCharacter* Target = Cast<AOverwatchCharacter>(Result.GetActor());
			if (Target && Target != this && !Target->IsDead())
			{
				// 시야 체크
				FVector DirectionToTarget = (Target->GetActorLocation() - Location).GetSafeNormal();
				FVector ForwardVector = GetActorForwardVector();
				float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);
				
				// 전방 90도 범위 내에 있는 적만 타겟으로
				if (DotProduct > 0.0f)
				{
					HighnoonTargets.Add(Target);
				}
			}
		}
		
		// 효과 재생
		MulticastPlayHighnoonEffects(true);
		
		// UI 표시
		ClientShowHighnoonUI(HighnoonTargets);
		
		// 지속 시간 타이머
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_HighnoonDuration,
			this,
			&ACassidyCharacter::EndHighnoon,
			HighnoonDuration,
			false
		);
		
		UE_LOG(LogTemp, Log, TEXT("Highnoon activated with %d targets"), HighnoonTargets.Num());
	}
}

bool ACassidyCharacter::ServerFireHighnoon_Validate()
{
	return true;
}

void ACassidyCharacter::ServerFireHighnoon_Implementation()
{
	if (!bIsHighnoon || !bCanFireHighnoon)
		return;
	
	// 모든 락온된 타겟에게 데미지
	for (AActor* Target : HighnoonTargets)
	{
		AOverwatchCharacter* OverwatchTarget = Cast<AOverwatchCharacter>(Target);
		if (OverwatchTarget && !OverwatchTarget->IsDead())
		{
			// 즉사 데미지
			OverwatchTarget->Hit(HighnoonMaxDamage, this);
			UE_LOG(LogTemp, Log, TEXT("하이눈 발사"));
			
			// 탄환 효과
			FVector StartLocation = GetActorLocation();
			FVector EndLocation = OverwatchTarget->GetActorLocation();
			MulticastPlayTracerEffect(StartLocation, EndLocation, true);
		}
	}
	
	// 발사 사운드
	if (HighnoonFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HighnoonFireSound, GetActorLocation());
	}
	
	// Highnoon 종료
	EndHighnoon();
}

// 멀티캐스트 RPC
void ACassidyCharacter::MulticastPlayHighnoonEffects_Implementation(bool bActivate)
{
	if (bActivate)
	{
		// Highnoon 활성화 사운드
		if (HighnoonActivateSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HighnoonActivateSound, GetActorLocation());
		}
		
		// 캡슐 컬리전 업데이트 (필요한 경우)
		// GetCapsuleComponent()->SetCollisionResponseToChannel(...);
	}
	else
	{
		// 효과 종료
	}
}

// 서버에서 총알 발사 처리
bool ACassidyCharacter::ServerFireBullet_Validate(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction, bool bIsFanFireShot)
{
    return true;
}

void ACassidyCharacter::ServerFireBullet_Implementation(const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction, bool bIsFanFireShot)
{
    // 사망, 팬 파이어 중, 재장전 중, 총알 부족이면 발사 불가

    if (IsDead() || (bIsFanFireShot == false && bIsFanFiring) || bIsReloading || CurrentAmmo <= 0)
    {
        return;
    }
    
    // 총알 소모
    CurrentAmmo--;
	UE_LOG(LogTemp, Log, TEXT("남은 탄환 : %d (ServerFireBullet)"), CurrentAmmo);
    
    // 효과 재생 (모든 클라이언트에 전파)
    MulticastPlayFireEffects(bIsFanFireShot, StartLocation, Direction);
    
    // 트레이스 엔드 포인트 계산 파라미터의 Dir말고 다시계산 why? 전달하면서 값 오류
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
	if (bIsFanFireShot)
	{
		// 랜덤 편차 계산
		float PitchSpread = FMath::RandRange(-FanFireMaxSpreadAngle, FanFireMaxSpreadAngle);
		float YawSpread = FMath::RandRange(-FanFireMaxSpreadAngle, FanFireMaxSpreadAngle);
            
		// 원래 회전에 랜덤 편차 추가
		CameraRotation.Pitch += PitchSpread;
		CameraRotation.Yaw += YawSpread;
	}
	FVector TraceDirection = CameraRotation.Vector();
	
    FVector TraceEnd = StartLocation + (TraceDirection * WeaponRange);
    
    // 충돌 쿼리 파라미터 설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;
    
    // 명중 결과
    FHitResult HitResult;
    
    // 라인 트레이스 실행
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

	MulticastPlayTracerEffect(StartLocation, bHit? HitResult.ImpactPoint : TraceEnd, bHit);
	
    
    // 명중 시 데미지 처리
    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            // 데미지 적용 - Hit 함수 호출
            ServerProcessHit(HitActor, HitResult.ImpactPoint);
        }
    }
    
    // 총알을 모두 소모했고 팬 파이어가 아니면 자동 재장전
    if (CurrentAmmo <= 0 && !bIsFanFiring)
    {
        StartReload(FInputActionValue());
    }
}

// 서버에서 히트 처리
bool ACassidyCharacter::ServerProcessHit_Validate(AActor* HitActor, const FVector_NetQuantize& HitLocation)
{
    return true;
}

void ACassidyCharacter::ServerProcessHit_Implementation(AActor* HitActor, const FVector_NetQuantize& HitLocation)
{
    if (HitActor && !IsDead() && GetLocalRole() == ROLE_Authority)
    {
        // hit 함수를 사용하도록 수정된 코드
        AOverwatchCharacter* OverwatchCharacter = Cast<AOverwatchCharacter>(HitActor);
        if (OverwatchCharacter)
        {
            OverwatchCharacter->Hit(WeaponDamage, this);
            
            // 명중 시 궁극기 충전
            ChargeUltimate(UltimateChargePerHit);
        }
    }
}

// 모든 클라이언트에 총알 발사 효과 전파
void ACassidyCharacter::MulticastPlayFireEffects_Implementation(bool bIsFanFireShot, const FVector_NetQuantize& StartLocation, const FVector_NetQuantize& Direction)
{
	// 로컬 반동 효과 추가
	if (IsLocallyControlled())
	{
		Recoil(bIsFanFireShot);

		AActor* ChildActor = PeaceKeeperWeapon->GetChildActor();
		APeaceKeeper* Pistol = Cast<APeaceKeeper>(ChildActor);
		if (Pistol)
		{
			Pistol->Recoil(bIsFanFireShot);
		}
		
	}
	
    // 발사 애니메이션 재생
    if (FireAnimation)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(FireAnimation, 1.0f);
        }
    }

    // 발사 사운드 재생
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    	if (!bIsFanFireShot)
    	{
    		FireSoundEcho();
    	}

    }

    // 총구 이펙트 재생
    if (MuzzleFlash)
    {
        const FVector MuzzleLocation = FPWeaponMesh->GetSocketLocation(FName("Muzzle"));
        const FRotator MuzzleRotation = FPWeaponMesh->GetSocketRotation(FName("Muzzle"));
        
    	UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlash,                       
			FPWeaponMesh,                        
			FName("Muzzle"),                   
			FVector::ZeroVector,              
			FRotator::ZeroRotator,               
			EAttachLocation::SnapToTarget,      
			true                                  
		);
    }

    // 탄피 이펙트 재생
    if (ShellEject)
    {
        const FVector EjectLocation = FPWeaponMesh->GetSocketLocation(FName("ShellEjectSocket"));
        const FRotator EjectRotation = FPWeaponMesh->GetSocketRotation(FName("ShellEjectSocket"));
        
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEject, EjectLocation, EjectRotation);
    }
    
    // 디버그 라인은 클라이언트 측에서만 그림 (멀티캐스트에서는 제거)
    // 여기서 그리면 이상한 방향으로 나가는 문제가 발생
}

// 섬광탄 던지기
void ACassidyCharacter::ThrowFlashbang(const FInputActionValue& Value)
{
	// 사망, 쿨타임 중, 팬 파이어 중, 재장전 중, 구르기 중, Highnoon 중이면 사용 불가
	if (IsDead() || bFlashbangOnCooldown || bIsFanFiring || bIsReloading || bIsDodging || bIsHighnoon)
	{
		return;
	}
	
    // 플레이어 시점에서 약간 앞쪽 위치 계산
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);

    // 전방 벡터 계산
    FVector ForwardVector = CameraRotation.Vector();

    // 약간 앞쪽에 섬광탄이 터지는 위치 계산
    FlashbangLocation = CameraLocation + (ForwardVector * 500.0f);
    
    // 서버 RPC 호출
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerThrowFlashbang(FlashbangLocation);
        return;
    }
    
	// 쿨타임 설정
	bFlashbangOnCooldown = true;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_FlashbangCooldown,
		this,
		&ACassidyCharacter::FinishFlashbangCooldown,
		FlashbangCooldown,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("섬광탄 사용"));

	// 지연 후 섬광탄 폭발 설정
	FTimerHandle TimerHandle_Flashbang;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Flashbang,
		this,
		&ACassidyCharacter::Flashbang,
		0.12f,
		false
	);
}

// 서버에서 섬광탄 처리
bool ACassidyCharacter::ServerThrowFlashbang_Validate(const FVector_NetQuantize& Location)
{
    return true;
}

void ACassidyCharacter::ServerThrowFlashbang_Implementation(const FVector_NetQuantize& Location)
{
    if (!IsDead() && !bFlashbangOnCooldown && !bIsFanFiring && !bIsReloading && !bIsDodging)
    {
        FlashbangLocation = Location;
        ThrowFlashbang(FInputActionValue());
    }
}

// 섬광탄 폭발 효과
void ACassidyCharacter::Flashbang()
{
    // 서버에서 효과 전파
    if (GetLocalRole() == ROLE_Authority)
    {
        MulticastPlayFlashbangEffects(FlashbangLocation);
        
		TArray<FOverlapResult> OverlapResults;
    	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(FlashbangRadius);
    	FCollisionQueryParams QueryParams;
    	QueryParams.AddIgnoredActor(this);

    	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		FlashbangLocation,
		FQuat::Identity,
		ECC_Visibility,
		CollisionShape,
		QueryParams
    	);
		
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor)
			{
				UE_LOG(LogTemp, Log, TEXT("%s 섬광탄 적용"),*HitActor->GetName());
			}
			
		}
    	
    }
}

// 모든 클라이언트에 섬광탄 효과 전파
void ACassidyCharacter::MulticastPlayFlashbangEffects_Implementation(const FVector_NetQuantize& Location)
{
    // 섬광탄 이펙트 재생
    if (FlashbangExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FlashbangExplosionEffect, Location, FRotator::ZeroRotator);
    }

    // 섬광탄 소리 재생
    if (FlashbangSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FlashbangSound, Location);
    }
    
    // 디버그 표시 (개발용)
    //DrawDebugSphere(GetWorld(), Location, FlashbangRadius, 16, FColor::Yellow, false, 2.0f);
}

// 섬광탄 쿨타임 완료
void ACassidyCharacter::FinishFlashbangCooldown()
{
	bFlashbangOnCooldown = false;
	UE_LOG(LogTemp, Log, TEXT("섬광탄 쿨타임 완료"));
}

// 구르기 입력 처리
void ACassidyCharacter::Dodge(const FInputActionValue& Value)
{
	// 사망, 구르기 중, 쿨타임 중, 팬 파이어 중, Highnoon 중이면 불가
	if (IsDead() || bIsDodging || bDodgingOnCooldown || bIsFanFiring || bIsHighnoon)
	{
		return;
	}
	
    // 서버 RPC 호출
    if (GetLocalRole() < ROLE_Authority)
    {
    	StopFanFireUI();
        ServerDodge();
        return;
    }
    
	bIsDodging = true;
	bDodgingOnCooldown = true;


	// 재장전 중이면 중단하고 즉시 장전 완료
	if (bIsReloading)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Reload);
		FinishReload();
	}

	// 쿨타임 타이머 설정
	FTimerHandle TimerHandle_DodgeCooldown;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_DodgeCooldown,
		[this]() { bDodgingOnCooldown = false; },
		DodgeCooldown,
		false
	);

	// 구르기 종료 타이머 설정
	FTimerHandle TimerHandle_Dodge;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Dodge,
		this,
		&ACassidyCharacter::FinishDodge,
		0.25f,
		false
	);

	// 모든 클라이언트에 효과 전파
	DodgeMontage();
    MulticastPlayDodgeEffects();

	UE_LOG(LogTemp, Log, TEXT("구르기"));
}

void ACassidyCharacter::StartHighnoon(const FInputActionValue& Value)
{
	if (!IsDead() && !bIsReloading && !bIsDodging && !bIsFanFiring && UltimateCharge >= MaxUltimateCharge && !bIsHighnoon)
	{
		ActivateHighnoon();
	}
}

// 서버에서 구르기 처리
bool ACassidyCharacter::ServerDodge_Validate()
{
    return true;
}

void ACassidyCharacter::ServerDodge_Implementation()
{
    if (!IsDead() && !bIsDodging && !bDodgingOnCooldown && !bIsFanFiring)
    {
        Dodge(FInputActionValue());
    }
}

// 모든 클라이언트에 구르기 효과 전파
void ACassidyCharacter::MulticastPlayDodgeEffects_Implementation()
{
    // 이동 속도 증가
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->MaxWalkSpeed = DodgeSpeed;
	MovementComp->MaxAcceleration = 50000.0f;

    // 카메라 효과 (로컬 플레이어만)
    if (IsLocallyControlled())
    {
        DownCameraOnDodge();
    }

    // 구르기 사운드 재생
    if (DodgeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, GetActorLocation());
    }
}

void ACassidyCharacter::MulticastPlayTracerEffect_Implementation(const FVector_NetQuantize& StartLocation,
	const FVector_NetQuantize& EndLocation,bool bHit)
{
	UNiagaraComponent* TracerEffect = nullptr;
	FHitResult HitResult;
	FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
	if (BulletTracer)
	{
		float Distance = FVector::Dist(StartLocation, EndLocation);

		FVector MuzzleLocation = FPWeaponMesh->GetSocketLocation(FName("Muzzle"));
		
		TracerEffect =UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		BulletTracer,
		MuzzleLocation,
		Direction.Rotation(),
		FVector(1.f),
		true                    // bAutoDestroy
		);
	}

	if (TracerEffect)
	{
		TracerEffect->SetVariableVec3(FName("BeamEnd"), EndLocation );

		TracerEffect->SetAutoDestroy(true);
	}
	
	if (BlueCircleDecal && bHit)
	{
		UDecalComponent* HitDecal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		BlueCircleDecal,
		FVector(5,5,5),
		EndLocation,
		Direction.Rotation(),
		0.75f
		);

		if (HitDecal)
		{
			//HitDecal->FadeScreenSize = 0.0f; 버그
			HitDecal->SetFadeScreenSize(0.0f);

			//HitDecal->SetFadeOut(0.75f,1.25,true);
		}
	}
}

// 모든 클라이언트에 구르기 종료 효과 전파
void ACassidyCharacter::MulticastPlayDodgeResetEffects_Implementation()
{
    // 이동 속도 원상복구
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    MovementComp->MaxWalkSpeed = BaseWalkSpeed;
    MovementComp->MaxAcceleration = 2048.0f;
}

// 구르기 종료
void ACassidyCharacter::FinishDodge()
{
	bIsDodging = false;
	CurrentAmmo = MaxAmmo; // 구르기 후 자동 재장전

    // 모든 클라이언트에 효과 전파
    MulticastPlayDodgeResetEffects();

	UE_LOG(LogTemp, Log, TEXT("구르기 끝"));
}
