#include "SmashGameInstance.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "MoviePlayer.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Engine/LocalPlayer.h"
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
	// ========================================================
// LOCAL MULTIPLAYER - INPUT DEVICE CONNECTION EVENT
//
// Écoute les connexions et déconnexions des manettes.
// ========================================================

	IPlatformInputDeviceMapper::Get()
		.GetOnInputDeviceConnectionChange()
		.AddUObject(
			this,
			&USmashGameInstance::HandleInputDeviceConnectionChange
		);

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
// manette est connectée ou déconnectée.
//
// Pour l'instant, on affiche seulement les IDs pour vérifier
// exactement ce qu'Unreal détecte.
// ============================================================
void USmashGameInstance::HandleInputDeviceConnectionChange(
	EInputDeviceConnectionState NewConnectionState,
	FPlatformUserId PlatformUserId,
	FInputDeviceId InputDeviceId
)
{
	// ============================================================
	// DEBUG - PÉRIPHÉRIQUE DÉTECTÉ
	// ============================================================

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("INPUT DEVICE EVENT | Device=%d | PlatformUser=%d | State=%d"),
		InputDeviceId.GetId(),
		PlatformUserId.GetInternalId(),
		static_cast<int32>(NewConnectionState)
	);

	// ============================================================
	// DÉCONNEXION
	// ============================================================

	if (NewConnectionState == EInputDeviceConnectionState::Disconnected)
	{
		for (FSmashLocalPlayerData& PlayerData : ConnectedPlayers)
		{
			if (PlayerData.InputDeviceId == InputDeviceId)
			{
				PlayerData.bIsConnected = false;

				UE_LOG(
					LogTemp,
					Warning,
					TEXT("MANETTE DECONNECTEE | Device=%d -> PlayerIndex=%d"),
					InputDeviceId.GetId(),
					PlayerData.PlayerIndex
				);

				break;
			}
		}

		return;
	}

	// ============================================================
	// CONNEXION / RECONNEXION
	// ============================================================

	if (NewConnectionState != EInputDeviceConnectionState::Connected)
	{
		return;
	}

	// ------------------------------------------------------------
	// Cette manette est déjà connue : on reconnecte le même joueur.
	// ------------------------------------------------------------

	for (FSmashLocalPlayerData& PlayerData : ConnectedPlayers)
	{
		if (PlayerData.InputDeviceId == InputDeviceId)
		{
			PlayerData.bIsConnected = true;

			// Réapplique le routage physique -> LocalPlayer.
			RouteInputDeviceToLocalPlayer(
				PlayerData.PlayerIndex,
				InputDeviceId
			);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("MANETTE RECONNECTEE | Device=%d -> PlayerIndex=%d"),
				InputDeviceId.GetId(),
				PlayerData.PlayerIndex
			);

			return;
		}
	}

	// ============================================================
	// NOUVELLE MANETTE
	//
	// P1 = clavier.
	// P2 = index 1
	// P3 = index 2
	// P4 = index 3
	// ============================================================

	const int32 NewPlayerIndex = ConnectedPlayers.Num() + 1;

	if (NewPlayerIndex > 3)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MANETTE IGNOREE | Maximum 4 joueurs | Device=%d"),
			InputDeviceId.GetId()
		);

		return;
	}

	FSmashLocalPlayerData NewPlayer;
	NewPlayer.PlayerIndex = NewPlayerIndex;
	NewPlayer.InputDeviceId = InputDeviceId;
	NewPlayer.bIsConnected = true;

	// IMPORTANT : on enregistre le Device AVANT de créer le LocalPlayer.
	ConnectedPlayers.Add(NewPlayer);

	CreateSmashLocalPlayer(
		NewPlayerIndex,
		InputDeviceId
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("MANETTE ENREGISTREE | Device=%d -> PlayerIndex=%d"),
		InputDeviceId.GetId(),
		NewPlayerIndex
	);
}


// ============================================================
// LOCAL MULTIPLAYER - CREATE LOCAL PLAYER
//
// Crée P2/P3/P4 avec leur ControllerId normal, puis associe
// la manette PHYSIQUE au PlatformUser du LocalPlayer créé.
//
// Contrairement à SetPlatformUserId(), on ne modifie pas le
// LocalPlayer. On déplace uniquement la manette vers le bon
// PlatformUser.
// ============================================================

void USmashGameInstance::CreateSmashLocalPlayer(
	int32 PlayerIndex,
	FInputDeviceId InputDeviceId
)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("CREATE LOCAL PLAYER REQUEST | PlayerIndex=%d | Device=%d"),
		PlayerIndex,
		InputDeviceId.GetId()
	);

	// ============================================================
	// SÉCURITÉ - SI LE LOCAL PLAYER EXISTE DÉJÀ
	// ============================================================

	if (ULocalPlayer* ExistingLocalPlayer = GetLocalPlayerByIndex(PlayerIndex))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("LOCAL PLAYER DEJA EXISTANT | PlayerIndex=%d | ControllerId=%d | PlatformUser=%d"),
			PlayerIndex,
			ExistingLocalPlayer->GetControllerId(),
			ExistingLocalPlayer->GetPlatformUserId().GetInternalId()
		);

		RouteInputDeviceToLocalPlayer(
			PlayerIndex,
			InputDeviceId
		);

		return;
	}

	// ============================================================
	// DEBUG - P1
	// ============================================================

	if (ULocalPlayer* Player1 = GetLocalPlayerByIndex(0))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("P1 INFO | ControllerId=%d | PlatformUser=%d"),
			Player1->GetControllerId(),
			Player1->GetPlatformUserId().GetInternalId()
		);
	}

	// ============================================================
	// DEBUG - PROPRIÉTAIRE ACTUEL DE LA MANETTE
	// ============================================================

	IPlatformInputDeviceMapper& DeviceMapper =
		IPlatformInputDeviceMapper::Get();

	const FPlatformUserId DevicePlatformUser =
		DeviceMapper.GetUserForInputDevice(InputDeviceId);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("DEVICE BEFORE CREATE | Device=%d -> PlatformUser=%d"),
		InputDeviceId.GetId(),
		DevicePlatformUser.GetInternalId()
	);

	// ============================================================
	// CRÉER LE LOCAL PLAYER PAR INDEX
	//
	// On conserve la version stable qui fonctionnait déjà :
	// P2 = ControllerId 1
	// P3 = ControllerId 2
	// P4 = ControllerId 3
	// ============================================================

	FString Error;

	ULocalPlayer* NewLocalPlayer = CreateLocalPlayer(
		PlayerIndex,
		Error,
		true
	);

	if (!NewLocalPlayer)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ECHEC LOCAL PLAYER | PlayerIndex=%d | Error=%s"),
			PlayerIndex,
			*Error
		);

		return;
	}

	// ============================================================
	// IMPORTANT - NE PAS FAIRE SetPlatformUserId()
	//
	// Le LocalPlayer garde le PlatformUser créé normalement par UE.
	// On va plutôt associer la vraie manette à ce PlatformUser.
	// ============================================================

	RouteInputDeviceToLocalPlayer(
		PlayerIndex,
		InputDeviceId
	);

	// ============================================================
	// DEBUG - LOCAL PLAYER FINAL
	// ============================================================

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("LOCAL PLAYER CREE | PlayerIndex=%d | Device=%d | ControllerId=%d | PlatformUser=%d"),
		PlayerIndex,
		InputDeviceId.GetId(),
		NewLocalPlayer->GetControllerId(),
		NewLocalPlayer->GetPlatformUserId().GetInternalId()
	);

	// ============================================================
	// DEBUG - TOUS LES LOCAL PLAYERS
	// ============================================================

	for (int32 Index = 0; Index < GetNumLocalPlayers(); ++Index)
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayerByIndex(Index))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("PLAYER STATE | Index=%d | ControllerId=%d | PlatformUser=%d"),
				Index,
				LocalPlayer->GetControllerId(),
				LocalPlayer->GetPlatformUserId().GetInternalId()
			);
		}
	}

	// ============================================================
	// PRÉVENIR LE BLUEPRINT
	// ============================================================

	OnSmashLocalPlayerCreated(PlayerIndex);
}


// ============================================================
// LOCAL MULTIPLAYER - ROUTE PHYSICAL INPUT DEVICE
//
// C'est la correction importante pour :
//
// P1 clavier
// P2 première manette
// P3 deuxième manette
// P4 troisième manette
//
// La première manette arrive normalement sur PlatformUser 0,
// comme le clavier. On la déplace vers le PlatformUser de P2.
// P3/P4 sont également vérifiés et corrigés si nécessaire.
//
// IMPORTANT :
// Dans Project Settings, "Skip Assigning Gamepad to Player 1"
// doit être DÉCOCHÉ avec cette architecture. Sinon le
// GameViewportClient applique encore son propre +1/remapping.
// ============================================================

void USmashGameInstance::RouteInputDeviceToLocalPlayer(
	int32 PlayerIndex,
	FInputDeviceId InputDeviceId
)
{
	ULocalPlayer* TargetLocalPlayer =
		GetLocalPlayerByIndex(PlayerIndex);

	if (!TargetLocalPlayer)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ROUTING FAILED | LocalPlayer invalide | PlayerIndex=%d | Device=%d"),
			PlayerIndex,
			InputDeviceId.GetId()
		);

		return;
	}

	IPlatformInputDeviceMapper& DeviceMapper =
		IPlatformInputDeviceMapper::Get();

	const FPlatformUserId CurrentUser =
		DeviceMapper.GetUserForInputDevice(InputDeviceId);

	const FPlatformUserId TargetUser =
		TargetLocalPlayer->GetPlatformUserId();

	// Déjà parfaitement routé.
	if (CurrentUser == TargetUser)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ROUTING OK | Device=%d -> PlayerIndex=%d | PlatformUser=%d"),
			InputDeviceId.GetId(),
			PlayerIndex,
			TargetUser.GetInternalId()
		);

		return;
	}

	// ============================================================
	// CHANGER LE PROPRIÉTAIRE DE LA MANETTE
	//
	// Cette fonction déclenche un PairingChange et non un nouvel
	// événement de connexion, donc elle ne doit pas créer P3/P4
	// accidentellement dans HandleInputDeviceConnectionChange().
	// ============================================================

	const bool bChanged =
		DeviceMapper.Internal_ChangeInputDeviceUserMapping(
			InputDeviceId,
			TargetUser,
			CurrentUser
		);

	if (bChanged)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ROUTING CHANGED | Device=%d | User %d -> %d | PlayerIndex=%d"),
			InputDeviceId.GetId(),
			CurrentUser.GetInternalId(),
			TargetUser.GetInternalId(),
			PlayerIndex
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ROUTING CHANGE FAILED | Device=%d | User %d -> %d | PlayerIndex=%d"),
			InputDeviceId.GetId(),
			CurrentUser.GetInternalId(),
			TargetUser.GetInternalId(),
			PlayerIndex
		);
	}
}


















