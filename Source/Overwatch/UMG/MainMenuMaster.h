// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "MainMenuMaster.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScreenChangedSignature,
//    EMenuScreen, PreviousScreen, EMenuScreen, CurrentScreen);
class UOutGameUI;

UENUM(BlueprintType)
enum class EMenuScreen : uint8
{
    MainMenu,    // 메인 메뉴 화면 (기본)
    PlaySelect,  // 게임 모드 선택 화면
    HeroGallery, // 영웅 갤러리 화면
    Lootbox,     // 전리품 상자 화면
    Settings,    // 설정 화면
    SelectPlay,  //플레이 타입 선택
    Training,     // 훈련선택
    TrainingGround //훈련장 선택

};
/**
 마스터 아래 위젯들은 모두 OutGameUI를 상속받아 이 마스터클래스의 레퍼런스를 가지고 있음!
 */
UCLASS()
class OVERWATCH_API UMainMenuMaster : public UUserWidget
{
	GENERATED_BODY()
	
    public:
    /** 특정 메뉴 화면으로 전환합니다 */
    UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
    void SwitchToScreen(EMenuScreen Screen);
    
    virtual void NativeConstruct() override;
protected:
    /** 화면 전환 이벤트 - 애니메이션 및 추가 로직에 사용 */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu|Events")
    void OnScreenChanged(EMenuScreen NewScreen);
    virtual void OnScreenChanged_Implementation(EMenuScreen NewScreen);
    

    
    /** 현재 활성화된 메뉴 화면 */
    UPROPERTY(BlueprintReadOnly, Category = "Menu|State")
    EMenuScreen CurrentScreen;
    
    /** 이전에 활성화된 메뉴 화면 */
    UPROPERTY(BlueprintReadOnly, Category = "Menu|State")
    EMenuScreen PreviousScreen;

private:
    /** 위젯 스위처 컴포넌트 - 화면 전환을 관리함 */
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Menu|Components", meta = (AllowPrivateAccess = "true"))
    UWidgetSwitcher* ContentSwitcher;
    
    //BP에서 미리 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<EMenuScreen, TSubclassOf<UOutGameUI>> ScreenWidgetClasses;
    
    // 위젯 재사용 
    UPROPERTY()
    TMap<EMenuScreen, UOutGameUI*> ScreenWidgets;
};
