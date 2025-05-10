// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OVERWATCH_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMenuPlayerController();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	UUserWidget* MainMenuWidget;

};
