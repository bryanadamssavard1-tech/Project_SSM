// Fill out your copyright notice in the Description page of Project Settings.


#include "SmashGameInstance.h"
#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
void USmashGameInstance::Init()
{
	Super::Init();
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USmashGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USmashGameInstance::EndLoadingScreen);
}
void USmashGameInstance::BeginLoadingScreen(const FString& MapName)
{
	// Start the loading screen
	// You can customize this function to show your own loading screen widget or animation

	UE_LOG(LogTemp, Warning, TEXT("lOADING screen started"));

	FLoadingScreenAttributes LoadingScreen;	

	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 10.0f; // Set the minimum display time for the loading screen (in seconds)
	LoadingScreen.WidgetLoadingScreen = 
	SNew(SBorder)
		.BorderBackgroundColor(FLinearColor::Black)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Loading...")))
		];
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