// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MenuPlayerController.h"
#include "Camera/CameraActor.h"
#include "Blueprint/UserWidget.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    PlayerControllerClass = AMenuPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // 서버에서는 기본 설정만 처리
    // UI와 카메라 설정은 MenuPlayerController에서 처리
    UE_LOG(LogTemp, Warning, TEXT("MainMenuGameMode: Started on server"));
}
