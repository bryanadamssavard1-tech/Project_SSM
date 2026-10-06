#include "SmashGameInstance.h"

#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Texture2D.h"
#include "Widgets/Layout/SBorder.h"
// SLATE
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Misc/Paths.h"
// STYLE
#include "Styling/SlateBrush.h"
#include "Styling/CoreStyle.h"

// ============================================================
// GAME INSTANCE - INIT
// ============================================================

void USmashGameInstance::Init()
{
	Super::Init();

	// --------------------------------------------------------
	// LOADING SCREEN - PRELOAD BACKGROUND TEXTURE
	// --------------------------------------------------------

	LoadingBackgroundTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Project/UI/LoadingScreen/lightning-mcqueen--26021.lightning-mcqueen--26021")
	);

	if (LoadingBackgroundTexture)
	{
		LoadingBackgroundTexture->UpdateResource();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Loading background texture loaded successfully")
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("FAILED to load loading background texture")
		);
	}


	// --------------------------------------------------------
	// LOADING SCREEN - MAP EVENTS
	// --------------------------------------------------------

	FCoreUObjectDelegates::PreLoadMap.AddUObject(
		this,
		&USmashGameInstance::BeginLoadingScreen
	);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&USmashGameInstance::EndLoadingScreen
	);
}


// ============================================================
// LOADING SCREEN - BEGIN
// ============================================================

void USmashGameInstance::BeginLoadingScreen(const FString& MapName)
{
	// --------------------------------------------------------
	// ONLY SHOW LOADING SCREEN FOR "test" MAP
	// --------------------------------------------------------
// ============================================================
	// LOADING SCREEN - ONLY COMBAT MAPS
	// ============================================================

	if (!MapName.Contains(TEXT("Final_Arena")))
	{
		return;
	}



	UE_LOG(
		LogTemp,
		Warning,
		TEXT("LOADING SCREEN STARTED - Map: %s"),
		*MapName
	);


	// --------------------------------------------------------
	// CHECK MOVIE PLAYER
	// --------------------------------------------------------

	if (!GetMoviePlayer())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Movie Player is NOT available")
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Movie Player is available")
	);



	// --------------------------------------------------------
	// CREATE LOADING SCREEN SETTINGS
	// --------------------------------------------------------

	FLoadingScreenAttributes LoadingScreen;

	// ============================================================
	// LOADING SCREEN - BEHAVIOR
	// ============================================================

	// Ferme automatiquement le loading lorsque la map est prête
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;

	// IMPORTANT : ne pas attendre StopMovie() manuellement
	LoadingScreen.bWaitForManualStop = false;

	// Garde quand même le loading screen minimum 10 secondes
	LoadingScreen.MinimumLoadingScreenDisplayTime = 10.0f;


	// --------------------------------------------------------
	// CREATE BACKGROUND BRUSH
	// --------------------------------------------------------

	LoadingBackgroundBrush = MakeShared<FSlateBrush>();

	if (LoadingBackgroundTexture)
	{
		LoadingBackgroundBrush->SetResourceObject(
			LoadingBackgroundTexture
		);
	}

	LoadingBackgroundBrush->ImageSize =
		FVector2D(
			1920.0f,
			1080.0f
		);
	// ============================================================
// LOADING SCREEN - FORMAT STAGE NAME
// ============================================================

	FString StageName = MapName;

	// Enlève le chemin si Unreal en fournit un
	StageName = FPaths::GetBaseFilename(StageName);

	// Remplace les "_" par des espaces
	StageName.ReplaceInline(TEXT("_"), TEXT(" "));

	// Met le nom en majuscules
	StageName = StageName.ToUpper();

	// ========================================================
	// CREATE LOADING SCREEN UI
	// ========================================================

	// ============================================================
// CREATE PLAYER PORTRAIT BRUSHES
// ============================================================

// PLAYER 1
	if (Player1Portrait)
	{
		Player1PortraitBrush = MakeShared<FSlateBrush>();

		Player1PortraitBrush->SetResourceObject(
			Player1Portrait
		);

		Player1PortraitBrush->ImageSize =
			FVector2D(
				420.0f,
				420.0f
			);
	}


	// PLAYER 2
	if (Player2Portrait)
	{
		Player2PortraitBrush = MakeShared<FSlateBrush>();

		Player2PortraitBrush->SetResourceObject(
			Player2Portrait
		);

		Player2PortraitBrush->ImageSize =
			FVector2D(
				420.0f,
				420.0f
			);
	}


	// ============================================================
	// CREATE LOADING SCREEN UI
	// ============================================================
	LoadingScreen.WidgetLoadingScreen =

		SNew(SOverlay)


		// ============================================================
		// 01 - BACKGROUND
		// ============================================================

		+SOverlay::Slot()
		[
			SNew(SImage)
				.Image(LoadingBackgroundBrush.Get())
		]


		// ============================================================
		// 02 - CINEMATIC DARK FILTER
		// ============================================================

		+SOverlay::Slot()
		[
			SNew(SColorBlock)
				.Color(
					FLinearColor(
						0.0f,
						0.0f,
						0.0f,
						0.58f
					)
				)
		]


	// ============================================================
	// 03 - TOP BLACK CINEMATIC BAR
	// ============================================================

	+SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
				.HeightOverride(115.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								0.01f,
								0.01f,
								0.01f,
								0.92f
							)
						)
				]
		]


	// ============================================================
	// 04 - BOTTOM BLACK CINEMATIC BAR
	// ============================================================

	+SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		[
			SNew(SBox)
				.HeightOverride(105.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								0.01f,
								0.01f,
								0.01f,
								0.94f
							)
						)
				]
		]


	// ============================================================
	// 05 - PLAYER 1 BACK PANEL
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(
			45.0f,
			120.0f,
			0.0f,
			100.0f
		)
		[
			SNew(SBox)
				.WidthOverride(650.0f)
				.HeightOverride(650.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								0.15f,
								0.01f,
								0.01f,
								0.42f
							)
						)
				]
		]


	// ============================================================
	// 06 - PLAYER 2 BACK PANEL
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(
			0.0f,
			120.0f,
			45.0f,
			100.0f
		)
		[
			SNew(SBox)
				.WidthOverride(650.0f)
				.HeightOverride(650.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								0.01f,
								0.04f,
								0.18f,
								0.42f
							)
						)
				]
		]


	// ============================================================
	// 07 - PLAYER 1 ACCENT STRIP
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(
			45.0f,
			120.0f,
			0.0f,
			100.0f
		)
		[
			SNew(SBox)
				.WidthOverride(9.0f)
				.HeightOverride(650.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								1.0f,
								0.06f,
								0.03f,
								1.0f
							)
						)
				]
		]


	// ============================================================
	// 08 - PLAYER 2 ACCENT STRIP
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(
			0.0f,
			120.0f,
			45.0f,
			100.0f
		)
		[
			SNew(SBox)
				.WidthOverride(9.0f)
				.HeightOverride(650.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								0.08f,
								0.35f,
								1.0f,
								1.0f
							)
						)
				]
		]


	// ============================================================
	// 09 - PLAYER 1 PORTRAIT
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(
			55.0f,
			100.0f,
			0.0f,
			95.0f
		)
		[
			SNew(SBox)
				.WidthOverride(650.0f)
				.HeightOverride(650.0f)
				[
					SNew(SImage)
						.Image(
							Player1PortraitBrush.IsValid()
							? Player1PortraitBrush.Get()
							: nullptr
						)
				]
		]


	// ============================================================
	// 10 - PLAYER 2 PORTRAIT
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(
			0.0f,
			100.0f,
			55.0f,
			95.0f
		)
		[
			SNew(SBox)
				.WidthOverride(650.0f)
				.HeightOverride(650.0f)
				[
					SNew(SImage)
						.Image(
							Player2PortraitBrush.IsValid()
							? Player2PortraitBrush.Get()
							: nullptr
						)
				]
		]


	// ============================================================
	// 11 - HUGE BACKGROUND VS
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
				.Text(
					FText::FromString(
						TEXT("VS")
					)
				)
				.ColorAndOpacity(
					FLinearColor(
						1.0f,
						1.0f,
						1.0f,
						0.06f
					)
				)
				.Font(
					FCoreStyle::GetDefaultFontStyle(
						"Bold",
						270
					)
				)
		]


	// ============================================================
	// 12 - CENTER VERTICAL LIGHT
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
				.WidthOverride(3.0f)
				.HeightOverride(500.0f)
				[
					SNew(SColorBlock)
						.Color(
							FLinearColor(
								1.0f,
								1.0f,
								1.0f,
								0.35f
							)
						)
				]
		]


	// ============================================================
	// 13 - MAIN VS
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)

				.Padding(
					FMargin(
						32.0f,
						18.0f
					)
				)

				.BorderBackgroundColor(
					FLinearColor(
						0.0f,
						0.0f,
						0.0f,
						0.78f
					)
				)

				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("VS")
							)
						)

						.ColorAndOpacity(
							FLinearColor::White
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								100
							)
						)
				]
		]


	// ============================================================
	// 14 - PLAYER 1 INFORMATION
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(
			75.0f,
			0.0f,
			0.0f,
			125.0f
		)
		[
			SNew(SVerticalBox)

				// PLAYER NUMBER
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("PLAYER 1")
							)
						)

						.ColorAndOpacity(
							FLinearColor(
								1.0f,
								0.12f,
								0.06f,
								1.0f
							)
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								18
							)
						)
				]

			// TEMP CHARACTER NAME
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("FIGHTER")
							)
						)

						.ColorAndOpacity(
							FLinearColor::White
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								44
							)
						)
				]
		]


	// ============================================================
	// 15 - PLAYER 2 INFORMATION
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(
			0.0f,
			0.0f,
			75.0f,
			125.0f
		)
		[
			SNew(SVerticalBox)

				// PLAYER NUMBER
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("PLAYER 2")
							)
						)

						.ColorAndOpacity(
							FLinearColor(
								0.15f,
								0.45f,
								1.0f,
								1.0f
							)
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								18
							)
						)
				]

			// TEMP CHARACTER NAME
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("FIGHTER")
							)
						)

						.ColorAndOpacity(
							FLinearColor::White
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								44
							)
						)
				]
		]


	// ============================================================
	// 16 - STAGE CARD
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(
			0.0f,
			25.0f,
			0.0f,
			0.0f
		)
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("BATTLE STAGE")
							)
						)

						.ColorAndOpacity(
							FLinearColor(
								0.60f,
								0.60f,
								0.60f,
								1.0f
							)
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								13
							)
						)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(
					0.0f,
					2.0f
				)
				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								StageName
							)
						)

						.ColorAndOpacity(
							FLinearColor::White
						)

						.Font(
							FCoreStyle::GetDefaultFontStyle(
								"Bold",
								31
							)
						)
				]
		]


	// ============================================================
	// 17 - BOTTOM LOADING TEXT
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(
			0.0f,
			0.0f,
			0.0f,
			54.0f
		)
		[
			SNew(STextBlock)

				.Text(
					FText::FromString(
						TEXT("PREPARING BATTLE")
					)
				)

				.ColorAndOpacity(
					FLinearColor(
						0.70f,
						0.70f,
						0.70f,
						1.0f
					)
				)

				.Font(
					FCoreStyle::GetDefaultFontStyle(
						"Bold",
						12
					)
				)
		]


	// ============================================================
	// 18 - LOADING BAR
	// ============================================================

	+SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(
			0.0f,
			0.0f,
			0.0f,
			30.0f
		)
		[
			SNew(SBox)

				.WidthOverride(500.0f)

				.HeightOverride(5.0f)

				[
					SNew(SProgressBar)

						.Percent(
							TOptional<float>()
						)
				]
		];
	// --------------------------------------------------------
	// SETUP LOADING SCREEN
	// --------------------------------------------------------

	GetMoviePlayer()
		->SetupLoadingScreen(
			LoadingScreen
		);


	// --------------------------------------------------------
	// START LOADING SCREEN
	// --------------------------------------------------------

	GetMoviePlayer()
		->PlayMovie();


	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Movie Player PlayMovie called")
	);
}


// ============================================================
// LOADING SCREEN - UPDATE ANIMATION
// ============================================================



void USmashGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
	// ============================================================
	// LOADING SCREEN - MAP FINISHED
	// ============================================================

	if (LoadedWorld)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MAP FINISHED LOADING: %s"),
			*LoadedWorld->GetName()
		);
	}

	// NE PAS enlever UpdateLoadingAnimation ici.
	// Le MoviePlayer va fermer automatiquement le loading screen
	// après le MinimumLoadingScreenDisplayTime.
}