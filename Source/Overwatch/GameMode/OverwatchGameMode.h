#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OverwatchGameMode.generated.h"

/**
 * 오버워치 게임 모드 - 인게임 환경을 관리
 */
UCLASS()
class OVERWATCH_API AOverwatchGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AOverwatchGameMode();

	virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    // 플레이어 리스폰 처리
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RespawnPlayer(AController* Controller);
    
    // 팀 밸런싱 처리
    UFUNCTION(BlueprintCallable, Category = "Game")
    void BalanceTeams();
    
    // 게임 시작 및 종료
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartMatch();
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void EndMatch();
    
protected:
    // 게임 설정
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float MatchDuration;
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 MaxPlayersPerTeam;
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 MinPlayers;
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float RespawnDelay;
    
    // 맵 정보
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TArray<FString> MapRotation;
    
    // 스폰 포인트 관리
    TArray<AActor*> RedTeamSpawnPoints;
    TArray<AActor*> BlueTeamSpawnPoints;
    
    // 스폰 포인트 수집
    void CollectSpawnPoints();
    
    // 타이머 핸들
    TMap<AController*, FTimerHandle> RespawnTimers;
    
private:
    // 플레이어를 팀에 배정
    void AssignPlayerToTeam(AController* Controller);
    
    // 리스폰 완료 콜백
    void FinishRespawn(AController* Controller);
};
