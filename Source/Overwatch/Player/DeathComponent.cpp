#include "DeathComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UDeathComponent::UDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// 기본 설정
	RootBoneName = TEXT("pelvis");
	ImpulseStrength = 1000.0f;
	bApplyImpulse = true;
}

void UDeathComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDeathComponent::ActivateRagdoll_Implementation(ACharacter* Character, FVector ImpulseDirection)
{
	if (!Character)
		return;
		
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Regdoll!"));
	// 1. 캐릭터 움직임 중지
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->DisableMovement();
		Character->GetCharacterMovement()->SetComponentTickEnabled(false);
	}
	
	// 2. 캡슐 콜리전 수정
	if (Character->GetCapsuleComponent())
	{
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Character->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	
	// 3. 애니메이션 정지
	if (Mesh->GetAnimInstance())
	{
		Mesh->GetAnimInstance()->StopAllMontages(0.0f);
	}
	
	// 4. 메시 콜리전 설정
	Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	// 5. 물리 시뮬레이션 활성화
	// 모든 본에 대해 시뮬레이션 (루트 본 이름이 다를 수 있음)
	Mesh->SetSimulatePhysics(true);
	
	// 대체 방법: 특정 본 이하만 시뮬레이션
	if (!RootBoneName.IsNone())
	{
		Mesh->SetAllBodiesBelowSimulatePhysics(RootBoneName, true, true);
	}
	
	// 6. 물리 블렌드 웨이트 설정
	Mesh->SetAllBodiesPhysicsBlendWeight(1.0f, false);
	
	// 7. 초기 충격 적용
	if (bApplyImpulse && !ImpulseDirection.IsZero())
	{
		// 충격 방향이 없으면 기본값 사용
		if (ImpulseDirection == FVector::ZeroVector)
		{
			ImpulseDirection = -Character->GetActorForwardVector() * ImpulseStrength;
			ImpulseDirection.Z = ImpulseStrength * 0.5f;
		}
		
		Mesh->AddImpulse(ImpulseDirection, NAME_None, true);
	}
	
	// 8. 본 트랜스폼 업데이트 모드 설정
	Mesh->SetAnimationMode(EAnimationMode::AnimationCustomMode);
	Mesh->bBlendPhysics = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Ragdoll Activated for %s"), *Character->GetName());
}

void UDeathComponent::DeactivateRagdoll_Implementation(ACharacter* Character)
{
	if (!Character)
		return;
		
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
		return;
		
	// 1. 물리 시뮬레이션 비활성화
	Mesh->SetSimulatePhysics(false);
	Mesh->SetAllBodiesBelowSimulatePhysics(RootBoneName, false, true);
	
	// 2. 물리 블렌드 비활성화
	Mesh->SetAllBodiesPhysicsBlendWeight(0.0f, false);
	Mesh->bBlendPhysics = false;
	
	// 3. 애니메이션 모드 복원
	Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	
	// 4. 메시 콜리전 복원
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	
	// 5. 캡슐 콜리전 복원
	if (Character->GetCapsuleComponent())
	{
		Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	
	// 6. 캐릭터 이동 컴포넌트 활성화
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Character->GetCharacterMovement()->SetComponentTickEnabled(true);
	}
	
	// 7. 위치 및 회전 정리
	FVector NewLocation = Character->GetActorLocation();
	NewLocation.Z += 100.0f; // 바닥에 묻히지 않도록
	Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::ResetPhysics);
	
	// 회전 초기화
	FRotator NewRotation = Character->GetActorRotation();
	NewRotation.Pitch = 0.0f;
	NewRotation.Roll = 0.0f;
	Character->SetActorRotation(NewRotation);
	
	UE_LOG(LogTemp, Warning, TEXT("Ragdoll Deactivated for %s"), *Character->GetName());
}

void UDeathComponent::HandleDeath(ACharacter* Character, AActor* Killer)
{
	if (!Character)
		return;
		
	// 죽음 시 충격 방향 계산
	FVector ImpulseDirection = FVector::ZeroVector;
	
	if (Killer)
	{
		// 킬러 방향에서 충격 적용
		ImpulseDirection = (Character->GetActorLocation() - Killer->GetActorLocation()).GetSafeNormal();
		ImpulseDirection *= ImpulseStrength;
		ImpulseDirection.Z = ImpulseStrength * 0.5f;
	}
	
	// 래그돌 활성화
	ActivateRagdoll(Character, ImpulseDirection);
}
