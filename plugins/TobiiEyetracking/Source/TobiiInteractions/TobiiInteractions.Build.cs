/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

using System;
using System.IO;
using Tools.DotNETCommon;

namespace UnrealBuildTool.Rules
{
    public class TobiiInteractions : ModuleRules
    {
        public TobiiInteractions(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Core"
                , "CoreUObject"
                , "Engine"
                , "Slate"
                , "SlateCore"
                , "UMG"
                , "HeadMountedDisplay"
            });

            PublicDependencyModuleNames.AddRange(new string[]
            {
                "EyeTracker"
                , "TobiiGTOM"
            });

            PrivateIncludePaths.AddRange(new string[]
            {
                "TobiiInteractions/Public"
                , "TobiiInteractions/Public/Common"
                , "TobiiInteractions/Public/Desktop"
            });

            string AssemblyLocation = Path.GetDirectoryName(new Uri(System.Reflection.Assembly.GetExecutingAssembly().CodeBase).LocalPath);
            DirectoryReference RootDirectory = new DirectoryReference(Path.Combine(AssemblyLocation, "..", "..", ".."));
            bool IsEnginePlugin = RootDirectory.GetDirectoryName() == "Engine";

            PublicDefinitions.Add("TOBII_COMPILE_AS_ENGINE_PLUGIN=" + (IsEnginePlugin ? 1 : 0));
            
            //Platform specific
            if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32)
            {
                string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
                string PluginsPath = Path.Combine(ModuleDirectory, "../../../");
                string TobiiRelativeAPIPath = "TobiiEyetracking/ThirdParty/GameIntegration";
                string TobiiRelativeIncludePath = Path.Combine(TobiiRelativeAPIPath, "include");

                //Includes
                PrivateIncludePaths.Add(Path.Combine(PluginsPath, TobiiRelativeIncludePath));
            }
        }
    }
}
