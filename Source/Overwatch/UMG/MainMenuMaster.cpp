// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuMaster.h"
#include "OutGameUI.h"

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
