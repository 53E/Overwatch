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

// 생성자
ACassidyCharacter::ACassidyCharacter()
{
	// 매 프레임 Tick() 호출 설정
	PrimaryActorTick.bCanEverTick = true;

	// 카메라 컴포넌트 생성
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f)); // 눈 높이 위치
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 1인칭 메시 컴포넌트 생성 (총)
	FPGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPGun"));
	FPGunMesh->SetOnlyOwnerSee(true); // 소유자만 볼 수 있음
	FPGunMesh->SetupAttachment(FirstPersonCamera);
	FPGunMesh->CastShadow = false;
	FPGunMesh->SetRelativeLocation(FVector(20.0f, 10.0f, -10.0f));
	FPGunMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	// 변수 초기화
	MaxWalkSpeed = 600;
	DodgeSpeed = 2100;
	MaxAmmo = 6; // 캐서디는 6발 리볼버
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
	bIsFanFiring = false;
	bIsDodging = false;
	FireRate = 0.5f; 
	FanFireRate = 0.15f; // 연사 발사 속도
	LastFireTime = 0.0f;
	ReloadTime = 1.5f; // 재장전 시간 1.5초
	WeaponDamage = 70.0f; // 기본 데미지
	WeaponRange = 5000.0f; // 사거리
	FanFireMaxSpreadAngle = 7.5f; // 팬 파이어 최대 발산 각도 (도 단위)

	RecoilRate = 1.15f;
	FanFireRecoilRate = 1.75f;

	DodgeCooldown = 2.0f;

	FlashbangCooldown = 2.0f;
	bFlashbangOnCooldown = false;
	bDodgingOnCooldown = false;
	FlashbangRadius = 130.0f; // 폭발 범위 

}

// 게임 시작시 호출
void ACassidyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 탄창 설정
	CurrentAmmo = MaxAmmo;

	// Enhanced Input 매핑 컨텍스트 추가
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			//Subsystem->ClearMappingContext(DefaultMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// 매 프레임 호출
void ACassidyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 입력 바인딩
void ACassidyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (EnhancedInputComponent)
	{
		// 이동 입력 바인딩
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::Move);
		
		// 시점 이동 입력 바인딩
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::Look);
		
		// 점프 입력 바인딩
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		
		// 발사 입력 바인딩
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ACassidyCharacter::StopFire);
		
		// 연사 입력 바인딩
		EnhancedInputComponent->BindAction(FanFireAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::FanFire);
		
		// 재장전 입력 바인딩
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::StartReload);
		// 섬광탄 입력 바인딩
		EnhancedInputComponent->BindAction(FlashbangAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::ThrowFlashbang);

		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ACassidyCharacter::Dodge);
	}
}

// 이동 입력 처리 (Enhanced Input)
void ACassidyCharacter::Move(const FInputActionValue& Value)
{
	// 2D 벡터 입력값 가져오기
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// 전방/후방 이동
		if (MovementVector.Y != 0.0f)
		{
			AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		}
		
		// 좌/우 이동
		if (MovementVector.X != 0.0f)
		{
			AddMovementInput(GetActorRightVector(), MovementVector.X);
		}
	}
}

// 시점 이동 입력 처리 (Enhanced Input)
void ACassidyCharacter::Look(const FInputActionValue& Value)
{
	// 2D 벡터 입력값 가져오기
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// 좌우 시점 이동
		if (LookAxisVector.X != 0.0f)
		{
			AddControllerYawInput(LookAxisVector.X);
		}
		
		// 상하 시점 이동
		if (LookAxisVector.Y != 0.0f)
		{
			AddControllerPitchInput(LookAxisVector.Y);
		}
	}
}

// 발사 시작 (Enhanced Input)
void ACassidyCharacter::StartFire(const FInputActionValue& Value)
{
	FireWeapon();
}

// 발사 중단 (Enhanced Input)
void ACassidyCharacter::StopFire(const FInputActionValue& Value)
{
	// 필요시 연속 발사 중단 코드 추가
}

// 우클릭 연사 (Enhanced Input)
void ACassidyCharacter::FanFire(const FInputActionValue& Value)
{
	// 재장전 중이거나 총알이 없으면 발사 불가
	if (bIsReloading || CurrentAmmo <= 0 || bIsFanFiring || bIsDodging)
	{
		return;
	}

	bIsFanFiring = true;

	// 원 애니메이션 bp에서 구현
	FanFireUI((CurrentAmmo + 5) * FanFireRate);
	
	// 남은 총알만큼 연사
	int32 bulletsToFire = CurrentAmmo;
	
	// 즉시 첫 발 발사
	FireBullet(true);
	
	// 나머지 총알 타이머로 발사
	for (int32 i = 1; i < bulletsToFire; i++)
	{
		FTimerHandle TimerHandle_FanFire;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_FanFire,
			[this]() { FireBullet(true); },
			FanFireRate * i,
			false
		);
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

// 재장전 시작 (Enhanced Input)
void ACassidyCharacter::StartReload(const FInputActionValue& Value)
{

	if (CurrentAmmo == MaxAmmo || bIsReloading || bIsFanFiring)
	{
		return;
	}

	bIsReloading = true;
	UE_LOG(LogTemp, Log, TEXT("장전 시작"));

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

	// 재장전 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Reload, 
		this, 
		&ACassidyCharacter::FinishReload, 
		ReloadTime, 
		false
	);
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
	// 팬 파이어 중이거나 재장전 중이거나 총알이 없으면 발사 불가
	if (bIsFanFiring || bIsReloading || CurrentAmmo <= 0)
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
	// 총알 소모
	CurrentAmmo--;

	Recoil(bIsFanFireShot);

	UE_LOG(LogTemp, Log, TEXT("남은 탄환 : %d"), CurrentAmmo);

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
	}

	// 총구 이펙트 재생
	if (MuzzleFlash)
	{
		const FVector MuzzleLocation = FPGunMesh->GetSocketLocation(FName("MuzzleSocket"));
		const FRotator MuzzleRotation = FPGunMesh->GetSocketRotation(FName("MuzzleSocket"));
		
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation, MuzzleRotation);
	}

	// 탄피 이펙트 재생
	if (ShellEject)
	{
		const FVector EjectLocation = FPGunMesh->GetSocketLocation(FName("ShellEjectSocket"));
		const FRotator EjectRotation = FPGunMesh->GetSocketRotation(FName("ShellEjectSocket"));
		
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShellEject, EjectLocation, EjectRotation);
	}

	// 라인 트레이스 (히트스캔)를 통한 명중 판정
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
	QueryParams.AddIgnoredActor(this); // 자기 자신은 무시
	QueryParams.bTraceComplex = true; // 복잡한 충돌 체크
	
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

	// 디버그 라인 그리기 (개발용)
	#if WITH_EDITOR
	FColor LineColor = bIsFanFireShot ? FColor::Yellow : FColor::Red;
	DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, LineColor, false, 1.0f, 0, 1.0f);
	#endif

	// 명중 시 데미지 처리
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			
			// 명중 이펙트 (선택적)
			// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}
	}


	if (CurrentAmmo <= 0)
	{

		StartReload(FInputActionValue());
	}
	
}


void ACassidyCharacter::ThrowFlashbang(const FInputActionValue& Value)
{
	// 쿨타임 중이면 사용 불가
	if (bFlashbangOnCooldown || bIsFanFiring || bIsReloading || bIsDodging)
	{
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

	// 플레이어 시점에서 약간 앞쪽 위치 계산
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);

	// 전방 벡터 계산
	FVector ForwardVector = CameraRotation.Vector();

	// 약간 앞쪽에 섬광탄이 터지는 위치 계산 (약 3미터 앞)
	FlashbangLocation = CameraLocation + (ForwardVector * 500.0f);

	FTimerHandle TimerHandle_Flashbang;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Flashbang,
		[this]() {Flashbang(); },
		0.12f,
		false
	);

}

void ACassidyCharacter::Flashbang()
{

	// 섬광탄 이펙트 재생
	if (FlashbangExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FlashbangExplosionEffect, FlashbangLocation, FRotator::ZeroRotator);
	}

	// 섬광탄 소리 재생
	if (FlashbangSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FlashbangSound, FlashbangLocation);
	}

	// 디버그 표시 (개발용)
#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), FlashbangLocation, FlashbangRadius, 16, FColor::Yellow, false, 1.0f);
#endif
}

void ACassidyCharacter::FinishFlashbangCooldown()
{
	bFlashbangOnCooldown = false;
	UE_LOG(LogTemp, Log, TEXT("섬광탄 쿨타임 완료"));
}


void ACassidyCharacter::Dodge()
{
	if (bIsDodging || bDodgingOnCooldown || bIsFanFiring)
	{
		return;
	}
	bIsDodging = true;
	bDodgingOnCooldown = true;

	StopFanFireUI();

	if (bIsReloading)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Reload);
		FinishReload();
	}

	FTimerHandle TimerHandle_DodgeCooldown;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_DodgeCooldown,
		[this]() {bDodgingOnCooldown = false;},
		DodgeCooldown,
		false
	);

	FTimerHandle TimerHandle_Dodge;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_Dodge,
		this,
		&ACassidyCharacter::FinishDodge,
		0.25f,
		false
	);

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->MaxWalkSpeed = DodgeSpeed;
	MovementComp->MaxAcceleration = 50000.0f;
	DownCameraOnDodge();
	//MovementComp->SetMovementMode(MOVE_Flying);

	if (DodgeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("구르기"));
}

void ACassidyCharacter::FinishDodge()
{
	bIsDodging = false;
	CurrentAmmo = MaxAmmo;
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	MovementComp->MaxWalkSpeed = MaxWalkSpeed;
	MovementComp->MaxAcceleration = 2048.0f;
	MovementComp->SetMovementMode(MOVE_Walking);

	UE_LOG(LogTemp, Log, TEXT("구르기 끝"));
}
