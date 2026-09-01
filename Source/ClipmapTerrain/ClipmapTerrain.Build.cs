// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ClipmapTerrain : ModuleRules
{
	public ClipmapTerrain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","MeshDescription","StaticMeshDescription", "MeshConversion", "Chaos" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd", "DerivedDataCache" });
        }
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(
                            Path.Combine(ModuleDirectory, "ThirdParty", "FastNoise2", "FastNoise.lib"));
            PublicAdditionalLibraries.Add(
                            Path.Combine(ModuleDirectory, "ThirdParty", "FastNoise2", "FastSIMD.lib"));

            PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "ThirdParty", "FastNoise2", "include") });

            PublicDefinitions.Add("FASTNOISE_STATIC_LIB");
        }
    }
}
