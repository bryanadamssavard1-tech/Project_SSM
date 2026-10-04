// Fill out your copyright notice in the Description page of Project Settings.


#include "SmashGameInstance.h"
#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateBrush.h"

void USmashGameInstance::Init()
{
	Super::Init();
	// LOADING SCREEN - PRELOAD BACKGROUND TEXTURE
	LoadingBackgroundTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Project/UI/LoadingScreen/lightning-mcqueen--26021.lightning-mcqueen--26021")
	);

	if (LoadingBackgroundTexture)
	{
		LoadingBackgroundTexture->UpdateResource();
	}
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USmashGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USmashGameInstance::EndLoadingScreen);
}
void USmashGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (!MapName.Contains(TEXT("test")))
	{
		return;
	}
	// Start the loading screen
	// You can customize this function to show your own loading screen widget or animation

	UE_LOG(LogTemp, Warning, TEXT("lOADING screen started"));

	FLoadingScreenAttributes LoadingScreen;	
	LoadingBackgroundTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Project/UI/LoadingScreen/lightning-mcqueen--26021.lightning-mcqueen--26021")
	);
	// LOADING SCREEN - CREATE BACKGROUND BRUSH

	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 1.0f; // Set the minimum display time for the loading screen (in seconds)
	LoadingScreen.WidgetLoadingScreen =
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor::Black)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("LOADING...")))
		];
	LoadingBackgroundBrush = MakeShared<FSlateBrush>();

	LoadingBackgroundBrush->SetResourceObject(LoadingBackgroundTexture);
	LoadingBackgroundBrush->ImageSize = FVector2D(1920.0f, 1080.0f);
	LoadingScreen.WidgetLoadingScreen =
		SNew(SImage)
		.Image(LoadingBackgroundBrush.Get());
	if (!GetMoviePlayer())
		{
		UE_LOG(LogTemp, Warning, TEXT("Movie Player is not available"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Movie Player is available"));

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	GetMoviePlayer()->PlayMovie();
	UE_LOG(LogTemp, Warning, TEXT("Movie Player Play Called"));

		
		
	
}
void USmashGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
	// End the loading screen
	// You can customize this function to hide your own loading screen widget or animation

}