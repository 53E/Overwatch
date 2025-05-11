#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OverwatchPlayerState.generated.h"

/**
 * 능력 사용 기록 구조체
 * TMap 대신 TArray로 복제하기 위한 구조체
 */
USTRUCT()
struct FAbilityUseInfo
{
	GENERATED_BODY()

	// 능력 이름
	UPROPERTY()
	FName AbilityName;

	// 사용 횟수
	UPROPERTY()
	int32 UseCount;

	FAbilityUseInfo()
		: AbilityName(NAME_None)
		, UseCount(0)
	{}

	FAbilityUseInfo(FName InName, int32 InCount)
		: AbilityName(InName)
		, UseCount(InCount)
	{}
};

/**
 * 오버워치 플레이어 상태 클래스
 * 게임 전체에 복제되는 플레이어 데이터를 관리
 */
UCLASS()
class OVERWATCH_API AOverwatchPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AOverwatchPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 킬, 데스 등 게임 점수 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddKill();
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddDeath();
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddAssist();
	
	// 특수 능력 사용 통계
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddAbilityUse(FName AbilityName);
	
	// 게터 함수들
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetKills() const { return Kills; }
	
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetDeaths() const { return Deaths; }
	
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetAssists() const { return Assists; }
	
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetAbilityUseCount(FName AbilityName) const;
	
	// 팀 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeamID(int32 NewTeamID);
	
	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetTeamID() const { return TeamID; }
	
protected:
	// 플레이어 전투 통계
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Kills;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Assists;
	
	// 특수 능력 사용 통계 (TMap 대신 TArray 사용)
	UPROPERTY(Replicated)
	TArray<FAbilityUseInfo> AbilityUses;
	
	// 플레이어 팀 ID (0: 레드, 1: 블루)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 TeamID;
};
