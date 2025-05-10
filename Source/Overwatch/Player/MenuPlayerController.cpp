// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMenuPlayerController::AMenuPlayerController()
{
	bShowMouseCursor = true;
}

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(nullptr);
	SetInputMode(InputMode);

	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport(0);
		}
	}
}
