#include "OverwatchPlayerState.h"
#include "Net/UnrealNetwork.h"

AOverwatchPlayerState::AOverwatchPlayerState()
{
    // 초기 값 설정
    Kills = 0;
    Deaths = 0;
    Assists = 0;
    TeamID = -1; // 팀 미지정 상태
}

void AOverwatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 복제할 변수들 등록
    DOREPLIFETIME(AOverwatchPlayerState, Kills);
    DOREPLIFETIME(AOverwatchPlayerState, Deaths);
    DOREPLIFETIME(AOverwatchPlayerState, Assists);
    DOREPLIFETIME(AOverwatchPlayerState, AbilityUses);
    DOREPLIFETIME(AOverwatchPlayerState, TeamID);
}

void AOverwatchPlayerState::AddKill()
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    Kills++;
}

void AOverwatchPlayerState::AddDeath()
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    Deaths++;
}

void AOverwatchPlayerState::AddAssist()
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    Assists++;
}

void AOverwatchPlayerState::AddAbilityUse(FName AbilityName)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    // 현재 사용 횟수 찾기
    for (int32 i = 0; i < AbilityUses.Num(); i++)
    {
        if (AbilityUses[i].AbilityName == AbilityName)
        {
            // 기존 횟수 증가
            AbilityUses[i].UseCount++;
            return;
        }
    }
    
    // 새로운 능력 추가
    AbilityUses.Add(FAbilityUseInfo(AbilityName, 1));
}

int32 AOverwatchPlayerState::GetAbilityUseCount(FName AbilityName) const
{
    for (const FAbilityUseInfo& Info : AbilityUses)
    {
        if (Info.AbilityName == AbilityName)
        {
            return Info.UseCount;
        }
    }
    
    return 0;
}

void AOverwatchPlayerState::SetTeamID(int32 NewTeamID)
{
    // 서버에서만 처리
    if (GetLocalRole() != ROLE_Authority)
        return;
        
    TeamID = NewTeamID;
}
