#include "OverwatchGameMode.h"
#include "Overwatch/Player/Cassidy/CassidyCharacter.h"
#include "Overwatch/Player/OverwatchPlayerController.h"
#include "Overwatch/Player/OverwatchPlayerState.h"
#include "OverwatchGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameSession.h"
#include "TimerManager.h"

AOverwatchGameMode::AOverwatchGameMode()
{
    // 기본 폰 클래스를 CassidyCharacter로 설정
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Characters/Cassidy/BP_CassidyCharacter"));
    DefaultPawnClass = PlayerPawnClassFinder.Class;
    
    // BP_CassidyCharacter가 아직 존재하지 않는 경우 C++ 클래스를 사용
    if (!DefaultPawnClass)
    {
        DefaultPawnClass = ACassidyCharacter::StaticClass();
    }
    
    // 플레이어 컨트롤러 설정
    PlayerControllerClass = AOverwatchPlayerController::StaticClass();
    
    // 플레이어 상태 설정
    PlayerStateClass = AOverwatchPlayerState::StaticClass();
    
    // 게임 상태 설정
    GameStateClass = AOverwatchGameState::StaticClass();
    
    // 게임 설정 초기화
    MatchDuration = 600.0f; // 10분
    MaxPlayersPerTeam = 6;  // 팀당 최대 6명
    RespawnDelay = 500000.0f;    // 5초 후 리스폰
    
    // 세션 플레이어 수 설정
    MinPlayers = 2;         // 최소 2명
    
    // 데디케이티드 서버 설정
    bUseSeamlessTravel = true;
    
    // 네트워크 설정
    // 서버 실행 명령줄 예시:
    // <게임이름>Server.exe <맵이름> -server -log -PORT=7777 -MAXPLAYERS=12
    UE_LOG(LogTemp, Log, TEXT("Overwatch Game Mode - 데디케이티드 서버 설정이 로드되었습니다."));
    UE_LOG(LogTemp, Log, TEXT("* 브라우저에서 접속을 위한 서버 URL 포맷: <아이피>"));
    UE_LOG(LogTemp, Log, TEXT("* 클라이언트 명령줄 예시: <게임이름>.exe <서버아이피> -game"));
    
}

void AOverwatchGameMode::BeginPlay()
{
    Super::BeginPlay();

    
    // 스폰 포인트 수집
    CollectSpawnPoints();
    
    // 게임 시작 시 추가 로직이 필요한 경우 여기에 작성
    AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
    if (OverwatchGameState)
    {
        // 게임 상태 초기화
        OverwatchGameState->SetMatchDuration(MatchDuration);
    }
}

void AOverwatchGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // 명령줄 인수 처리
    UE_LOG(LogTemp, Log, TEXT("Initializing game on map: %s"), *MapName);
    
    // 서버 세션 설정
    if (GameSession)
    {
        // 세션 설정
        GameSession->MaxPlayers = MaxPlayersPerTeam * 2;
    }
}

void AOverwatchGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    // 플레이어가 게임에 참여하면 팀 배정
    AssignPlayerToTeam(NewPlayer);
    
    // 충분한 플레이어가 모였는지 확인
    int32 NumPlayers = GetNumPlayers();
    if (NumPlayers >= MinPlayers)
    {
        AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
        if (OverwatchGameState && OverwatchGameState->GetMatchState() == FName("WaitingForPlayers"))
        {
            // 게임 시작
            StartMatch();
        }
    }
}

void AOverwatchGameMode::Logout(AController* Exiting)
{
    // 플레이어 접속 종료 처리
    Super::Logout(Exiting);
    
    // 게임이 진행 중인지 확인
    AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
    if (OverwatchGameState && OverwatchGameState->GetMatchState() == FName("InProgress"))
    {
        // 남은 플레이어 수 확인
        int32 NumPlayers = GetNumPlayers();
        if (NumPlayers < MinPlayers)
        {
            // 플레이어가 부족하면 게임 종료
            EndMatch();
        }
    }
}

void AOverwatchGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    
    // 새 플레이어 처리 - 기본 설정 및 리스폰
    RespawnPlayer(NewPlayer);
}

AActor* AOverwatchGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // 플레이어 상태 가져오기
    AOverwatchPlayerState* OverwatchPlayerState = Player ? Cast<AOverwatchPlayerState>(Player->PlayerState) : nullptr;
    
    // 팀에 따라 스폰 포인트 선택
    if (OverwatchPlayerState)
    {
        int32 TeamID = OverwatchPlayerState->GetTeamID();
        
        // 팀 ID에 따라 스폰 포인트 배열 선택
        TArray<AActor*>& TeamSpawnPoints = (TeamID == 0) ? RedTeamSpawnPoints : BlueTeamSpawnPoints;
        
        // 스폰 포인트가 있는 경우 랜덤 선택
        if (TeamSpawnPoints.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, TeamSpawnPoints.Num() - 1);
            return TeamSpawnPoints[RandomIndex];
        }
    }
    
    // 기본 로직 사용 (팀 지정이 안된 경우)
    return Super::ChoosePlayerStart_Implementation(Player);
}

void AOverwatchGameMode::RespawnPlayer(AController* Controller)
{
    if (!Controller)
        return;
        
    // 이미 리스폰 중인 경우 타이머 취소
    FTimerHandle* ExistingTimer = RespawnTimers.Find(Controller);
    if (ExistingTimer)
    {
        GetWorldTimerManager().ClearTimer(*ExistingTimer);
    }
    
    if (RespawnDelay <= 0.0f)
    {
        // 지연 없이 즉시 리스폰
        FinishRespawn(Controller);
    }
    else
    {
        // 지연 후 리스폰
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &AOverwatchGameMode::FinishRespawn, Controller),
            RespawnDelay,
            false
        );
        
        // 타이머 저장
        RespawnTimers.Add(Controller, TimerHandle);
        
        // 리스폰 카운트다운 표시 (옵션)
        AOverwatchPlayerController* PlayerController = Cast<AOverwatchPlayerController>(Controller);
        if (PlayerController)
        {
            // 클라이언트에 리스폰 카운트다운 표시 (향후 구현)
        }
    }
}

void AOverwatchGameMode::FinishRespawn(AController* Controller)
{
    if (!Controller)
        return;
        
    // 타이머 제거
    RespawnTimers.Remove(Controller);
    
    // 플레이어 상태 확인
    APlayerState* PlayerState = Controller->PlayerState;
    if (!PlayerState)
        return;
        
    // 팀 확인
    AOverwatchPlayerState* OverwatchPlayerState = Cast<AOverwatchPlayerState>(PlayerState);
    if (!OverwatchPlayerState || OverwatchPlayerState->GetTeamID() < 0)
    {
        // 팀이 지정되지 않은 경우 팀 배정
        AssignPlayerToTeam(Controller);
    }
    
    // 기존 폰 제거
    if (Controller->GetPawn())
    {
        Controller->GetPawn()->Destroy();
    }
    
    // 새 폰 생성
    RestartPlayer(Controller);
}

void AOverwatchGameMode::AssignPlayerToTeam(AController* Controller)
{
    if (!Controller || !Controller->PlayerState)
        return;
        
    // 게임 상태 확인
    AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
    if (!OverwatchGameState)
        return;
        
    // 플레이어 상태 가져오기
    AOverwatchPlayerState* OverwatchPlayerState = Cast<AOverwatchPlayerState>(Controller->PlayerState);
    if (!OverwatchPlayerState)
        return;
        
    // 팀 배정 - 플레이어 수가 적은 팀에 배정
    // 팀 0: 레드팀, 팀 1: 블루팀
    int32 RedTeamCount = 0;
    int32 BlueTeamCount = 0;
    
    // 현재 팀 인원 계산
    for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
    {
        AOverwatchPlayerState* PS = Cast<AOverwatchPlayerState>(*It);
        if (PS)
        {
            if (PS->GetTeamID() == 0)
            {
                RedTeamCount++;
            }
            else if (PS->GetTeamID() == 1)
            {
                BlueTeamCount++;
            }
        }
    }
    
    // 팀 배정 - 플레이어 수가 적은 팀 또는 동일한 경우 랜덤
    int32 AssignedTeam;
    if (RedTeamCount < BlueTeamCount)
    {
        AssignedTeam = 0; // 레드팀
    }
    else if (BlueTeamCount < RedTeamCount)
    {
        AssignedTeam = 1; // 블루팀
    }
    else
    {
        // 동일한 경우 랜덤
        AssignedTeam = FMath::RandBool() ? 0 : 1;
    }
    
    // 팀 설정
    OverwatchPlayerState->SetTeamID(AssignedTeam);
    
    // 게임 상태 업데이트
    OverwatchGameState->AddPlayerToTeam(Controller->PlayerState, AssignedTeam);
}

void AOverwatchGameMode::BalanceTeams()
{
    // 팀 재배정이 필요한 경우에만 사용
    TArray<AController*> Controllers;
    
    // 컨트롤러 수집
    for (TActorIterator<AController> It(GetWorld()); It; ++It)
    {
        AController* Controller = *It;
        if (Controller && Controller->IsPlayerController() && Controller->PlayerState)
        {
            Controllers.Add(Controller);
        }
    }
    
    // 각 플레이어 재배정
    for (AController* Controller : Controllers)
    {
        AssignPlayerToTeam(Controller);
    }
    
    // 모든 플레이어 리스폰
    for (AController* Controller : Controllers)
    {
        RespawnPlayer(Controller);
    }
}

void AOverwatchGameMode::StartMatch()
{
    // 게임 상태 가져오기
    AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
    if (!OverwatchGameState)
        return;
        
    // 게임 시작 상태로 변경
    OverwatchGameState->StartMatchTimer();
    
    UE_LOG(LogTemp, Log, TEXT("Match started!"));
}

void AOverwatchGameMode::EndMatch()
{
    // 게임 상태 가져오기
    AOverwatchGameState* OverwatchGameState = GetGameState<AOverwatchGameState>();
    if (!OverwatchGameState)
        return;
        
    // 게임 종료 상태로 변경
    OverwatchGameState->StopMatchTimer();
    OverwatchGameState->SetMatchState(FName("MatchEnded"));
    
    UE_LOG(LogTemp, Log, TEXT("Match ended!"));
}

void AOverwatchGameMode::CollectSpawnPoints()
{
    // 기존 배열 초기화
    RedTeamSpawnPoints.Empty();
    BlueTeamSpawnPoints.Empty();
    
    // 레벨에서 플레이어 스타트 찾기
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* PlayerStart = *It;
        
        // 태그로 팀 구분
        if (PlayerStart->PlayerStartTag == FName("RedTeam"))
        {
            RedTeamSpawnPoints.Add(PlayerStart);
        }
        else if (PlayerStart->PlayerStartTag == FName("BlueTeam"))
        {
            BlueTeamSpawnPoints.Add(PlayerStart);
        }
        else
        {
            // 기본값 - 양쪽 팀에 추가
            RedTeamSpawnPoints.Add(PlayerStart);
            BlueTeamSpawnPoints.Add(PlayerStart);
        }
    }
    
    // 스폰 포인트 로그
    UE_LOG(LogTemp, Log, TEXT("Collected %d Red team and %d Blue team spawn points"), 
        RedTeamSpawnPoints.Num(), BlueTeamSpawnPoints.Num());
}
