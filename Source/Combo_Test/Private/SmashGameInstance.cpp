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
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Misc/Optional.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Colors/SColorBlock.h"

void USmashGameInstance::Init()
{
	Super::Init();
	// LOADING SCREEN - PRELOAD BACKGROUND TEXTURE

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
	LoadingScreen.MinimumLoadingScreenDisplayTime = 10.0f; // Set the minimum display time for the loading screen (in seconds)
	LoadingBackgroundBrush = MakeShared<FSlateBrush>();
	GetMoviePlayer()->OnMoviePlaybackTick().AddUObject(
		this,
		&USmashGameInstance::UpdateLoadingAnimation
	);
	LoadingBackgroundBrush->SetResourceObject(LoadingBackgroundTexture);
	LoadingBackgroundBrush->ImageSize = FVector2D(1920.0f, 1080.0f);
	LoadingScreen.WidgetLoadingScreen =
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SImage)
				.Image(LoadingBackgroundBrush.Get())
		]

		// LOADING SCREEN - TEXT + BAR
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("LOADING...")))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
						.WidthOverride(180.0f)
						.HeightOverride(8.0f)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SOverlay)

								// BAR BACKGROUND
								+ SOverlay::Slot()
								[
									SNew(SColorBlock)
										.Color(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f))
								]

								// MOVING WHITE SEGMENT
								+ SOverlay::Slot()
								.HAlign(HAlign_Left)
								[
									SAssignNew(LoadingBarWidget, SBox)
										.WidthOverride(40.0f)
										.HeightOverride(8.0f)
										[
											SNew(SColorBlock)
												.Color(FLinearColor::White)
										]
								]
						]
				]
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
	
void USmashGameInstance::UpdateLoadingAnimation(float DeltaTime)
{
	LoadingBarOffset += DeltaTime * 14.0f;

		if (LoadingBarOffset > 140.0f)
		{
			LoadingBarOffset = 0.0f;
		}

		// LOADING SCREEN - MOVE BAR
		if (LoadingBarWidget.IsValid())
		{
			LoadingBarWidget->SetRenderTransform(
				FVector2D(LoadingBarOffset, 0.0f)
			);
		}
}

void USmashGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
	// End the loading screen
	// You can customize this function to hide your own loading screen widget or animation

}