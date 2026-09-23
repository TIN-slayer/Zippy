# Project instructions

## Project

This is an Unreal Engine 5 project using C++ and Blueprints.

The Unreal Editor may already be running and can be accessed through the configured Unreal MCP server.

## Unreal Editor

- Never launch UnrealEditor.exe or open the .uproject unless explicitly requested.
- Never start a second Unreal Editor instance unless explicitly requested.
- Assume Unreal Editor is already running when Unreal MCP tools are available.
- Prefer Unreal MCP tools for inspecting or modifying Blueprints, Actors, Components, Levels, and assets.
- Never modify .uasset or .umap files as raw binary files.

## C++

- Follow Unreal Engine C++ conventions.
- Comment you code.
- Respect Unreal reflection requirements for UCLASS, USTRUCT, UENUM, UPROPERTY, and UFUNCTION.
- Never manually edit generated headers.
- Prefer forward declarations when appropriate.
- Keep header dependencies minimal.
- Maintain correct module dependencies in Build.cs files.
- Preserve Blueprint-facing APIs unless the requested change requires modifying them.

## Generated files

Do not manually edit:

- Binaries/
- DerivedDataCache/
- Intermediate/
- Saved/

unless explicitly required for diagnostics.

## Workflow

Before making significant changes:

1. Inspect the existing implementation.
2. Determine whether the change belongs in C++, Blueprint, or both.
3. Reuse existing project architecture where practical.
4. Make the smallest coherent change.
5. Compile or otherwise validate the change.
6. If Unreal MCP is available, verify the resulting state in Unreal Editor.

## Safety

- Do not delete assets, Blueprints, classes, levels, or configuration without explicit approval.
- Do not use destructive Git commands such as `git reset --hard` unless explicitly requested.
- Do not overwrite or discard user changes you did not create.

## MCP

- When Unreal MCP tools are available, use them instead of launching Unreal Editor processes.
- If an Unreal MCP operation fails, report the failure before attempting to launch or restart Unreal Editor.