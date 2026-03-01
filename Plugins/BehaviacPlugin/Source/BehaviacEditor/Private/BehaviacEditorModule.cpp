// BehaviacEditorModule.cpp — toolbar button to open the Behaviac BT web editor.

#include "BehaviacEditorModule.h"
#include "BehaviacEditorStyle.h"
#include "BehaviacEditorToolbarCommands.h"
#include "ToolMenus.h"
#include "HAL/PlatformProcess.h"
#include "LevelEditor.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FBehaviacEditorModule"

// Path resolved at runtime relative to the plugin's own directory.
static FString GetBTEditorPath()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("BehaviacPlugin");
	if (!Plugin.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[BehaviacEditor] BehaviacPlugin not found via IPluginManager!"));
		return FString();
	}
	return Plugin->GetBaseDir() / TEXT("Editor/index.html");
}

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
	FString EditorPath = GetBTEditorPath();
	if (EditorPath.IsEmpty()) return;

	UE_LOG(LogTemp, Log, TEXT("[BehaviacEditor] Opening BT editor: %s"), *EditorPath);

#if PLATFORM_MAC
	FPlatformProcess::ExecProcess(TEXT("/usr/bin/open"), *EditorPath, nullptr, nullptr, nullptr);
#elif PLATFORM_WINDOWS
	FPlatformProcess::LaunchURL(*FString::Printf(TEXT("file:///%s"), *EditorPath), nullptr, nullptr);
#else
	FPlatformProcess::LaunchURL(*EditorPath, nullptr, nullptr);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBehaviacEditorModule, BehaviacEditor)
