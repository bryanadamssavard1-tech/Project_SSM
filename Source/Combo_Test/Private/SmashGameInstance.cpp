
#include "SmashGameInstance.h"

#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"

// ============================================================
// SLATE - BASIC WIDGETS
// ============================================================

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

// ============================================================
// SLATE - STYLE
// ============================================================

#include "Styling/SlateBrush.h"
#include "Styling/CoreStyle.h"


// ============================================================
// 01 - GAME INSTANCE INITIALIZATION
//
// Cette fonction est appelée lorsque le GameInstance démarre.
// On charge ici les textures principales du loading screen
// et on connecte les événements de changement de map.
// ============================================================

void USmashGameInstance::Init()
{
	Super::Init();


	// --------------------------------------------------------
	// LOAD DEFAULT BACKGROUND
	//
	// Pour l'instant on utilise une seule image.
	// Plus tard chaque stage pourra avoir son propre background.
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
			TEXT("Loading Screen: Background loaded")
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Loading Screen: Background NOT found")
		);
	}


	// --------------------------------------------------------
	// LOAD CENTER ENERGY SLASH
	//
	// IMPORTANT:
	// Modifie ce chemin si ta texture n'est pas située ici.
	// --------------------------------------------------------

	CenterSlashTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Project/UI/LoadingScreen/FX/T_UI_CenterSlash.T_UI_CenterSlash")
	);


	if (CenterSlashTexture)
	{
		CenterSlashTexture->UpdateResource();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Loading Screen: Center slash loaded")
		);
	}


	// --------------------------------------------------------
	// REGISTER MAP EVENTS
	//
	// PreLoadMap = juste avant le changement de niveau.
	// PostLoadMapWithWorld = lorsque le niveau est prêt.
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
// 02 - BEGIN LOADING SCREEN
//
// Construit et démarre tout le loading screen.
// ============================================================
void USmashGameInstance::BeginLoadingScreen(const FString& MapName)
{
	// ========================================================
	// 01 - MAP FILTER
	//
	// Le loading screen doit fonctionner seulement
	// pour la map de combat.
	// ========================================================

	if (!MapName.Contains(TEXT("Final_Arena")))
	{
		// ----------------------------------------------------
		// IMPORTANT :
		// Si on va vers le menu de sélection,
		// on s'assure qu'aucun MoviePlayer ne reste actif.
		// ----------------------------------------------------

		if (GetMoviePlayer() && GetMoviePlayer()->IsMovieCurrentlyPlaying())
		{
			GetMoviePlayer()->StopMovie();
		}

		return;
	}


	// ========================================================
	// 02 - CHECK MOVIE PLAYER
	// ========================================================

	if (!GetMoviePlayer())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Loading Screen: MoviePlayer unavailable")
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Loading Screen starting for: %s"),
		*MapName
	);
	// ========================================================
	// 03 - LOADING SCREEN SETTINGS
	// ========================================================

	FLoadingScreenAttributes LoadingScreen;


	// Ferme automatiquement lorsque la map est chargée.
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;


	// Ne nécessite pas StopMovie() manuellement.
	LoadingScreen.bWaitForManualStop = false;


	// Garde le loading screen au moins 10 secondes.
	LoadingScreen.MinimumLoadingScreenDisplayTime = 10.0f;


	// ========================================================
	// 04 - FORMAT STAGE NAME
	//
	// Final_Arena devient FINAL ARENA.
	// ========================================================

	FString StageName = MapName;

	StageName = FPaths::GetBaseFilename(StageName);

	StageName.ReplaceInline(
		TEXT("_"),
		TEXT(" ")
	);

	StageName = StageName.ToUpper();


	// ========================================================
	// 05 - CREATE BACKGROUND BRUSH
	// ========================================================

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


	// ========================================================
	// 06 - CREATE CENTER SLASH BRUSH
	// ========================================================

	CenterSlashBrush.Reset();


	if (CenterSlashTexture)
	{
		CenterSlashBrush = MakeShared<FSlateBrush>();

		CenterSlashBrush->SetResourceObject(
			CenterSlashTexture
		);

		CenterSlashBrush->ImageSize =
			FVector2D(
				500.0f,
				800.0f
			);
	}


	// ========================================================
	// 07 - PREPARE PLAYER DATA
	//
	// Maximum 4 joueurs.
	// ========================================================

	const int32 PlayerCount =
		FMath::Clamp(
			LoadingPlayers.Num(),
			0,
			4
		);


	// ========================================================
	// 08 - CREATE PLAYER PORTRAIT BRUSHES
	// ========================================================

	PlayerPortraitBrushes.Empty();

	PlayerPortraitBrushes.SetNum(
		PlayerCount
	);


	for (int32 Index = 0; Index < PlayerCount; Index++)
	{
		if (LoadingPlayers[Index].Portrait)
		{
			PlayerPortraitBrushes[Index] =
				MakeShared<FSlateBrush>();

			PlayerPortraitBrushes[Index]
				->SetResourceObject(
					LoadingPlayers[Index].Portrait
				);

			PlayerPortraitBrushes[Index]
				->ImageSize =
				FVector2D(
					700.0f,
					700.0f
				);
		}
	}


	// ========================================================
	// 09 - PLAYER COLORS
	//
	// Chaque joueur possède sa propre couleur.
	// ========================================================

	const FLinearColor PlayerColors[4] =
	{
		FLinearColor(
			1.00f,
			0.10f,
			0.03f,
			1.00f
		),

		FLinearColor(
			0.05f,
			0.40f,
			1.00f,
			1.00f
		),

		FLinearColor(
			0.15f,
			0.95f,
			0.35f,
			1.00f
		),

		FLinearColor(
			0.85f,
			0.25f,
			1.00f,
			1.00f
		)
	};


	// ========================================================
	// 10 - HELPER : CREATE PLAYER CARD
	//
	// Cette fonction locale génère automatiquement une carte
	// complète pour n'importe quel joueur.
	//
	// Elle contient :
	// - glow
	// - portrait
	// - numéro du joueur
	// - nom du personnage
	// ========================================================

	auto CreatePlayerCard =
		[this, &PlayerColors](
			int32 Index,
			float CardWidth,
			float CardHeight
			)
		-> TSharedRef<SWidget>
		{
			// ----------------------------------------------------
			// SAFETY CHECK
			// ----------------------------------------------------

			if (!LoadingPlayers.IsValidIndex(Index))
			{
				return
					SNew(SBox);
			}


			// ----------------------------------------------------
			// PLAYER DISPLAY NUMBER
			// ----------------------------------------------------

			const FString PlayerLabel =
				FString::Printf(
					TEXT("PLAYER %d"),
					Index + 1
				);


			// ----------------------------------------------------
			// CHARACTER NAME
			// ----------------------------------------------------

			FString CharacterName =
				LoadingPlayers[Index].CharacterName;


			if (CharacterName.IsEmpty())
			{
				CharacterName =
					TEXT("FIGHTER");
			}


			CharacterName =
				CharacterName.ToUpper();


			// ----------------------------------------------------
			// PORTRAIT BRUSH
			// ----------------------------------------------------

			const FSlateBrush* PortraitBrush =
				nullptr;


			if (
				PlayerPortraitBrushes.IsValidIndex(Index)
				&&
				PlayerPortraitBrushes[Index].IsValid()
				)
			{
				PortraitBrush =
					PlayerPortraitBrushes[Index].Get();
			}


			// ----------------------------------------------------
			// PLAYER COLOR
			// ----------------------------------------------------

			const FLinearColor AccentColor =
				PlayerColors[Index];


			// ----------------------------------------------------
			// CREATE CARD
			// ----------------------------------------------------

			return

				SNew(SBox)

				.WidthOverride(
					CardWidth
				)

				.HeightOverride(
					CardHeight
				)

				[

					SNew(SOverlay)


						// ============================================
						// CARD - DARK BACKGROUND
						// ============================================

						+SOverlay::Slot()
						[
							SNew(SBorder)

								.BorderBackgroundColor(
									FLinearColor(
										0.01f,
										0.01f,
										0.015f,
										0.88f
									)
								)
						]


					// ============================================
					// CARD - PLAYER COLOR GLOW
					// ============================================

					+SOverlay::Slot()
						[
							SNew(SBorder)

								.BorderBackgroundColor(
									FLinearColor(
										AccentColor.R,
										AccentColor.G,
										AccentColor.B,
										0.18f
									)
								)
						]


					// ============================================
					// CARD - TOP COLOR STRIP
					// ============================================

					+SOverlay::Slot()

						.VAlign(
							VAlign_Top
						)

						[
							SNew(SBox)

								.HeightOverride(
									7.0f
								)

								[
									SNew(SColorBlock)

										.Color(
											AccentColor
										)
								]
						]


					// ============================================
					// CARD - PORTRAIT
					// ============================================

					+SOverlay::Slot()

						.HAlign(
							HAlign_Fill
						)

						.VAlign(
							VAlign_Fill
						)

						.Padding(
							10.0f,
							10.0f,
							10.0f,
							100.0f
						)

						[
							SNew(SScaleBox)

								.Stretch(
									EStretch::ScaleToFit
								)

								[
									SNew(SImage)

										.Image(
											PortraitBrush
										)
								]
						]


					// ============================================
					// CARD - BOTTOM NAME PLATE
					// ============================================

					+SOverlay::Slot()

						.VAlign(
							VAlign_Bottom
						)

						[
							SNew(SBorder)

								.Padding(
									FMargin(
										22.0f,
										12.0f
									)
								)

								.BorderBackgroundColor(
									FLinearColor(
										0.0f,
										0.0f,
										0.0f,
										0.88f
									)
								)

								[
									SNew(SVerticalBox)


										// ------------------------------------
										// PLAYER NUMBER
										// ------------------------------------

										+SVerticalBox::Slot()

										.AutoHeight()

										[
											SNew(STextBlock)

												.Text(
													FText::FromString(
														PlayerLabel
													)
												)

												.ColorAndOpacity(
													AccentColor
												)

												.Font(
													FCoreStyle::GetDefaultFontStyle(
														"Bold",
														15
													)
												)
										]


									// ------------------------------------
									// CHARACTER NAME
									// ------------------------------------

									+SVerticalBox::Slot()

										.AutoHeight()

										.Padding(
											0.0f,
											2.0f,
											0.0f,
											0.0f
										)

										[
											SNew(STextBlock)

												.Text(
													FText::FromString(
														CharacterName
													)
												)

												.ColorAndOpacity(
													FLinearColor::White
												)

												.Font(
													FCoreStyle::GetDefaultFontStyle(
														"Bold",
														36
													)
												)
										]
								]
						]
				];
		};


	// ========================================================
	// 11 - CREATE PLAYERS LAYOUT
	//
	// Le layout change automatiquement selon le nombre
	// de joueurs.
	// ========================================================

	TSharedRef<SOverlay> PlayersOverlay =
		SNew(SOverlay);


	// ========================================================
	// 11A - ONE PLAYER
	//
	// Fighter reveal au centre.
	// ========================================================

	if (PlayerCount == 1)
	{
		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Center
			)

			.VAlign(
				VAlign_Center
			)

			.Padding(
				0.0f,
				110.0f,
				0.0f,
				120.0f
			)

			[
				CreatePlayerCard(
					0,
					650.0f,
					700.0f
				)
			];
	}


	// ========================================================
	// 11B - TWO PLAYERS
	//
	// Layout classique P1 VS P2.
	// ========================================================

	else if (PlayerCount == 2)
	{
		// ----------------------------------------------------
		// PLAYER 1
		// ----------------------------------------------------

		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Left
			)

			.VAlign(
				VAlign_Center
			)

			.Padding(
				45.0f,
				100.0f,
				0.0f,
				110.0f
			)

			[
				CreatePlayerCard(
					0,
					650.0f,
					690.0f
				)
			];


		// ----------------------------------------------------
		// PLAYER 2
		// ----------------------------------------------------

		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Right
			)

			.VAlign(
				VAlign_Center
			)

			.Padding(
				0.0f,
				100.0f,
				45.0f,
				110.0f
			)

			[
				CreatePlayerCard(
					1,
					650.0f,
					690.0f
				)
			];


		// ----------------------------------------------------
		// CENTER ENERGY SLASH
		// ----------------------------------------------------

		if (CenterSlashBrush.IsValid())
		{
			PlayersOverlay->AddSlot()

				.HAlign(
					HAlign_Center
				)

				.VAlign(
					VAlign_Center
				)

				[
					SNew(SBox)

						.WidthOverride(
							350.0f
						)

						.HeightOverride(
							730.0f
						)

						[
							SNew(SImage)

								.Image(
									CenterSlashBrush.Get()
								)
						]
				];
		}


		// ----------------------------------------------------
		// LARGE BACKGROUND VS
		// ----------------------------------------------------

		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Center
			)

			.VAlign(
				VAlign_Center
			)

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
							0.10f
						)
					)

					.Font(
						FCoreStyle::GetDefaultFontStyle(
							"Bold",
							240
						)
					)
			];


		// ----------------------------------------------------
		// MAIN VS
		// ----------------------------------------------------

		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Center
			)

			.VAlign(
				VAlign_Center
			)

			[
				SNew(SBorder)

					.Padding(
						FMargin(
							30.0f,
							14.0f
						)
					)

					.BorderBackgroundColor(
						FLinearColor(
							0.0f,
							0.0f,
							0.0f,
							0.75f
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
									95
								)
							)
					]
			];
	}


	// ========================================================
	// 11C - THREE PLAYERS
	//
	// Trois grandes cartes côte à côte.
	// ========================================================

	else if (PlayerCount == 3)
	{
		TSharedRef<SHorizontalBox> ThreePlayerBox =
			SNew(SHorizontalBox);


		for (int32 Index = 0; Index < 3; Index++)
		{
			ThreePlayerBox->AddSlot()

				.FillWidth(
					1.0f
				)

				.HAlign(
					HAlign_Center
				)

				.VAlign(
					VAlign_Center
				)

				.Padding(
					18.0f
				)

				[
					CreatePlayerCard(
						Index,
						500.0f,
						650.0f
					)
				];
		}


		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Fill
			)

			.VAlign(
				VAlign_Center
			)

			.Padding(
				35.0f,
				120.0f,
				35.0f,
				120.0f
			)

			[
				ThreePlayerBox
			];
	}


	// ========================================================
	// 11D - FOUR PLAYERS
	//
	// Grille AAA 2 x 2.
	// ========================================================

	else if (PlayerCount >= 4)
	{
		TSharedRef<SUniformGridPanel> FourPlayerGrid =
			SNew(SUniformGridPanel)

			.SlotPadding(
				FMargin(
					15.0f
				)
			);


		// ----------------------------------------------------
		// PLAYER 1
		// ----------------------------------------------------

		FourPlayerGrid->AddSlot(
			0,
			0
		)

			[
				CreatePlayerCard(
					0,
					610.0f,
					350.0f
				)
			];


		// ----------------------------------------------------
		// PLAYER 2
		// ----------------------------------------------------

		FourPlayerGrid->AddSlot(
			1,
			0
		)

			[
				CreatePlayerCard(
					1,
					610.0f,
					350.0f
				)
			];


		// ----------------------------------------------------
		// PLAYER 3
		// ----------------------------------------------------

		FourPlayerGrid->AddSlot(
			0,
			1
		)

			[
				CreatePlayerCard(
					2,
					610.0f,
					350.0f
				)
			];


		// ----------------------------------------------------
		// PLAYER 4
		// ----------------------------------------------------

		FourPlayerGrid->AddSlot(
			1,
			1
		)

			[
				CreatePlayerCard(
					3,
					610.0f,
					350.0f
				)
			];


		PlayersOverlay->AddSlot()

			.HAlign(
				HAlign_Center
			)

			.VAlign(
				VAlign_Center
			)

			.Padding(
				0.0f,
				115.0f,
				0.0f,
				125.0f
			)

			[
				FourPlayerGrid
			];
	}


	// ========================================================
	// 12 - FULL AAA LOADING SCREEN
	// ========================================================

	LoadingScreen.WidgetLoadingScreen =

		SNew(SOverlay)


		// ====================================================
		// BACKGROUND IMAGE
		// ====================================================

		+SOverlay::Slot()
		[
			SNew(SImage)

				.Image(
					LoadingBackgroundBrush.Get()
				)
		]


	// ====================================================
	// CINEMATIC DARK FILTER
	// ====================================================

	+SOverlay::Slot()
		[
			SNew(SColorBlock)

				.Color(
					FLinearColor(
						0.0f,
						0.0f,
						0.0f,
						0.52f
					)
				)
		]


	// ====================================================
	// TOP CINEMATIC BAR
	// ====================================================

	+SOverlay::Slot()

		.VAlign(
			VAlign_Top
		)

		[
			SNew(SBox)

				.HeightOverride(
					105.0f
				)

				[
					SNew(SColorBlock)

						.Color(
							FLinearColor(
								0.005f,
								0.005f,
								0.008f,
								0.93f
							)
						)
				]
		]


	// ====================================================
	// BOTTOM CINEMATIC BAR
	// ====================================================

	+SOverlay::Slot()

		.VAlign(
			VAlign_Bottom
		)

		[
			SNew(SBox)

				.HeightOverride(
					100.0f
				)

				[
					SNew(SColorBlock)

						.Color(
							FLinearColor(
								0.005f,
								0.005f,
								0.008f,
								0.96f
							)
						)
				]
		]


	// ====================================================
	// TOP STAGE INFORMATION
	// ====================================================

	+SOverlay::Slot()

		.HAlign(
			HAlign_Center
		)

		.VAlign(
			VAlign_Top
		)

		.Padding(
			0.0f,
			18.0f,
			0.0f,
			0.0f
		)

		[
			SNew(SVerticalBox)


				// ------------------------------------------------
				// NEXT BATTLE
				// ------------------------------------------------

				+SVerticalBox::Slot()

				.AutoHeight()

				.HAlign(
					HAlign_Center
				)

				[
					SNew(STextBlock)

						.Text(
							FText::FromString(
								TEXT("NEXT BATTLE")
							)
						)

						.ColorAndOpacity(
							FLinearColor(
								0.60f,
								0.60f,
								0.65f,
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


			// ------------------------------------------------
			// STAGE NAME
			// ------------------------------------------------

			+SVerticalBox::Slot()

				.AutoHeight()

				.HAlign(
					HAlign_Center
				)

				.Padding(
					0.0f,
					2.0f,
					0.0f,
					0.0f
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
								33
							)
						)
				]
		]


	// ====================================================
	// PLAYER AREA
	// ====================================================

	+SOverlay::Slot()
		[
			PlayersOverlay
		]


		// ====================================================
		// BOTTOM LOADING INFORMATION
		// ====================================================

		+SOverlay::Slot()

		.HAlign(
			HAlign_Center
		)

		.VAlign(
			VAlign_Bottom
		)

		.Padding(
			0.0f,
			0.0f,
			0.0f,
			25.0f
		)

		[
			SNew(SVerticalBox)


				// ------------------------------------------------
				// PREPARING BATTLE TEXT
				// ------------------------------------------------

				+SVerticalBox::Slot()

				.AutoHeight()

				.HAlign(
					HAlign_Center
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
								0.72f,
								0.72f,
								0.75f,
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


			// ------------------------------------------------
			// LOADING BAR
			// ------------------------------------------------

			+SVerticalBox::Slot()

				.AutoHeight()

				.HAlign(
					HAlign_Center
				)

				.Padding(
					0.0f,
					7.0f,
					0.0f,
					0.0f
				)

				[
					SNew(SBox)

						.WidthOverride(
							480.0f
						)

						.HeightOverride(
							6.0f
						)

						[
							SNew(SProgressBar)

								.Percent(
									TOptional<float>()
								)
						]
				]
		];


	// ========================================================
	// 13 - SETUP MOVIE PLAYER
	// ========================================================

	GetMoviePlayer()
		->SetupLoadingScreen(
			LoadingScreen
		);


	// ========================================================
	// 14 - START LOADING SCREEN
	// ========================================================

	GetMoviePlayer()
		->PlayMovie();


	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"Loading Screen started - %d player(s)"
		),
		PlayerCount
	);
}


// ============================================================
// 15 - END LOADING SCREEN
//
// La fermeture est automatique grâce à
// bAutoCompleteWhenLoadingCompletes.
//
// Cette fonction sert principalement au debug.
// ============================================================

void USmashGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
	// ========================================================
	// LOADING SCREEN - MAP FINISHED
	// ========================================================

	if (!LoadedWorld)
	{
		return;
	}

	const FString LoadedMapName = LoadedWorld->GetName();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Map finished loading: %s"),
		*LoadedMapName
	);


	// ========================================================
	// MENU MAP - FORCE RELEASE MOVIE PLAYER
	//
	// Si on revient au menu de sélection,
	// aucun loading screen ne doit garder le contrôle.
	// ========================================================

	if (!LoadedMapName.Contains(TEXT("Final_Arena")))
	{
		if (GetMoviePlayer() && GetMoviePlayer()->IsMovieCurrentlyPlaying())
		{
			GetMoviePlayer()->StopMovie();
		}
	}
}