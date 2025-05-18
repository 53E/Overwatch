// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuWidget.h"



void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // �⺻ �޴� ���� ����
    CurrentMenuState = EMenuState::MainMenu;
}

void UMainMenuWidget::OpenLevel()
{
    
}

void UMainMenuWidget::ChangeMenuState(EMenuState NewState)
{
    // ���� ���¿� �ٸ� ��츸 ó��
    if (CurrentMenuState != NewState)
    {
        CurrentMenuState = NewState;

        // ��������Ʈ���� �������̵��� �� �ֵ��� ������ �ּ�ȭ
        // ���� ���濡 ���� �߰� ������ ��������Ʈ���� ����
    }
}

// ��ư Ŭ�� �̺�Ʈ ó�� �Լ���
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
