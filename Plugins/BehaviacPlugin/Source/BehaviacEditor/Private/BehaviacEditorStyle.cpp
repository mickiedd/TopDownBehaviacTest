// BehaviacEditorStyle.cpp

#include "BehaviacEditorStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FBehaviacEditorStyle::StyleInstance = nullptr;

void FBehaviacEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBehaviacEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBehaviacEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BehaviacEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

const FVector2D Icon40x40(40.f, 40.f);

TSharedRef<FSlateStyleSet> FBehaviacEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("BehaviacEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("BehaviacPlugin")->GetBaseDir() / TEXT("Resources"));
	Style->Set("BehaviacEditor.OpenBTEditor", new IMAGE_BRUSH(TEXT("BehaviacEditorIcon_40x"), Icon40x40));
	return Style;
}

#undef IMAGE_BRUSH

void FBehaviacEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FBehaviacEditorStyle::Get()
{
	return *StyleInstance;
}
