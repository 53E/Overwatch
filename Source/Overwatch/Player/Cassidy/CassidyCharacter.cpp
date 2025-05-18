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

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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
	
	// 무기 메시 설정 - 부모 클래스의 FPWeaponMesh를 사용
	FPWeaponMesh->SetRelativeLocation(FVector(20.0f, 10.0f, -10.0f));
	FPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    
    // 3인칭 메시 설정
    TPWeaponMesh->SetRelativeLocation(FVector(20.0f, 10.0f, -10.0f));
    TPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
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
	}
}

// 발사 시작 (Enhanced Input)
void ACassidyCharacter::StartFire(const FInputActionValue& Value)
{
	if (!IsDead())
	{
		FireWeapon();
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
	// 사망, 재장전 중, 탄약 부족, 팬 파이어 중, 구르기 중이면 발사 불가
	if (IsDead() || bIsReloading || CurrentAmmo <= 0 || bIsFanFiring || bIsDodging)
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
    
    // 클라이언트와 서버 모두에서 실행
	bIsFanFiring = true;

	
	// 남은 총알만큼 연사
	int32 bulletsToFire = CurrentAmmo;
	CurrentAmmo = 0; 
	// 나머지 총알 타이머로 발사
	for (FTimerHandle& Handle : FanFireTimerHandles)
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle);
	}
	FanFireTimerHandles.Empty();
	
	// 즉시 첫 발 발사
	UE_LOG(LogTemp, Log, TEXT("FanFire: 발사 번호 1"));
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
	// 사망, 최대 탄약, 재장전 중, 팬 파이어 중이면 리턴
	if (IsDead() || CurrentAmmo == MaxAmmo || bIsReloading || bIsFanFiring)
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
	// 사망, 팬 파이어 중, 재장전 중, 총알 부족이면 발사 불가
	if (IsDead() || bIsFanFiring || bIsReloading || CurrentAmmo <= 0)
	{
		// 총알이 없고 재장전 중이 아니면 자동 재장전
		if (CurrentAmmo <= 0 && !bIsReloading && !bIsFanFiring)
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
    if (GetLocalRole() < ROLE_Authority)
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
	
	
    // 서버 측 코드 (리슨 서버 호스트 또는 데디케이티드 서버)
    
    // 총알 소모 (제거됨 - ServerFireBullet에서 처리되기 때문에)
    
    // 카메라 정보 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    
    // 팬 파이어의 경우 랜덤 각도 적용
    if (bIsFanFireShot)
    {
        // 랜덤 편차 계산
        float PitchSpread = FMath::RandRange(-FanFireMaxSpreadAngle, FanFireMaxSpreadAngle);
        float YawSpread = FMath::RandRange(-FanFireMaxSpreadAngle, FanFireMaxSpreadAngle);
        
        // 원래 회전에 랜덤 편차 추가
        CameraRotation.Pitch += PitchSpread;
        CameraRotation.Yaw += YawSpread;
    }
    
    FVector TraceStart = CameraLocation;
    FVector TraceDirection = CameraRotation.Vector();
    FVector TraceEnd = TraceStart + (TraceDirection * WeaponRange);
    
    // 충돌 쿼리 파라미터 설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;
    
    // 명중 결과
    FHitResult HitResult;
    
    // 라인 트레이스 실행
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    // 효과 재생 (모든 클라이언트에 전파) - 발사 위치와 방향 정보 추가
    MulticastPlayFireEffects(bIsFanFireShot, TraceStart, TraceDirection);
    
    // 반동 효과 (로컬 클라이언트에서만)
    if (IsLocallyControlled())
    {
        Recoil(bIsFanFireShot);
    }
    
    //UE_LOG(LogTemp, Log, TEXT("남은 탄환 : %d"), CurrentAmmo);
    
    // 트레이서 이펙트 추가
    MulticastPlayTracerEffect(TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd);
    
    // 명중 시 데미지 처리 (서버에서만 수행)
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

	MulticastPlayTracerEffect(StartLocation, TraceEnd);
    
    /* 서버 로컬 디버그 라인 그리기 (리슨 서버일 경우)
    if (IsLocallyControlled())
    {
        FColor LineColor = bIsFanFireShot ? FColor::Yellow : FColor::Red;
        DrawDebugLine(GetWorld(), StartLocation, bHit ? HitResult.ImpactPoint : TraceEnd, LineColor, false, 2.0f, 0, 1.0f);
    }
	*/
    
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
	// 사망, 쿨타임 중, 팬 파이어 중, 재장전 중, 구르기 중이면 사용 불가
	if (IsDead() || bFlashbangOnCooldown || bIsFanFiring || bIsReloading || bIsDodging)
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
        
        // 범위 내 액터에 섬광 효과 적용
        TArray<AActor*> OverlappingActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), OverlappingActors);
        
        for (AActor* Actor : OverlappingActors)
        {
            // 자기 자신은 제외
            if (Actor != this)
            {
                // 거리 계산
                float Distance = FVector::Dist(FlashbangLocation, Actor->GetActorLocation());
                
                // 범위 내에 있으면 효과 적용 (이후 구현)
                if (Distance <= FlashbangRadius)
                {
                    // AOverwatchCharacter* OverwatchCharacter = Cast<AOverwatchCharacter>(Actor);
                    // if (OverwatchCharacter)
                    // {
                    //     // 섬광 효과 적용
                    // }
                }
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
	// 사망, 구르기 중, 쿨타임 중, 팬 파이어 중이면 불가
	if (IsDead() || bIsDodging || bDodgingOnCooldown || bIsFanFiring)
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
    MulticastPlayDodgeEffects();

	UE_LOG(LogTemp, Log, TEXT("구르기"));
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
	const FVector_NetQuantize& EndLocation)
{
	UNiagaraComponent* TracerEffect = nullptr;
	bool bHit = false;
	FHitResult HitResult;
	if (BulletTracer)
	{
		FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
		float Distance = FVector::Dist(StartLocation, EndLocation);

		// 충돌 테스트 
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_Visibility,
			QueryParams
		);

		FVector MuzzleLocation = FPWeaponMesh->GetSocketLocation(FName("Muzzle"));
		if (PeaceKeeperWeapon)
		{
			
		}
		
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
		TracerEffect->SetVariableVec3(FName("BeamEnd"),bHit? HitResult.ImpactPoint : EndLocation );

		TracerEffect->SetAutoDestroy(true);
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
