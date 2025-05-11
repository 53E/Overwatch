#include "OverwatchCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// 네트워크 복제 설정
void AOverwatchCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 체력 관련 변수 복제 설정
	DOREPLIFETIME(AOverwatchCharacter, CurrentHealth);
	DOREPLIFETIME(AOverwatchCharacter, Shield);
	DOREPLIFETIME(AOverwatchCharacter, Armor);
	DOREPLIFETIME(AOverwatchCharacter, bIsDead);
}

// 생성자
AOverwatchCharacter::AOverwatchCharacter()
{
	// 매 프레임 Tick() 호출 설정
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// 카메라 컴포넌트 생성
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f)); // 눈 높이 위치
	FirstPersonCamera->bUsePawnControlRotation = true;

	// 1인칭 무기 메시 컴포넌트 생성
	FPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPWeaponMesh"));
	FPWeaponMesh->SetOnlyOwnerSee(true); // 소유자만 볼 수 있음
	FPWeaponMesh->SetupAttachment(FirstPersonCamera);
	FPWeaponMesh->CastShadow = false;
	
	// 3인칭 무기 메시 생성 (다른 플레이어에게 보이는 것)
	TPWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPWeaponMesh"));
	TPWeaponMesh->SetOwnerNoSee(true); // 소유자는 볼 수 없음
	TPWeaponMesh->SetupAttachment(GetMesh()); // 캐릭터 메시에 부착

	// 체력 초기화
	MaxHealth = 200.0f;
	CurrentHealth = MaxHealth;
	Armor = 0.0f;
	MaxShield = 0.0f;
	Shield = MaxShield;
	ShieldRegenRate = 30.0f; // 초당 30 쉴드 재생
	ShieldRegenDelay = 3.0f; // 데미지 후 3초 후 재생 시작
	bIsDead = false;
	
	// 이동 속도 초기화
	BaseWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AOverwatchCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기화
	CurrentHealth = MaxHealth;
	Shield = MaxShield;
	bIsDead = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
	// 1인칭/3인칭 메시 가시성 설정
	if (IsLocallyControlled())
	{
		// 로컬 플레이어는 1인칭 무기만 보임
		FPWeaponMesh->SetVisibility(true);
		TPWeaponMesh->SetVisibility(false);
	}
	else
	{
		// 다른 플레이어는 3인칭 무기만 보임
		FPWeaponMesh->SetVisibility(false);
		TPWeaponMesh->SetVisibility(true);
	}
	
	// Enhanced Input 매핑 컨텍스트 추가
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AOverwatchCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOverwatchCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 로컬 플레이어만 입력 처리
    if (!IsLocallyControlled())
        return;

	// Enhanced Input으로 캐스팅
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (EnhancedInputComponent)
	{
		// 기본 입력 바인딩
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOverwatchCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOverwatchCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

// TakeDamage 대신 Hit 함수 구현 (서버 권한)
float AOverwatchCharacter::Hit(float DamageAmount, AActor* DamageCauser)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return 0.0f;
        
	if (bIsDead)
		return 0.0f;

	float ActualDamage = DamageAmount;
	
	// 데미지 분배 로직: 쉴드 > 아머 > 체력 순으로 데미지 적용
	// 쉴드 데미지 계산
	if (Shield > 0)
	{
		if (Shield >= ActualDamage)
		{
			Shield -= ActualDamage;
			ActualDamage = 0;
		}
		else
		{
			ActualDamage -= Shield;
			Shield = 0;
		}
		
		// 쉴드 재생 타이머 재설정
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRegen);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShieldRegen, this, &AOverwatchCharacter::StartShieldRegen, ShieldRegenDelay, false);
	}
	
	// 아머 데미지 계산 (아머는 받는 데미지를 일부 감소시킴)
	if (ActualDamage > 0 && Armor > 0)
	{
		// 아머는 데미지를 최대 50%까지 감소 (기본적으로 데미지의 30% 감소)
		float DamageReduction = FMath::Min(ActualDamage * 0.3f, ActualDamage * 0.5f);
		ActualDamage -= DamageReduction;
	}
	
	// 체력 데미지 계산
	if (ActualDamage > 0)
	{
		CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
		if (CurrentHealth <= 0)
		{
			Die();
		}
	}
	
	// 데미지 이벤트 발생 (블루프린트에서 처리 가능)
	OnDamaged(DamageAmount, DamageCauser);
	
	return ActualDamage;
}


void AOverwatchCharacter::Die()
{
	if (bIsDead)
		return;
		
	bIsDead = true;
	
	// 모든 움직임 중지
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 무기 숨기기
	if (FPWeaponMesh)
	{
		FPWeaponMesh->SetVisibility(false);
	}
	
	if (TPWeaponMesh)
	{
		TPWeaponMesh->SetVisibility(false);
	}
	
	// 이벤트 호출 (블루프린트에서 구현)
	OnDeath();
	
	// 일정 시간 후 리스폰 처리는 게임모드에서 관리
}

void AOverwatchCharacter::Heal(float HealAmount)
{
	if (bIsDead || HealAmount <= 0)
		return;
		
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
}

void AOverwatchCharacter::StartShieldRegen()
{
	// 쉴드 재생이 필요하고 가능한 경우
	if (Shield < MaxShield && !bIsDead && MaxShield > 0)
	{
		// 0.1초마다 쉴드 재생
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShieldRegen, this, &AOverwatchCharacter::RegenShield, 0.1f, true);
	}
}

void AOverwatchCharacter::RegenShield()
{
	// 최대 쉴드에 도달했거나 사망한 경우 타이머 중지
	if (Shield >= MaxShield || bIsDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRegen);
		Shield = FMath::Min(Shield, MaxShield);
		return;
	}
	
	// 0.1초당 쉴드 재생량 계산
	float ShieldRegenAmount = ShieldRegenRate * 0.1f;
	Shield = FMath::Min(MaxShield, Shield + ShieldRegenAmount);
}

void AOverwatchCharacter::Move(const FInputActionValue& Value)
{
	// 사망 시 움직임 불가
	if (bIsDead)
		return;
		
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

void AOverwatchCharacter::Look(const FInputActionValue& Value)
{
	// 사망 시 시점 변경 불가
	if (bIsDead)
		return;
		
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
