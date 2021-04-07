/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiInteractionsStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FTobiiInteractionsStyle::TobiiEyetrackingInteractionsStyleInstance = nullptr;

void FTobiiInteractionsStyle::Initialize()
{
	if (!TobiiEyetrackingInteractionsStyleInstance.IsValid())
	{
		TobiiEyetrackingInteractionsStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*TobiiEyetrackingInteractionsStyleInstance);
	}
}

void FTobiiInteractionsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*TobiiEyetrackingInteractionsStyleInstance);
	ensure(TobiiEyetrackingInteractionsStyleInstance.IsUnique());
	TobiiEyetrackingInteractionsStyleInstance.Reset();
}

FName FTobiiInteractionsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("TobiiStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef<FSlateStyleSet> FTobiiInteractionsStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("TobiiStyle"));

#if TOBII_COMPILE_AS_ENGINE_PLUGIN
	Style->SetContentRoot(FPaths::EnginePluginsDir() / TEXT("Runtime/TobiiEyetracking/Resources"));
#else
	Style->SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("TobiiEyetracking/Resources"));
#endif

	//The names here after ClassIcon. must match the type names of the types we want to offer the icon for.
	Style->Set("ClassIcon.TobiiCleanUIContainer", new IMAGE_BRUSH(TEXT("CleanUI16x"), Icon16x16));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FTobiiInteractionsStyle::ReloadTextures()
{
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FTobiiInteractionsStyle::Get()
{
	return *TobiiEyetrackingInteractionsStyleInstance;
}
