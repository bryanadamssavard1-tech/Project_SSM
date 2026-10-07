#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SmashGameInstance.generated.h"


// ============================================================
// FORWARD DECLARATIONS
// ============================================================

class UTexture2D;
class SBox;
struct FSlateBrush;


// ============================================================
// LOADING PLAYER DATA
//
// Contient toutes les informations nécessaires
// pour afficher un joueur dans le loading screen.
// ============================================================

USTRUCT(BlueprintType)
struct FLoadingPlayerData
{
	GENERATED_BODY()


	// --------------------------------------------------------
	// PLAYER INDEX
	//
	// 0 = Player 1
	// 1 = Player 2
	// 2 = Player 3
	// 3 = Player 4
	// --------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	int32 PlayerIndex = 0;


	// --------------------------------------------------------
	// CHARACTER NAME
	//
	// Exemple :
	// Bryan
	// Riktor
	// Dummy
	// --------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	FString CharacterName;


	// --------------------------------------------------------
	// CHARACTER PORTRAIT
	//
	// Texture utilisée dans le loading screen.
	// --------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	UTexture2D* Portrait = nullptr;
};


// ============================================================
// SMASH GAME INSTANCE
// ============================================================

UCLASS()
class COMBO_TEST_API USmashGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:

	// ========================================================
	// 01 - INITIALIZATION
	// ========================================================

	virtual void Init() override;


	// ========================================================
	// 02 - LOADING PLAYERS DATA
	//
	// Contient entre 1 et 4 joueurs.
	// Ces données survivent au OpenLevel car elles sont
	// stockées dans le GameInstance.
	// ========================================================

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	TArray<FLoadingPlayerData> LoadingPlayers;


	// ========================================================
	// 03 - LOADING SCREEN BACKGROUND
	//
	// Background utilisé actuellement pour le loading screen.
	// Plus tard, on pourra utiliser une image différente
	// selon le stage.
	// ========================================================

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	UTexture2D* LoadingBackgroundTexture = nullptr;


	// ========================================================
	// 04 - CENTER SLASH
	//
	// Texture utilisée derrière le VS.
	// ========================================================

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen")
	UTexture2D* CenterSlashTexture = nullptr;


private:

	// ========================================================
	// 05 - LOADING SCREEN FUNCTIONS
	// ========================================================

	// Appelée juste avant le chargement d'une map.
	void BeginLoadingScreen(const FString& MapName);

	// Appelée lorsque la map est chargée.
	void EndLoadingScreen(UWorld* LoadedWorld);


	// ========================================================
	// 06 - BACKGROUND BRUSH
	//
	// Slate utilise un FSlateBrush pour afficher une texture.
	// ========================================================

	TSharedPtr<FSlateBrush> LoadingBackgroundBrush;


	// ========================================================
	// 07 - CENTER SLASH BRUSH
	// ========================================================

	TSharedPtr<FSlateBrush> CenterSlashBrush;


	// ========================================================
	// 08 - PLAYER PORTRAIT BRUSHES
	//
	// Maximum de 4 portraits.
	// Chaque élément correspond à LoadingPlayers[Index].
	// ========================================================

	TArray<TSharedPtr<FSlateBrush>> PlayerPortraitBrushes;
};