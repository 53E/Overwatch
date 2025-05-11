#include "OverwatchGameSession.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"

AOverwatchGameSession::AOverwatchGameSession()
{
    // 기본값 설정
    ServerName = TEXT("Overwatch Server");
    bIsLANServer = false;
    bIsDedicatedServer = true;
    DefaultMaxPlayers = 12;
    
    // 부모 클래스의 MaxPlayers 설정
    MaxPlayers = DefaultMaxPlayers;
}

void AOverwatchGameSession::RegisterServer()
{
    // Online Subsystem 가져오기
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // 세션 인터페이스 가져오기
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            // 기존 세션 제거
            Sessions->DestroySession(NAME_GameSession);
            
            // 세션 설정 구성
            TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
            
            // 세션 설정 구성
            SessionSettings->bIsLANMatch = bIsLANServer;
            SessionSettings->bUsesPresence = false;
            SessionSettings->bShouldAdvertise = true;
            SessionSettings->bAllowJoinInProgress = true;
            SessionSettings->bAllowInvites = true;
            SessionSettings->bUsesPresence = false;
            SessionSettings->bAllowJoinViaPresence = false;
            SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
            SessionSettings->NumPublicConnections = MaxPlayers;
            SessionSettings->NumPrivateConnections = 0;
            
            // 서버 정보 설정
            SessionSettings->Set(TEXT("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
            
            // 세션 생성
            Sessions->CreateSession(0, NAME_GameSession, *SessionSettings);
            
            UE_LOG(LogTemp, Log, TEXT("Server registration started: %s"), *ServerName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No OnlineSubsystem found, server will not be registered online"));
    }
}

void AOverwatchGameSession::InitOptions(const FString& Options)
{
    Super::InitOptions(Options);
    
    // 명령줄 옵션에서 서버 설정 가져오기
    FString ServerNameOption = UGameplayStatics::ParseOption(Options, TEXT("ServerName"));
    if (!ServerNameOption.IsEmpty())
    {
        ServerName = ServerNameOption;
    }
    
    // LAN 서버 여부
    FString LANOption = UGameplayStatics::ParseOption(Options, TEXT("LAN"));
    if (!LANOption.IsEmpty())
    {
        bIsLANServer = LANOption.ToBool();
    }
    
    // 플레이어 수 제한 - 부모 클래스의 MaxPlayers 사용
    FString MaxPlayersOption = UGameplayStatics::ParseOption(Options, TEXT("MaxPlayers"));
    if (!MaxPlayersOption.IsEmpty())
    {
        MaxPlayers = FCString::Atoi(*MaxPlayersOption);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Game session initialized: ServerName=%s, LAN=%s, MaxPlayers=%d"), 
        *ServerName, bIsLANServer ? TEXT("true") : TEXT("false"), MaxPlayers);
}

void AOverwatchGameSession::OnStartSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    Super::OnStartSessionComplete(InSessionName, bWasSuccessful);
    
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session started successfully: %s"), *InSessionName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to start session: %s"), *InSessionName.ToString());
    }
}

void AOverwatchGameSession::OnEndSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    Super::OnEndSessionComplete(InSessionName, bWasSuccessful);
    
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session ended successfully: %s"), *InSessionName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to end session: %s"), *InSessionName.ToString());
    }
}

void AOverwatchGameSession::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    // 추가 접속 처리 로직
    if (NewPlayer && NewPlayer->PlayerState)
    {
        UE_LOG(LogTemp, Log, TEXT("Player logged in: %s"), *NewPlayer->PlayerState->GetPlayerName());
    }
}

bool AOverwatchGameSession::AllowSpectating()
{
    // 관전자 허용 여부
    // 기본적으로 허용
    return true;
}
