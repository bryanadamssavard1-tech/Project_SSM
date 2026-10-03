// Fill out your copyright notice in the Description page of Project Settings.


#include "SmashGameInstance.h"
#include "MoviePlayer.h"
#include "UObject!/CoreUObjectDelegates.h"

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
	FLoadingScreenAttributes LoadingScreen;	

	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}