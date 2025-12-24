<div align="center">

  [![SDK (Windows)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-sdk-windows.yml/badge.svg)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-sdk-windows.yml)
  
  [![Half-Life 2 Deathmatch (Windows)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-hl2mp-windows.yml/badge.svg)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-hl2mp-windows.yml)
  [![Team Fortress 2 (Windows)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-tf-windows.yml/badge.svg)](https://github.com/Generalisk/SourceSDK-Generalisk/actions/workflows/build-tf-windows.yml)

  ###
  
  [![Commit Activity](https://img.shields.io/github/commit-activity/w/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
  [![Commit Activity](https://img.shields.io/github/commit-activity/m/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
  [![Commit Activity](https://img.shields.io/github/commit-activity/y/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
  [![Commit Activity](https://img.shields.io/github/commit-activity/t/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
  
  <!--[![License](https://img.shields.io/github/license/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk/blob/main/LICENSE)-->
  [![Issues](https://img.shields.io/github/issues/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk/issues)
  [![File Size](https://img.shields.io/github/repo-size/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
  [![Last Commit](https://img.shields.io/github/last-commit/Generalisk/SourceSDK-Generalisk)](https://github.com/Generalisk/SourceSDK-Generalisk)
</div>

<div align="center">
  
  # Source SDK
</div>

This is my fork of Valve Software's [2013 Source SDK](https://github.com/ValveSoftware/source-sdk-2013), which I use as a base for my own source engine projects.

This fork contains bug fixes, QoL improvements & even some new features (some of these I may/may not have stolen from the official Source SDK's pull request page :P).

## CONTENTS
- [Featured Games](#featured-games)
- [Requirements](#requirements)
- [Build Instructions](#build-instructions)
- [License](#license)
- [Credits](#credits)
- [Additional Resources](#additional-resources)

## FEATURED GAMES
- [Half-Life 2](https://store.steampowered.com/app/220/HalfLife_2/)
- [Half-Life 2: Deathmatch](https://store.steampowered.com/app/320/HalfLife_2_Deathmatch/)
- [Team Fortress 2](https://store.steampowered.com/app/440/Team_Fortress_2/)

## REQUIREMENTS
- [Source SDK 2013 Base Multiplayer](https://steamdb.info/app/243750/) via [Steam](https://store.steampowered.com/about/)
### Windows
- [Microsoft Visual Studio 2022](https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2022-and-other-products) with the following workflows & components:
  - `Desktop development with C++` workflow
  - `MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)`
  - `Windows 11 SDK (10.0.22621.0)` or `Windows 10 SDK (10.0.19041.1)`
- [Python](https://www.python.org/downloads/) 3.13 or later
### Linux
- [Podman](https://podman.io/)

## BUILD INSTRUCTIONS
### Windows
- Inside the `src` directory, run:
  ```
  createallprojects.bat
  ```
  This will regenerate the Visual Studio Project Files with any changes you have made to the VPC scripts and will generate new project files if they don't exist or couldn't be found.
- Open the Solution (`everything.sln`) in Visual Studio.
- Make sure that the Platform Configuration is set to `Release` instead of `Debug`.
- In the menu, go to `Build -> Build Solution` or press `Ctrl + Shift + B`.
#### Debugging
- To start debugging your source engine project, press `F5` or click the `Local Windows Debugger` button with the green arrow to start debugging inside of Visual Studio.
  - *Make sure that the client project is selected as the startup project otherwise the debugger will not work correctly*
### Linux
- Inside the `src` directory, run:
  ```
  ./buildallprojects
  ```
  This will compile your project automatically against the Steam Runtime.
> [!IMPORTANT]
> Mods that are distributed on Steam MUST be built against the Steam Runtime, which the above steps will automatically do for you.

## LICENSE
The Source SDK and, by extension, the projects that you create are, by default, licensed under Valve's `SOURCE 1 SDK LICENSE`, which is contained in the [LICENSE](LICENSE) file at the root of the repository.

For information & guidance on distributing your mod on or off Steam, check the Source Engine [distribution guidelines & FAQ](https://partner.steamgames.com/doc/sdk/uploading/distributing_source_engine).

## CREDITS
- [Source Engine](https://github.com/ValveSoftware/source-sdk-2013) by [Valve Software](https://www.valvesoftware.com/)

## ADDITIONAL RESOURCES
- [Valve Developer Wiki](https://developer.valvesoftware.com/wiki/Setting_up_Source_SDK_Base_2013_Multiplayer)
- [Valve Developer Community Discord Server](https://discord.gg/AC8254CJax)
