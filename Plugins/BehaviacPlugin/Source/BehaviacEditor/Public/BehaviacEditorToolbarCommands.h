// BehaviacEditorToolbarCommands — UI_COMMAND for the BT editor toolbar button.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BehaviacEditorStyle.h"

class FBehaviacEditorToolbarCommands : public TCommands<FBehaviacEditorToolbarCommands>
{
public:
	FBehaviacEditorToolbarCommands()
		: TCommands<FBehaviacEditorToolbarCommands>(
			  TEXT("BehaviacEditor"),
			  NSLOCTEXT("Contexts", "BehaviacEditor", "Behaviac Editor"),
			  NAME_None,
			  FBehaviacEditorStyle::GetStyleSetName())
	{
	}

	void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenBTEditor;
};
