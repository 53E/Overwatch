#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OverwatchPlayerController.generated.h"

/**
 * 오버워치 플레이어 컨트롤러 클래스
 * 클라이언트와 서버 간의 통신을 처리
 */
UCLASS()
class OVERWATCH_API AOverwatchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOverwatchPlayerController();
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	// 네트워크 디버그 표시 전환
	UFUNCTION(exec)
	void ToggleNetworkDebug();
	
	// 캐릭터 스폰 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestRespawn();
	bool ServerRequestRespawn_Validate();
	void ServerRequestRespawn_Implementation();
	
	// 팀 변경 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestTeamChange(int32 NewTeamID);
	bool ServerRequestTeamChange_Validate(int32 NewTeamID);
	void ServerRequestTeamChange_Implementation(int32 NewTeamID);
	
	// 게임 상태 UI 업데이트
	UFUNCTION(Client, Reliable)
	void ClientUpdateGameStateUI(const FString& GameState, float TimeRemaining);
	void ClientUpdateGameStateUI_Implementation(const FString& GameState, float TimeRemaining);
	
protected:
	// Tick에서 네트워크 디버그 정보 업데이트
	virtual void Tick(float DeltaTime) override;
	
	// 네트워크 디버그 사용 여부
	bool bShowNetworkDebug;
	
private:
	// 일치 정보 타이머 핸들
	FTimerHandle TimerHandle_RequestGameState;
	
	// 주기적으로 게임 상태 요청
	void RequestGameStateUpdate();
};
