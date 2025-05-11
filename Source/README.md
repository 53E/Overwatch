# 오버워치 캐서디 멀티플레이어 구현

## 개요

이 프로젝트는 오버워치의 캐서디 캐릭터를 구현하고 데디케이티드 서버에서 멀티플레이어 게임을 지원합니다.

## 기능

- 캐서디 캐릭터 구현 (1인칭 슈팅)
- 기본 무기 및 능력 (팬 더 해머, 섬광탄, 구르기)
- 클라이언트-서버 네트워크 모델
- 멀티플레이어 게임 기능
- 데디케이티드 서버 지원

## 빌드 방법

1. 프로젝트 파일(.uproject)을 우클릭하고 "Generate Visual Studio project files"를 선택합니다.
2. 생성된 .sln 파일을 Visual Studio에서 엽니다.
3. Development Editor 구성으로 빌드합니다.
4. 완료 후 언리얼 에디터에서 프로젝트를 열 수 있습니다.

## 데디케이티드 서버 빌드

1. 언리얼 에디터에서 File > Package Project > Build Configuration에서 "Development Server"를 선택합니다.
2. Platforms 메뉴에서 대상 플랫폼(Windows 등)을 선택합니다.
3. 패키징이 완료될 때까지 기다립니다.

또는 명령줄에서 다음 명령을 실행합니다:
```
<UnrealEnginePath>/Engine/Build/BatchFiles/RunUAT.bat BuildCookRun -project=<ProjectPath>/<ProjectName>.uproject -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -server -serverplatform=Win64 -noclient -build -stage -pak -archive -archivedirectory=<OutputPath>
```

## 서버 실행 방법

서버를 실행하려면 다음 명령을 사용합니다:
```
<GameName>Server.exe <MapName> -server -log -PORT=7777 -MAXPLAYERS=12
```

예시:
```
OverwatchServer.exe /Game/Maps/TestMap -server -log -PORT=7777 -MAXPLAYERS=12
```

## 클라이언트 실행 방법

클라이언트를 실행하여 서버에 연결하려면 다음 명령을 사용합니다:
```
<GameName>.exe <ServerIP> -game
```

예시:
```
Overwatch.exe 192.168.1.100 -game
```

또는 콘솔을 사용하여 접속할 수도 있습니다:
```
open 192.168.1.100
```

## 주요 클래스 설명

- **OverwatchCharacter**: 모든 캐릭터의 기본 클래스
- **CassidyCharacter**: 캐서디 캐릭터 구현 
- **OverwatchGameMode**: 게임 규칙 및 플레이어 관리
- **OverwatchGameState**: 게임 상태 및 팀 정보
- **OverwatchPlayerState**: 플레이어 통계 및 팀 정보
- **OverwatchPlayerController**: 플레이어 입력 및 네트워크 디버깅
- **OverwatchGameSession**: 세션 및 플레이어 관리

## 디버깅

게임 실행 중에 F10 키를 눌러 네트워크 디버깅 정보를 확인할 수 있습니다.

## 맵 설정

맵에서 다음 작업을 수행해야 합니다:
1. PlayerStart 액터를 배치하고 적절한 팀 태그를 설정합니다:
   - RedTeam - 빨간팀 스폰 포인트
   - BlueTeam - 파란팀 스폰 포인트
2. 게임모드를 BP_OverwatchGameMode로 설정합니다.
