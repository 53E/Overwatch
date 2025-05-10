// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 기본 메뉴 상태 설정
    CurrentMenuState = EMenuState::MainMenu;
}

void UMainMenuWidget::ChangeMenuState(EMenuState NewState)
{
    // 이전 상태와 다른 경우만 처리
    if (CurrentMenuState != NewState)
    {
        CurrentMenuState = NewState;

        // 블루프린트에서 오버라이드할 수 있도록 로직은 최소화
        // 상태 변경에 따른 추가 로직은 블루프린트에서 구현
    }
}

// 버튼 클릭 이벤트 처리 함수들
void UMainMenuWidget::OnPlayButtonClicked()
{
    ChangeMenuState(EMenuState::Play);
}

void UMainMenuWidget::OnHeroesButtonClicked()
{
    ChangeMenuState(EMenuState::Heroes);
}

void UMainMenuWidget::OnStoreButtonClicked()
{
    ChangeMenuState(EMenuState::Store);
}

void UMainMenuWidget::OnBattlePassButtonClicked()
{
    ChangeMenuState(EMenuState::BattlePass);
}

void UMainMenuWidget::OnLootboxButtonClicked()
{
    ChangeMenuState(EMenuState::Lootbox);
}

void UMainMenuWidget::OnSocialButtonClicked()
{
    ChangeMenuState(EMenuState::Social);
}

void UMainMenuWidget::OnProfileButtonClicked()
{
    ChangeMenuState(EMenuState::Profile);
}

void UMainMenuWidget::OnChallengesButtonClicked()
{
    ChangeMenuState(EMenuState::Challenges);
}
