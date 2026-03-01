// BehaviacEditorToolbarCommands.cpp

#include "BehaviacEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FBehaviacEditorModule"

void FBehaviacEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenBTEditor,
		"BT Editor",
		"Open the Behaviac Behavior Tree web editor",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
