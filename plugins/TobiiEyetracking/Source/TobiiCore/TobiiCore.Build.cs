/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

using System;
using System.IO;
using System.Diagnostics;
using Tools.DotNETCommon;

namespace UnrealBuildTool.Rules
{
    public class TobiiCore : ModuleRules
    {
        //If you want to remove eyetracking functionality from your builds without modifying your game code / project you can do so easily by setting this to false.
        private bool IsEyetrackingActive { get { return true; } }
        
        public TobiiCore(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Core"
                , "CoreUObject"
                , "Engine"
                , "UMG"
                , "InputCore"
                , "InputDevice"					
                , "HeadMountedDisplay"
			});

            PublicDependencyModuleNames.AddRange(new string[]
            {
                "EyeTracker"
            });

            if (Target.bBuildEditor)
            {
                DynamicallyLoadedModuleNames.AddRange(new string[] { "LevelEditor" });
                PublicDependencyModuleNames.AddRange(new string[]
                {
                    "Slate"
                    , "SlateCore"
                    , "EditorStyle"
                    , "UnrealEd"
                    , "MainFrame"
                    , "GameProjectGeneration"
                    , "WebBrowser"
                    , "RHI"
                });
            }

            string AssemblyLocation = Path.GetDirectoryName(new Uri(System.Reflection.Assembly.GetExecutingAssembly().CodeBase).LocalPath);
            DirectoryReference RootDirectory = new DirectoryReference(Path.Combine(AssemblyLocation, "..", "..", ".."));
            bool IsEnginePlugin = RootDirectory.GetDirectoryName() == "Engine";
            PublicDefinitions.Add("TOBII_COMPILE_AS_ENGINE_PLUGIN=" + (IsEnginePlugin ? 1 : 0));

            //Platform specific
            if (IsEyetrackingActive && (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32))
            {
                string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
                string PluginsPath = Path.Combine(ModuleDirectory, "../../../");
                string TobiiRelativeAPIPath = "TobiiEyetracking/ThirdParty/GameIntegration";
                string TobiiRelativeIncludePath = Path.Combine(TobiiRelativeAPIPath, "include");
                string TobiiRelativeLibraryBasePath = Path.Combine(TobiiRelativeAPIPath, "lib");

                //Includes
                PrivateIncludePaths.Add(Path.Combine(PluginsPath, TobiiRelativeIncludePath));

                //Add libraries
                AddLibrary(Path.Combine(PluginsPath, TobiiRelativeLibraryBasePath, PlatformString, (Target.Platform == UnrealTargetPlatform.Win32) ? "tobii_gameintegration_x86.lib" : "tobii_gameintegration_x64.lib"));

                //Add DLL
                string RelativeGICDllPath = "";
                string GICDllName = (Target.Platform == UnrealTargetPlatform.Win32) ? "tobii_gameintegration_x86.dll" : "tobii_gameintegration_x64.dll";

                if (IsEnginePlugin)
                {
                    RelativeGICDllPath = Path.Combine("Binaries/ThirdParty/TobiiEyetracking", PlatformString, GICDllName);
                    RuntimeDependencies.Add("$(EngineDir)/" + RelativeGICDllPath);
                }
                else
                {
                    RelativeGICDllPath = Path.Combine(TobiiRelativeLibraryBasePath, PlatformString, GICDllName);
                    RuntimeDependencies.Add(Path.Combine(PluginsPath, RelativeGICDllPath));
                }

                PublicDefinitions.Add("TOBII_EYETRACKING_ACTIVE=1");
                PublicDefinitions.Add("TOBII_GIC_RELATIVE_DLL_PATH=R\"(" + RelativeGICDllPath + ")\"");
                PublicDelayLoadDLLs.Add(GICDllName);
            }
            else
            {
                PublicDefinitions.Add("TOBII_EYETRACKING_ACTIVE=0");
            }
        }

        private void AddLibrary(string libraryPath)
        {
            if (File.Exists(libraryPath))
            {
                PublicAdditionalLibraries.Add(libraryPath);
            }
            else
            {
                Debug.WriteLine("Cannot find Tobii API Lib file. Path does not exist! " + libraryPath);
            }
        }
    }
}
