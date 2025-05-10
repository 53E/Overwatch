#include "OverwatchGameMode.h"
#include "Overwatch/Player/Cassidy/CassidyCharacter.h"
#include "UObject/ConstructorHelpers.h"

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
}

void AOverwatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// 게임 시작 시 추가 로직이 필요한 경우 여기에 작성
}
