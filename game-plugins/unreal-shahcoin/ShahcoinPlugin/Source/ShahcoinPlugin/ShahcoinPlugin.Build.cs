using UnrealBuildTool;

public class ShahcoinPlugin : ModuleRules
{
	public ShahcoinPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add other public include paths required here ...
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"HTTP",
				"Json",
				"JsonUtilities",
				"WebSockets",
				"Networking",
				"Sockets"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"HTTP",
				"Json",
				"JsonUtilities",
				"WebSockets"
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
		
		// Enable SSL for secure API calls
		bEnableUndefinedIdentifierWarnings = false;
		
		// Platform-specific settings
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			// macOS-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Linux-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android-specific settings
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			// iOS-specific settings
		}
	}
}
