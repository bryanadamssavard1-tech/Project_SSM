#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SmashGameInstance.generated.h"

class UTexture2D;
class SBox;
struct FSlateBrush;

UCLASS()
class COMBO_TEST_API USmashGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Loading Screen")
	UTexture2D* Player1Portrait = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Loading Screen")
	UTexture2D* Player2Portrait = nullptr;
	TSharedPtr<FSlateBrush> Player1PortraitBrush;
	TSharedPtr<FSlateBrush> Player2PortraitBrush;
	// ============================================================
	// GAME INSTANCE
	// ============================================================

	virtual void Init() override;


private:

	// ============================================================
	// LOADING SCREEN FUNCTIONS
	// ============================================================

	void BeginLoadingScreen(const FString& MapName);

	void EndLoadingScreen(UWorld* LoadedWorld);



	


	// ============================================================
	// LOADING SCREEN WIDGETS
	// ============================================================


	TSharedPtr<FSlateBrush> LoadingBackgroundBrush;


	// ============================================================
	// LOADING SCREEN BACKGROUND
	// ============================================================

	UPROPERTY()
	UTexture2D* LoadingBackgroundTexture = nullptr;


};
