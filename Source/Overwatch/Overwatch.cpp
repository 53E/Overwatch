// Fill out your copyright notice in the Description page of Project Settings.

#include "Overwatch.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Net/UnrealNetwork.h"

FOverwatchNetworkDebugger::FOverwatchNetworkDebugger()
{
    // 기본값 초기화
    bShowNetworkDebugInfo = false;
}

void FOverwatchNetworkDebugger::ToggleNetworkDebug()
{
    bShowNetworkDebugInfo = !bShowNetworkDebugInfo;
}

void FOverwatchNetworkDebugger::DisplayNetworkDebug(UWorld* World)
{
    if (!bShowNetworkDebugInfo || !World) return;
    
    // 디버그 정보 표시
    GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, 
        FString::Printf(TEXT("Network Role: %s"), 
        World->GetNetMode() == NM_Client ? TEXT("Client") : 
        World->GetNetMode() == NM_ListenServer ? TEXT("Listen Server") : 
        World->GetNetMode() == NM_DedicatedServer ? TEXT("Dedicated Server") : TEXT("Standalone")));
    
    // 더 많은 네트워크 통계 추가 가능
    if (World->GetNetDriver())
    {
        GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Connection Count: %d"), World->GetNetDriver()->ClientConnections.Num()));
    }
}

// 오버워치 모듈 구현
class FOverwatchModule : public FDefaultGameModuleImpl
{
    virtual void StartupModule() override
    {
        // 모듈 시작 시 초기화 작업 수행
        UE_LOG(LogTemp, Log, TEXT("Overwatch Module Started"));
    }
    
    virtual void ShutdownModule() override
    {
        // 모듈 종료 시 정리 작업 수행
        UE_LOG(LogTemp, Log, TEXT("Overwatch Module Shutdown"));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FOverwatchModule, Overwatch, "Overwatch");
