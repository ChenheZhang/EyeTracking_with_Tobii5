/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#if WITH_EDITOR

#include "STobiiLicenseWindow.h"
#include "STobiiWelcomeWindow.h"

#include "CoreGlobals.h"
#include "Editor.h"
#include "UnrealClient.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "TobiiLicenseWindow"

void STobiiLicenseWindow::Construct(const FArguments& InArgs, FTobiiCoreModule* InCoreModule)
{
	if (GEditor == nullptr)
	{
		return;
	}

	CoreModule = InCoreModule;

#if TOBII_COMPILE_AS_ENGINE_PLUGIN
	FString LicenseFilePath = FPaths::EnginePluginsDir() / TEXT("Runtime/TobiiEyetracking/Resources/License.txt");
#else
	FString LicenseFilePath = FPaths::ProjectPluginsDir() / TEXT("TobiiEyetracking/Resources/License.txt");
#endif

	FString LicenseText;
	bool bCanAccept = true;
	if (!FFileHelper::LoadFileToString(LicenseText, *LicenseFilePath))
	{
		LicenseText = "License file not found in Resources folder!\n\nAccept button disabled.\nIf you cannot repair/replace the Tobii SDK, please remove it from your project.";
		bCanAccept = false;
	}

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
		.ClientSize(FVector2D(1024, 768))
		.ScreenPosition(FVector2D((float)(GEditor->GetActiveViewport()->GetSizeXY().X) / 2.0,
		(float)(GEditor->GetActiveViewport()->GetSizeXY().Y) / 2.0))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(0.9f)
			.Padding(5)
			[
				SNew(SScrollBox)
				+SScrollBox::Slot()
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
						SNew(STextBlock)
						.AutoWrapText(true)
						.Margin(10.0f)
						.Text(FText::FromString(LicenseText))
					]
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.05f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				.IsEnabled(bCanAccept)
				.OnClicked(this, &STobiiLicenseWindow::AcceptLicense)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OkButton", "I Accept the terms of this agreement"))
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(0.05f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				.OnClicked(this, &STobiiLicenseWindow::ShutdownEditor)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ExitButton", "I do not accept the terms. Shut down editor so I can remove the plugin."))
					.Justification(ETextJustify::Center)
				]
			]
		]);

	bIsTopmostWindow = true;
	FlashWindow();
}

FReply STobiiLicenseWindow::AcceptLicense()
{
	RequestDestroyWindow();

	if (GConfig != nullptr)
	{
		GConfig->SetBool(TEXT("Tobii"), TEXT("TobiiLicenseAccepted"), true, GGameIni);
	}

	bool bTutorialShown = false;
	GConfig->GetBool(TEXT("Tobii"), TEXT("TobiiTutorialShown"), bTutorialShown, GGameIni);
	if (!bTutorialShown && GEditor != nullptr)
	{
		FSlateApplication::Get().AddWindow(SNew(STobiiWelcomeWindow));
	}

	return FReply::Handled();
}

FReply STobiiLicenseWindow::ShutdownEditor()
{
	RequestDestroyWindow();
	FGenericPlatformMisc::RequestExit(false);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

#endif //WITH_EDITOR
