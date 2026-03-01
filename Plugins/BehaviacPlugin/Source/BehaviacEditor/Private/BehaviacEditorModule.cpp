// BehaviacEditorModule.cpp — toolbar button to open the Behaviac BT web editor.

#include "BehaviacEditorModule.h"
#include "BehaviacEditorStyle.h"
#include "BehaviacEditorToolbarCommands.h"
#include "ToolMenus.h"
#include "HAL/PlatformProcess.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FBehaviacEditorModule"

// Path to the BehaviorU static web editor.
// We open it as a file:// URL — no server needed.
static const TCHAR* BehaviacEditorPath = TEXT("/Volumes/M2/Works/BehaviorU/Editor/index.html");

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
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			FBehaviacEditorToolbarCommands::Get().OpenBTEditor,
			TAttribute<FText>(),
			TAttribute<FText>(),
			FSlateIcon(FBehaviacEditorStyle::GetStyleSetName(), "BehaviacEditor.OpenBTEditor")));
		Entry.SetCommandList(PluginCommands);
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
	UE_LOG(LogTemp, Log, TEXT("[BehaviacEditor] Opening BT editor: %s"), BehaviacEditorPath);

#if PLATFORM_MAC
	// Use /usr/bin/open with the file path — most reliable on macOS
	FPlatformProcess::ExecProcess(TEXT("/usr/bin/open"), BehaviacEditorPath, nullptr, nullptr, nullptr);
#elif PLATFORM_WINDOWS
	FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file:///%s"), BehaviacEditorPath), nullptr, nullptr);
#else
	FPlatformProcess::LaunchURL(BehaviacEditorPath, nullptr, nullptr);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBehaviacEditorModule, BehaviacEditor)
