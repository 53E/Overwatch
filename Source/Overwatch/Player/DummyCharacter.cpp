#include "DummyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DeathComponent.h"

ADummyCharacter::ADummyCharacter()
{
	// 더미 캐릭터 기본 설정
	PrimaryActorTick.bCanEverTick = false;
	
	// 높은 체력 설정
	MaxHealth = 1000.0f;
	CurrentHealth = MaxHealth;
	
	// 자동 부활 설정
	bAutoRevive = true;
	ReviveDelay = 3.0f;
	
	// AI 컨트롤러 비활성화
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;
	
	// 더미는 쉴드가 없음
	MaxShield = 0.0f;
	Shield = 0.0f;
}

void ADummyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 더미는 특정 팀에 속하지 않음
	// 모든 플레이어가 공격 가능
}

void ADummyCharacter::Die(AActor* Killer)
{
	// 부모 클래스 Die 호출
	Super::Die(Killer);
	
	
	// 자동 부활 설정
	if (bAutoRevive && GetLocalRole() == ROLE_Authority)
	{
		// 기존 타이머가 있다면 취소
		if (GetWorld()->GetTimerManager().IsTimerActive(ReviveTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(ReviveTimerHandle);
		}
		
		// 부활 타이머 설정
		GetWorld()->GetTimerManager().SetTimer(
			ReviveTimerHandle,
			this,
			&ADummyCharacter::Revive,
			ReviveDelay,
			false
		);
	}
}

void ADummyCharacter::Revive_Implementation()
{
	// 서버에서만 처리
	if (GetLocalRole() != ROLE_Authority)
		return;
		
	// 이미 살아있다면 리턴
	if (!bIsDead)
		return;
	/*
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; 
	SpawnParams.Instigator = GetInstigator(); 
	GetWorld()->SpawnActor<ADummyCharacter>(ADummyCharacter::StaticClass(),GetActorLocation(),GetActorRotation(),SpawnParams);
	*/

	this->Destroy();
	
	// 죽음 상태 해제
	bIsDead = false;
	
	// 체력 완전 회복
	CurrentHealth = MaxHealth;
	Shield = MaxShield;
	
	// DeathComponent를 사용하여 래그돌 비활성화
	if (DeathComponent)
	{
		DeathComponent->DeactivateRagdoll(this);
	}
	else
	{
		// DeathComponent가 없는 경우 기본 처리
		if (GetMesh())
		{
			// 모든 본의 물리 비활성화
			GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), false, true);
			
			// 물리 블렌드 웨이트를 0으로 설정 (완전한 애니메이션)
			GetMesh()->SetAllBodiesPhysicsBlendWeight(0.0f, false);
			
			// 메시 콜리전 복구
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
			GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
			
			// 캐릭터를 원래 위치로 텔레포트 (레그돌로 움직인 경우)
			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z += 100.0f; // 약간 위로 스폰
			SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
			
			// 캐릭터 회전 초기화
			SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
		}
		
		// 충돌 복원
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		// 움직임 재활성화
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	
	// 무기 표시 (더미는 무기가 없을 수도 있음)
	if (TPWeapon)
	{
		TPWeapon->SetVisibility(true);
	}
	
	// 블루프린트 이벤트 호출 (부활 효과 등)
	OnRevive();
}

float ADummyCharacter::Hit(float DamageAmount, AActor* DamageCauser)
{
	// 부모 클래스의 Hit 함수 호출
	float ActualDamage = Super::Hit(DamageAmount, DamageCauser);
	
	// 데미지 표시 (클라이언트에서도 보이도록)
	if (ActualDamage > 0)
	{
		// 히트 위치 계산 (캐릭터 위쪽)
		FVector HitLocation = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 50.0f);
		
		// 블루프린트 이벤트 호출
		OnShowDamage(ActualDamage, HitLocation);
	}
	
	return ActualDamage;
}
