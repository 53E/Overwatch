
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuMaster.h"
#include "OutGameUI.generated.h"

/**
 * 
 */

UCLASS()
class OVERWATCH_API UOutGameUI : public UUserWidget
{

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
    UMainMenuMaster* MasterMenu;

protected:

	UFUNCTION(BlueprintCallable)
	void SwitchToScreen(EMenuScreen Screen);
};
