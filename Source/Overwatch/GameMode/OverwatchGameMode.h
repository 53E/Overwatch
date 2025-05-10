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
};
