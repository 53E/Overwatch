// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuMaster.h"
#include "OutGameUI.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void UMainMenuMaster::NativeConstruct()
{
    //CurrentScreen = EMenuScreen::MainMenu;
	/*UOutGameUI* FirstScreenWidget = CreateWidget<UOutGameUI>(
		this, ScreenWidgetClasses[CurrentScreen]);
	FirstScreenWidget->MasterMenu = this;

	ContentSwitcher->AddChild(FirstScreenWidget);
	ScreenWidgets.Add(CurrentScreen, FirstScreenWidget);
	ContentSwitcher->SetActiveWidget(ScreenWidgets[CurrentScreen]);*/
    SwitchToScreen(EMenuScreen::MainMenu);
    OnScreenChanged(CurrentScreen);
}

void UMainMenuMaster::PlayLobbySound(USoundBase* LobbySound)
{
	if (LobbySound)
	{
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LobbySound);
		
		if (AudioComponent)
		{
			LobbyAudioComp = AudioComponent;
			AudioComponent->bAutoDestroy = true; // 사운드 끝나면 자동 제거
			AudioComponent->SetVolumeMultiplier(0.8f); // 볼륨 조절
			AudioComponent->SetPitchMultiplier(1.0f);  // 피치 조절
		}
	}
}

void UMainMenuMaster::StopLobbySound()
{
	if (LobbyAudioComp)
	{
		LobbyAudioComp->Stop();
	}
}

void UMainMenuMaster::SwitchToScreen(EMenuScreen Screen)
{
    //if (CurrentScreen == Screen) return;

    PreviousScreen = CurrentScreen;
    CurrentScreen = Screen;

    // 1. ������ ���� �������� ���� ��� ���� ����
    if (!ScreenWidgets.Contains(Screen))
    {
        // ȭ�� ������ �´� ���� Ŭ������ �� �ν��Ͻ� ����
        UOutGameUI* NewScreenWidget = CreateWidget<UOutGameUI>(
            this, ScreenWidgetClasses[Screen]);
        NewScreenWidget->MasterMenu = this;
        ContentSwitcher->AddChild(NewScreenWidget);

        ScreenWidgets.Add(Screen, NewScreenWidget);

    }

    // 2. ����ó���� �ش� ȭ�� ������ Ȱ��ȭ
    ContentSwitcher->SetActiveWidget(ScreenWidgets[Screen]);

    // 3. ȭ�� ��ȯ �̺�Ʈ �߻�
    OnScreenChanged(Screen);
}



void UMainMenuMaster::OnScreenChanged_Implementation(EMenuScreen NewScreen)
{

}
