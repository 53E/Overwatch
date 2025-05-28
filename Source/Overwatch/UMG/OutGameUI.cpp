// Fill out your copyright notice in the Description page of Project Settings.



#include "OutGameUI.h"
#include "Sound/SoundBase.h"

void UOutGameUI::SwitchToScreen(EMenuScreen Screen)
{
	if (MasterMenu)
	{
		MasterMenu->SwitchToScreen(Screen);
	}
}

void UOutGameUI::PlayLobbySound(USoundBase* LobbySound)
{
	if (MasterMenu)
	{
		MasterMenu->PlayLobbySound(LobbySound);
	}
}

void UOutGameUI::StopLobbySound()
{
	if (MasterMenu)
	{
		MasterMenu->StopLobbySound();
	}
}


void UOutGameUI::TravelTrainingRoom_Implementation()
{
	GetWorld()->ServerTravel(TEXT("/Game/Overwatch/Level/TrainingRoom?listen"));
}
