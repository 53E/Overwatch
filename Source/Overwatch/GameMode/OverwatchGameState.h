#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OverwatchGameState.generated.h"

/**
 * 팀 정보 구조체
 */
USTRUCT()
struct FTeamInfo
{
    GENERATED_BODY()
    
    // 팀 ID
    UPROPERTY()
    int32 TeamID;
    
    // 팀 멤버 배열
    UPROPERTY()
    TArray<class APlayerState*> TeamMembers;
    
    FTeamInfo()
        : TeamID(-1)
    {}
    
    FTeamInfo(int32 InTeamID)
        : TeamID(InTeamID)
    {}
};

/**
 * 플레이어 팀 정보 구조체
 */
USTRUCT()
struct FPlayerTeamInfo
{
    GENERATED_BODY()
    
    // 플레이어 상태
    UPROPERTY()
    class APlayerState* PlayerState;
    
    // 팀 ID
    UPROPERTY()
    int32 TeamID;
    
    FPlayerTeamInfo()
        : PlayerState(nullptr)
        , TeamID(-1)
    {}
    
    FPlayerTeamInfo(class APlayerState* InPlayerState, int32 InTeamID)
        : PlayerState(InPlayerState)
        , TeamID(InTeamID)
    {}
};

/**
 * 오버워치 게임 상태 클래스
 * 모든 클라이언트에 복제되는 게임 상태를 관리
 */
UCLASS()
class OVERWATCH_API AOverwatchGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AOverwatchGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 게임 시간 관련 함수
	void StartMatchTimer();
	void StopMatchTimer();
	void SetMatchDuration(float NewDuration);
	
	// 팀 관련 함수
	void AddPlayerToTeam(class APlayerState* PlayerState, int32 TeamID);
	void RemovePlayerFromTeam(class APlayerState* PlayerState);
	
	// 게임 진행 상태 함수
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetMatchState(const FName& NewState);
	
	UFUNCTION(BlueprintPure, Category = "Game")
	FName GetMatchState() const { return CurrentMatchState; }

    // 팀 정보 조회 함수
    UFUNCTION(BlueprintPure, Category = "Teams")
    TArray<class APlayerState*> GetTeamPlayers(int32 TeamID) const;
    
    UFUNCTION(BlueprintPure, Category = "Teams")
    int32 GetPlayerTeam(class APlayerState* PlayerState) const;
    
    // 매치 타임 조회 함수
    UFUNCTION(BlueprintPure, Category = "Game")
    float GetMatchTimeRemaining() const { return MatchTimeRemaining; }
	
protected:
	// 게임 시간 관련 변수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	float MatchTimeRemaining;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float DefaultMatchDuration;
	
	// 게임 타이머 핸들
	FTimerHandle TimerHandle_MatchTimer;
	
	// 게임 진행 상태
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	FName CurrentMatchState;
	
	// 타이머 업데이트 함수
	UFUNCTION()
	void UpdateMatchTimer();
	
	// 팀 정보
	UPROPERTY(Replicated)
    TArray<FTeamInfo> Teams;
    
    // 플레이어별 팀 정보
    UPROPERTY(Replicated)
    TArray<FPlayerTeamInfo> PlayerTeams;
};
