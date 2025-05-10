// Fill out your copyright notice in the Description page of Project Settings.



#include "OutGameUI.h"

void UOutGameUI::SwitchToScreen(EMenuScreen Screen)
{
	if (MasterMenu)
	{
		MasterMenu->SwitchToScreen(Screen);
	}

}
