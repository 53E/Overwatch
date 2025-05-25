// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PeaceKeeper.generated.h"

UCLASS()
class OVERWATCH_API APeaceKeeper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APeaceKeeper();

	UFUNCTION(BlueprintImplementableEvent , Category = "PeaceKeeper")
	void Recoil(bool bIsFanFire);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
