// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Styling/SlateColor.h"

namespace NightmareMainMenuWidgetPrivate
{
	static UTextBlock* MakeMenuLabel(UWidgetTree* Tree, const FName& Name, const FString& Label, int32 FontSize, const FLinearColor& Color)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Text->SetText(FText::FromString(Label));
		Text->SetColorAndOpacity(FSlateColor(Color));
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Text->SetFont(Font);
		return Text;
	}

	static UButton* MakeMenuButton(UWidgetTree* Tree, const FName& Name, const FString& Label, bool bEnabled, int32 FontSize)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		Button->SetIsEnabled(bEnabled);
		UTextBlock* LabelText = MakeMenuLabel(
			Tree,
			*FString::Printf(TEXT("%s_Label"), *Name.ToString()),
			Label,
			FontSize,
			bEnabled ? FLinearColor::White : FLinearColor(0.35f, 0.35f, 0.35f, 1.0f));
		Button->AddChild(LabelText);
		return Button;
	}

	static void AnchorCenter(UCanvasPanelSlot* Slot, const FVector2D& Size, float CenterYFraction)
	{
		Slot->SetAnchors(FAnchors(0.5f, CenterYFraction, 0.5f, CenterYFraction));
		Slot->SetAlignment(FVector2D(0.5f, 0.5f));
		Slot->SetSize(Size);
		Slot->SetPosition(FVector2D(0.0f, 0.0f));
	}
}

TSharedRef<SWidget> UNightmareMainMenuWidget::RebuildWidget()
{
	using namespace NightmareMainMenuWidgetPrivate;

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	Background->SetBrushColor(FLinearColor::Black);
	if (UCanvasPanelSlot* BgSlot = RootCanvas->AddChildToCanvas(Background))
	{
		BgSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BgSlot->SetOffsets(FMargin(0.0f));
	}

	TitleText = MakeMenuLabel(WidgetTree, TEXT("TitleText"), TEXT("NIGHTMARE"), 72, FLinearColor::White);
	if (UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleText))
	{
		AnchorCenter(TitleSlot, FVector2D(800.0f, 90.0f), 0.22f);
	}

	StartButton = MakeMenuButton(WidgetTree, TEXT("StartButton"), TEXT("開始遊戲"), true, 28);
	StartButton->OnClicked.AddDynamic(this, &UNightmareMainMenuWidget::HandleStartClicked);
	if (UCanvasPanelSlot* StartSlot = RootCanvas->AddChildToCanvas(StartButton))
	{
		AnchorCenter(StartSlot, FVector2D(420.0f, 56.0f), 0.48f);
	}

	MultiplayerButton = MakeMenuButton(WidgetTree, TEXT("MultiplayerButton"), TEXT("多人連線"), false, 28);
	if (UCanvasPanelSlot* MultiSlot = RootCanvas->AddChildToCanvas(MultiplayerButton))
	{
		AnchorCenter(MultiSlot, FVector2D(420.0f, 56.0f), 0.56f);
	}

	QuitButton = MakeMenuButton(WidgetTree, TEXT("QuitButton"), TEXT("結束遊戲"), true, 28);
	QuitButton->OnClicked.AddDynamic(this, &UNightmareMainMenuWidget::HandleQuitClicked);
	if (UCanvasPanelSlot* QuitSlot = RootCanvas->AddChildToCanvas(QuitButton))
	{
		AnchorCenter(QuitSlot, FVector2D(420.0f, 56.0f), 0.64f);
	}

	return Super::RebuildWidget();
}

void UNightmareMainMenuWidget::HandleStartClicked()
{
	OnStartGameClicked.Broadcast();
}

void UNightmareMainMenuWidget::HandleQuitClicked()
{
	OnQuitGameClicked.Broadcast();
}
