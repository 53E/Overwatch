// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 네트워크 디버그 클래스
class FOverwatchNetworkDebugger
{
public:
    FOverwatchNetworkDebugger();
    
    // 디버그 표시 토글
    void ToggleNetworkDebug();
    
    // 디버그 정보 표시
    void DisplayNetworkDebug(UWorld* World);
    
    // 네트워크 디버그 표시 여부
    bool IsNetworkDebugEnabled() const { return bShowNetworkDebugInfo; }
    
private:
    bool bShowNetworkDebugInfo;
};

// 전역 디버그 객체
inline FOverwatchNetworkDebugger GNetworkDebugger;

// 윈도우 의존성을 피하기 위한 서버 플래트폼 감지 매크로
#if WITH_SERVER_CODE
#define OVERWATCH_WITH_SERVER 1
#else
#define OVERWATCH_WITH_SERVER 0
#endif

