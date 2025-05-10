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
    MainMenu,    // ���� �޴� ȭ�� (�⺻)
    PlaySelect,  // ���� ��� ���� ȭ��
    HeroGallery, // ���� ������ ȭ��
    Lootbox,     // ����ǰ ���� ȭ��
    Settings,    // ���� ȭ��
    SelectPlay,  //�÷��� Ÿ�� ����
    Training,     // �Ʒü���
    TrainingGround //�Ʒ��� ����

};
/**
 ������ �Ʒ� �������� ��� OutGameUI�� ��ӹ޾� �� ������Ŭ������ ���۷����� ������ ����!
 */
UCLASS()
class OVERWATCH_API UMainMenuMaster : public UUserWidget
{
	GENERATED_BODY()
	
    public:
    /** Ư�� �޴� ȭ������ ��ȯ�մϴ� */
    UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
    void SwitchToScreen(EMenuScreen Screen);
    
    virtual void NativeConstruct() override;
protected:
    /** ȭ�� ��ȯ �̺�Ʈ - �ִϸ��̼� �� �߰� ������ ��� */
    UFUNCTION(BlueprintNativeEvent, Category = "Menu|Events")
    void OnScreenChanged(EMenuScreen NewScreen);
    virtual void OnScreenChanged_Implementation(EMenuScreen NewScreen);
    

    
    /** ���� Ȱ��ȭ�� �޴� ȭ�� */
    UPROPERTY(BlueprintReadOnly, Category = "Menu|State")
    EMenuScreen CurrentScreen;
    
    /** ������ Ȱ��ȭ�� �޴� ȭ�� */
    UPROPERTY(BlueprintReadOnly, Category = "Menu|State")
    EMenuScreen PreviousScreen;

private:
    /** ���� ����ó ������Ʈ - ȭ�� ��ȯ�� ������ */
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Menu|Components", meta = (AllowPrivateAccess = "true"))
    UWidgetSwitcher* ContentSwitcher;
    
    //BP���� �̸� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<EMenuScreen, TSubclassOf<UOutGameUI>> ScreenWidgetClasses;
    
    // ���� ���� 
    UPROPERTY()
    TMap<EMenuScreen, UOutGameUI*> ScreenWidgets;
};
