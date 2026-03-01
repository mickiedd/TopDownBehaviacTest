// BehaviacEditorModule.h

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBehaviacEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void OnOpenBTEditor();

	TSharedPtr<class FUICommandList> PluginCommands;
};
