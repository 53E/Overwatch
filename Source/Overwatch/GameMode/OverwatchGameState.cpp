#include "OverwatchGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

AOverwatchGameState::AOverwatchGameState()
{
    // 기본값 설정
    DefaultMatchDuration = 600.0f; // 10분
    MatchTimeRemaining = DefaultMatchDuration;
    CurrentMatchState = FName("WaitingForPlayers");
    
    // 팀 배열 초기화 (레드팀, 블루팀)
    Teams.Add(FTeamInfo(0)); // 레드팀 (ID: 0)
    Teams.Add(FTeamInfo(1)); // 블루팀 (ID: 1)
}

void AOverwatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 복제할 변수들 등록
    DOREPLIFETIME(AOverwatchGameState, MatchTimeRemaining);
    DOREPLIFETIME(AOverwatchGameState, CurrentMatchState);
    DOREPLIFETIME(AOverwatchGameState, Teams);
    DOREPLIFETIME(AOverwatchGameState, PlayerTeams);
}

void AOverwatchGameState::StartMatchTimer()
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    // 타이머 설정 (1초마다 UpdateMatchTimer 호출)
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_MatchTimer,
        this,
        &AOverwatchGameState::UpdateMatchTimer,
        1.0f,
        true
    );
    
    // 매치 상태 변경
    SetMatchState(FName("InProgress"));
}

void AOverwatchGameState::StopMatchTimer()
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    // 타이머 중지
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_MatchTimer);
}

void AOverwatchGameState::SetMatchDuration(float NewDuration)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    MatchTimeRemaining = NewDuration;
}

void AOverwatchGameState::UpdateMatchTimer()
{
    // 타이머 업데이트
    MatchTimeRemaining = FMath::Max(0.0f, MatchTimeRemaining - 1.0f);
    
    // 시간이 끝났는지 확인
    if (MatchTimeRemaining <= 0.0f)
    {
        // 타이머 중지
        StopMatchTimer();
        
        // 매치 종료 상태로 변경
        SetMatchState(FName("MatchEnded"));
    }
}

void AOverwatchGameState::AddPlayerToTeam(APlayerState* PlayerState, int32 TeamID)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority || !PlayerState)
        return;
        
    // 유효한 팀 ID 확인
    if (TeamID < 0 || TeamID >= Teams.Num())
        return;
        
    // 이전 팀에서 제거
    RemovePlayerFromTeam(PlayerState);
    
    // 새 팀에 추가
    Teams[TeamID].TeamMembers.Add(PlayerState);
    
    // 플레이어 팀 정보 추가/업데이트
    bool bFound = false;
    for (int32 i = 0; i < PlayerTeams.Num(); i++)
    {
        if (PlayerTeams[i].PlayerState == PlayerState)
        {
            PlayerTeams[i].TeamID = TeamID;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        PlayerTeams.Add(FPlayerTeamInfo(PlayerState, TeamID));
    }
}

void AOverwatchGameState::RemovePlayerFromTeam(APlayerState* PlayerState)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority || !PlayerState)
        return;
        
    // 플레이어의 기존 팀 찾기
    int32 TeamID = GetPlayerTeam(PlayerState);
    if (TeamID >= 0 && TeamID < Teams.Num())
    {
        // 기존 팀에서 제거
        Teams[TeamID].TeamMembers.Remove(PlayerState);
    }
    
    // 플레이어 팀 정보 제거
    for (int32 i = 0; i < PlayerTeams.Num(); i++)
    {
        if (PlayerTeams[i].PlayerState == PlayerState)
        {
            PlayerTeams.RemoveAt(i);
            break;
        }
    }
}

TArray<APlayerState*> AOverwatchGameState::GetTeamPlayers(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < Teams.Num())
    {
        return Teams[TeamID].TeamMembers;
    }
    
    return TArray<APlayerState*>();
}

int32 AOverwatchGameState::GetPlayerTeam(APlayerState* PlayerState) const
{
    if (!PlayerState)
        return -1;
        
    for (const FPlayerTeamInfo& Info : PlayerTeams)
    {
        if (Info.PlayerState == PlayerState)
        {
            return Info.TeamID;
        }
    }
    
    return -1;
}

void AOverwatchGameState::SetMatchState(const FName& NewState)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    CurrentMatchState = NewState;
}
