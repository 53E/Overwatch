// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/MenuPlayerController.h"
#include "Camera/CameraActor.h"
#include "Blueprint/UserWidget.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    PlayerControllerClass = APlayerController::StaticClass();
    DefaultPawnClass = nullptr;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();


    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundCameras);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PC)
    {
        // UI 입력 모드 
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;

        if (FoundCameras.Num() > 0)
        {
            PC->SetViewTarget(FoundCameras[0]);
        }

        if (MainMenuWidgetClass)
        {
            UUserWidget* MainMenuWidget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
            if (MainMenuWidget)
            {
                MainMenuWidget->AddToViewport(0);
            }
        }
    }
}
