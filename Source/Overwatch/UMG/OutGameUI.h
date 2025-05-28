
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuMaster.h"
#include "OutGameUI.generated.h"

/**
 * 
 */
class USoundBase;

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

	UFUNCTION(BlueprintCallable)
	void PlayLobbySound(USoundBase* LobbySound);
	
	UFUNCTION(BlueprintCallable)
	void StopLobbySound();

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void TravelTrainingRoom();
	void TravelTrainingRoom_Implementation();

};
