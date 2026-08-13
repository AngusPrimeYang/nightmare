// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareFailPopupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace NightmareFailPopupWidgetPrivate
{
	static void AnchorCenter(UCanvasPanelSlot* Slot, const FVector2D& Size, float CenterYFraction)
	{
		Slot->SetAnchors(FAnchors(0.5f, CenterYFraction, 0.5f, CenterYFraction));
		Slot->SetAlignment(FVector2D(0.5f, 0.5f));
		Slot->SetSize(Size);
		Slot->SetPosition(FVector2D(0.0f, 0.0f));
	}
}

TSharedRef<SWidget> UNightmareFailPopupWidget::RebuildWidget()
{
	using namespace NightmareFailPopupWidgetPrivate;

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* DimBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DimBackground"));
	DimBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	if (UCanvasPanelSlot* DimSlot = RootCanvas->AddChildToCanvas(DimBackground))
	{
		DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		DimSlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));
	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel))
	{
		AnchorCenter(PanelSlot, FVector2D(560.0f, 360.0f), 0.5f);
	}

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("體力歸零")));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo TitleFont = TitleText->GetFont();
	TitleFont.Size = 36;
	TitleText->SetFont(TitleFont);
	if (UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleText))
	{
		AnchorCenter(TitleSlot, FVector2D(500.0f, 48.0f), 0.38f);
	}

	CountdownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownText"));
	CountdownText->SetText(FText::FromString(TEXT("10")));
	CountdownText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.0f, 1.0f)));
	FSlateFontInfo CountdownFont = CountdownText->GetFont();
	CountdownFont.Size = 64;
	CountdownText->SetFont(CountdownFont);
	if (UCanvasPanelSlot* CountdownSlot = RootCanvas->AddChildToCanvas(CountdownText))
	{
		AnchorCenter(CountdownSlot, FVector2D(200.0f, 80.0f), 0.48f);
	}

	UTextBlock* CountdownHint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountdownHint"));
	CountdownHint->SetText(FText::FromString(TEXT("秒後自動回到選單")));
	CountdownHint->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
	FSlateFontInfo HintFont = CountdownHint->GetFont();
	HintFont.Size = 20;
	CountdownHint->SetFont(HintFont);
	if (UCanvasPanelSlot* HintSlot = RootCanvas->AddChildToCanvas(CountdownHint))
	{
		AnchorCenter(HintSlot, FVector2D(400.0f, 28.0f), 0.54f);
	}

	ContinueButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ContinueButton"));
	UTextBlock* ContinueLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ContinueLabel"));
	ContinueLabel->SetText(FText::FromString(TEXT("繼續遊戲")));
	ContinueLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo ContinueFont = ContinueLabel->GetFont();
	ContinueFont.Size = 24;
	ContinueLabel->SetFont(ContinueFont);
	ContinueButton->AddChild(ContinueLabel);
	ContinueButton->OnClicked.AddDynamic(this, &UNightmareFailPopupWidget::HandleContinueClicked);
	if (UCanvasPanelSlot* ContinueSlot = RootCanvas->AddChildToCanvas(ContinueButton))
	{
		AnchorCenter(ContinueSlot, FVector2D(220.0f, 52.0f), 0.62f);
	}

	ReturnToMenuButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ReturnToMenuButton"));
	UTextBlock* ReturnLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReturnLabel"));
	ReturnLabel->SetText(FText::FromString(TEXT("回到選單")));
	ReturnLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo ReturnFont = ReturnLabel->GetFont();
	ReturnFont.Size = 24;
	ReturnLabel->SetFont(ReturnFont);
	ReturnToMenuButton->AddChild(ReturnLabel);
	ReturnToMenuButton->OnClicked.AddDynamic(this, &UNightmareFailPopupWidget::HandleReturnToMenuClicked);
	if (UCanvasPanelSlot* ReturnSlot = RootCanvas->AddChildToCanvas(ReturnToMenuButton))
	{
		AnchorCenter(ReturnSlot, FVector2D(220.0f, 52.0f), 0.70f);
	}

	return Super::RebuildWidget();
}

void UNightmareFailPopupWidget::RefreshCountdown(int32 DisplaySeconds)
{
	if (CountdownText)
	{
		CountdownText->SetText(FText::AsNumber(DisplaySeconds));
	}
}

void UNightmareFailPopupWidget::HandleContinueClicked()
{
	OnContinueClicked.Broadcast();
}

void UNightmareFailPopupWidget::HandleReturnToMenuClicked()
{
	OnReturnToMenuClicked.Broadcast();
}
