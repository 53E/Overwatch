// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutGameUI.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */

// �޴� ���� ������
UENUM(BlueprintType)
enum class EMenuState : uint8
{
    MainMenu,
    Play,
    Heroes,
    Store,
    BattlePass,
    Lootbox,
    Social,
    Profile,
    Challenges
};

UCLASS()
class OVERWATCH_API UMainMenuWidget : public UOutGameUI
{
	GENERATED_BODY()
	public:

	void OpenLevel();


    // ���� �޴� ����
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    EMenuState CurrentMenuState;
    
    // �޴� ��ȯ �Լ�
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ChangeMenuState(EMenuState NewState);
    
    // ��ư Ŭ�� �̺�Ʈ ó�� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnPlayButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnHeroesButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnStoreButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnBattlePassButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnLootboxButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnSocialButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnProfileButtonClicked();
    
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void OnChallengesButtonClicked();
    
protected:
    virtual void NativeConstruct() override;
	
};
