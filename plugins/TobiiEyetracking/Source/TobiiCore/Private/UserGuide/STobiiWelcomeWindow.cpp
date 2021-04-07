/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#if WITH_EDITOR

#include "STobiiWelcomeWindow.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "CoreGlobals.h"
#include "UnrealClient.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "STobiiWelcomeWindow"

void STobiiWelcomeWindow::Construct(const FArguments& InArgs)
{
	if (GEditor == nullptr)
	{
		return;
	}

	bDontShowAgain = false;

	SWindow::Construct(SWindow::FArguments()
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.IsPopupWindow(true)
		.CreateTitleBar(false)
		.SizingRule(ESizingRule::FixedSize)
		.SupportsTransparency(EWindowTransparency::None)
		.InitialOpacity(1.0f)
		.FocusWhenFirstShown(true)
		.bDragAnywhere(false)
		.ActivationPolicy(EWindowActivationPolicy::FirstShown)
		.ClientSize(FVector2D(600, 400))
		.ScreenPosition(FVector2D((float)(GEditor->GetActiveViewport()->GetSizeXY().X) / 2.0,
		(float)(GEditor->GetActiveViewport()->GetSizeXY().Y) / 2.0))
		[
			SNew(SGridPanel)
			.FillColumn(0, 1.0f)
			.FillRow(0, 1.0f)
			
			+ SGridPanel::Slot(0, 0)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Black)))
			]

			+ SGridPanel::Slot(0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(10.0f)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Margin(5.0f)
					.Font(FEditorStyle::GetFontStyle(FName("ToggleButton.LabelFont")))
					.Text(FText::FromString("Tobii Sample Content"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Margin(5.0f)
					.Text(FText::FromString("Thank you again for downloading the UE4 Tobii eyetracking plugin!.\nThis plugin comes loaded with sample content as well as the core functionality to enable your development environment to leverage the eyetracker hardware.\nThe sample content is packaged can be accessed via the content browser under the 'TobiiEyetracking' plugin section. As plugin content is not visible by default however, you need to first click the small eye button at the bottom right of your content browser and check the 'show plugin content' checkbox. After that you should be able to scroll your content browser down to see the category."))
				]
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(15.0f)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked(bDontShowAgain ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bDontShowAgain = State == ECheckBoxState::Checked; })
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Don't show this window again."))
					]
				]				

				+ SVerticalBox::Slot()
				.Padding(0.0f, 50.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Would you like to navigate to the samples now?"))
				]				

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.OnClicked(this, &STobiiWelcomeWindow::ShowDesktopSamples)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OkButton", "Yes, please show me the desktop samples"))
						.Justification(ETextJustify::Center)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.OnClicked(this, &STobiiWelcomeWindow::ShowXRSamples)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OkButtonXR", "Yes, please show me the XR samples"))
						.Justification(ETextJustify::Center)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.OnClicked(this, &STobiiWelcomeWindow::AbortTutorial)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ExitButton", "No thank you."))
						.Justification(ETextJustify::Center)
					]
				]
			]
		]);
	
	bIsTopmostWindow = true;
	FlashWindow();
}

void STobiiWelcomeWindow::UpdateFlag()
{
	if (GConfig != nullptr && bDontShowAgain)
	{
		GConfig->SetBool(TEXT("Tobii"), TEXT("TobiiTutorialShown"), true, GGameIni);
	}
}

FReply STobiiWelcomeWindow::ShowDesktopSamples()
{
	RequestDestroyWindow();
	UpdateFlag();

	//Actually turn on plugin content viewing and navigate the content browser to the sample
	IContentBrowserSingleton& ContentBrowserInterface = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get();
	ContentBrowserInterface.ForceShowPluginContent(false);

	TArray<FAssetData> MapAsset;
	MapAsset.Add(FAssetData(FName("/TobiiEyetracking/Sample/Desktop/DesktopExampleMap")
						  , FName("/TobiiEyetracking/Sample/Desktop")
						  , FName("DesktopExampleMap")
						  , FName("World")));
	ContentBrowserInterface.SyncBrowserToAssets(MapAsset);

	return FReply::Handled();
}

FReply STobiiWelcomeWindow::ShowXRSamples()
{
	RequestDestroyWindow();
	UpdateFlag();

	//Actually turn on plugin content viewing and navigate the content browser to the sample
	IContentBrowserSingleton& ContentBrowserInterface = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get();
	ContentBrowserInterface.ForceShowPluginContent(false);

	TArray<FAssetData> MapAsset;
	MapAsset.Add(FAssetData(FName("/TobiiEyetracking/Sample/XR/XRExampleMap")
		, FName("/TobiiEyetracking/Sample/XR")
		, FName("XRExampleMap")
		, FName("World")));
	ContentBrowserInterface.SyncBrowserToAssets(MapAsset);

	return FReply::Handled();
}

FReply STobiiWelcomeWindow::AbortTutorial()
{
	RequestDestroyWindow();
	UpdateFlag();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

#endif //WITH_EDITOR
