// BehaviacEditorModule.cpp — toolbar button to open the Behaviac BT web editor.

#include "BehaviacEditorModule.h"
#include "BehaviacEditorStyle.h"
#include "BehaviacEditorToolbarCommands.h"
#include "ToolMenus.h"
#include "HAL/PlatformProcess.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FBehaviacEditorModule"

// URL of the Behaviac behavior tree web editor.
// Change this if you're running it locally on a different port.
static const TCHAR* BehaviacEditorURL = TEXT("http://localhost:8080");

void FBehaviacEditorModule::StartupModule()
{
	FBehaviacEditorStyle::Initialize();
	FBehaviacEditorStyle::ReloadTextures();

	FBehaviacEditorToolbarCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FBehaviacEditorToolbarCommands::Get().OpenBTEditor,
		FExecuteAction::CreateRaw(this, &FBehaviacEditorModule::OnOpenBTEditor),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBehaviacEditorModule::RegisterMenus));
}

void FBehaviacEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FBehaviacEditorToolbarCommands::Unregister();
	FBehaviacEditorStyle::Shutdown();
}

void FBehaviacEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Try the UE5-style User toolbar slot first; fall back to PlayToolBar for older layouts
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	if (!ToolbarMenu)
		ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");

	if (ToolbarMenu)
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("BehaviacEditor");
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			FBehaviacEditorToolbarCommands::Get().OpenBTEditor,
			TAttribute<FText>(),
			TAttribute<FText>(),
			FSlateIcon(FBehaviacEditorStyle::GetStyleSetName(), "BehaviacEditor.OpenBTEditor")));
		return;
	}

	// Legacy extender fallback (UE4-style modules)
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule =
			FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension(
			"Settings",
			EExtensionHook::After,
			PluginCommands,
			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
			{
				Builder.AddToolBarButton(FBehaviacEditorToolbarCommands::Get().OpenBTEditor);
			}));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FBehaviacEditorModule::OnOpenBTEditor()
{
	FPlatformProcess::LaunchURL(BehaviacEditorURL, nullptr, nullptr);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBehaviacEditorModule, BehaviacEditor)
