// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SmashGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COMBO_TEST_API USmashGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	virtual void Init() override;

	void
		BeginLoadingScreen(const FString& MapName);	

	void
		EndLoadingScreen(UWorld* LoadedWorld);

};
