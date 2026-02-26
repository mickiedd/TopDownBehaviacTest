// Behaviac UE5 Plugin
// Licensed under the BSD 3-Clause License.

using UnrealBuildTool;

public class BehaviacTests : ModuleRules
{
	public BehaviacTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"BehaviacRuntime",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AutomationController",
		});
	}
}
