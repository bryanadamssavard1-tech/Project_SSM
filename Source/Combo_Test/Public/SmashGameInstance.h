// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Styling/SlateBrush.h"

#include "SmashGameInstance.generated.h"
/**
 * 
 */
	class UTexture2D;
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
private:

	// LOADING SCREEN - PERSISTENT BACKGROUND BRUSH
	TSharedPtr<FSlateBrush> LoadingBackgroundBrush;
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadingBackgroundTexture;
};
