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

    // 1. 위젯이 아직 생성되지 않은 경우 새로 생성
    if (!ScreenWidgets.Contains(Screen))
    {
        // 화면 유형에 맞는 위젯 클래스로 새 인스턴스 생성
        UOutGameUI* NewScreenWidget = CreateWidget<UOutGameUI>(
            this, ScreenWidgetClasses[Screen]);
        NewScreenWidget->MasterMenu = this;
        ContentSwitcher->AddChild(NewScreenWidget);

        ScreenWidgets.Add(Screen, NewScreenWidget);

    }

    // 2. 스위처에서 해당 화면 위젯을 활성화
    ContentSwitcher->SetActiveWidget(ScreenWidgets[Screen]);

    // 3. 화면 전환 이벤트 발생
    OnScreenChanged(Screen);
}



void UMainMenuMaster::OnScreenChanged_Implementation(EMenuScreen NewScreen)
{

}
