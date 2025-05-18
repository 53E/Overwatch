// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

AMenuPlayerController::AMenuPlayerController()
{
	bShowMouseCursor = true;
}

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 클라이언트의 로컬 플레이어 컨트롤러인지 확인
	if (IsLocalPlayerController())
	{
		// UI 설정
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(nullptr);
		SetInputMode(InputMode);

		// 카메라 설정
		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundCameras);
		if (FoundCameras.Num() > 0)
		{
			SetViewTarget(FoundCameras[0]);
		}

		// UI 위젯 생성
		if (MainMenuWidgetClass)
		{
			MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
			if (MainMenuWidget)
			{
				MainMenuWidget->AddToViewport(0);
			}
		}

		// 디버깅용 메시지
		UE_LOG(LogTemp, Warning, TEXT("MenuPlayerController: UI created on client"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MenuPlayerController: Not a local controller, skipping UI creation"));
	}
}
