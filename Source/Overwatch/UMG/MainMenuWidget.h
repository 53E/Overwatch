// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutGameUI.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */

// 메뉴 상태 열거형
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
    


    // 현재 메뉴 상태
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    EMenuState CurrentMenuState;
    
    // 메뉴 전환 함수
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ChangeMenuState(EMenuState NewState);
    
    // 버튼 클릭 이벤트 처리 함수들
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
