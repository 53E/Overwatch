#include "OverwatchPlayerController.h"
#include "Overwatch/Overwatch.h"
#include "Overwatch/GameMode/OverwatchGameMode.h"
#include "Overwatch/GameMode/OverwatchGameState.h"
#include "OverwatchPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AOverwatchPlayerController::AOverwatchPlayerController()
{
    // 매 프레임 Tick() 호출 설정
    PrimaryActorTick.bCanEverTick = true;
    
    // 네트워크 디버그 기본값 설정
    bShowNetworkDebug = false;
}

void AOverwatchPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // 로컬 플레이어인 경우 주기적으로 게임 상태 업데이트 요청
    if (IsLocalController())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_RequestGameState,
            this,
            &AOverwatchPlayerController::RequestGameStateUpdate,
            1.0f,
            true
        );
    }
}

void AOverwatchPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // 네트워크 디버그 전환 커맨드 바인딩
    InputComponent->BindKey(FKey("F10"), IE_Pressed, this, &AOverwatchPlayerController::ToggleNetworkDebug);
}

void AOverwatchPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 네트워크 디버그 정보 표시
    if (bShowNetworkDebug)
    {
        GNetworkDebugger.DisplayNetworkDebug(GetWorld());
    }
}

void AOverwatchPlayerController::ToggleNetworkDebug()
{
    bShowNetworkDebug = !bShowNetworkDebug;
    
    // 디버그 메시지 표시
    if (bShowNetworkDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("네트워크 디버그 활성화"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("네트워크 디버그 비활성화"));
    }
}

bool AOverwatchPlayerController::ServerRequestRespawn_Validate()
{
    return true;
}

void AOverwatchPlayerController::ServerRequestRespawn_Implementation()
{
    // 게임 모드에 리스폰 요청
    AOverwatchGameMode* GameMode = Cast<AOverwatchGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        GameMode->RespawnPlayer(this);
    }
}

bool AOverwatchPlayerController::ServerRequestTeamChange_Validate(int32 NewTeamID)
{
    // 유효한 팀 ID 검증 (0: 레드, 1: 블루)
    return NewTeamID >= 0 && NewTeamID <= 1;
}

void AOverwatchPlayerController::ServerRequestTeamChange_Implementation(int32 NewTeamID)
{
    // 플레이어 상태에 새 팀 ID 설정
    AOverwatchPlayerState* OverwatchPlayerState = Cast<AOverwatchPlayerState>(PlayerState);
    if (OverwatchPlayerState)
    {
        OverwatchPlayerState->SetTeamID(NewTeamID);
    }
    
    // 게임 상태에 플레이어 팀 업데이트
    AOverwatchGameState* GameState = Cast<AOverwatchGameState>(UGameplayStatics::GetGameState(this));
    if (GameState && PlayerState)
    {
        GameState->AddPlayerToTeam(PlayerState, NewTeamID);
    }
}

void AOverwatchPlayerController::ClientUpdateGameStateUI_Implementation(const FString& GameState, float TimeRemaining)
{
    // 클라이언트 UI 업데이트 (블루프린트에서 구현)
    // BP_OverwatchHUD에서 구현 예정
}

void AOverwatchPlayerController::RequestGameStateUpdate()
{
    // 게임 상태 정보 가져오기
    AOverwatchGameState* GameState = Cast<AOverwatchGameState>(UGameplayStatics::GetGameState(this));
    if (GameState)
    {
        // UI 업데이트
        ClientUpdateGameStateUI(GameState->GetMatchState().ToString(), GameState->GetMatchTimeRemaining());
    }
}
