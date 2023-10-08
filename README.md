# mhw-action-cloner
A plugin/framework to create new monster actions for Monster Hunter World.

## Installation
### For Users
If you want to install this to use custom actions other people have made, follow these steps:
1. Install Stracker's Loader if you haven't already. You can find it [here](https://www.nexusmods.com/monsterhunterworld/mods/1982).
2. Download the latest release from the [releases page](https://github.com/Fexty12573/mhw-action-cloner/releases).
3. Grab the `mhw-action-cloner-RELEASE.dll` file from the zip file and place it in your `Monster Hunter World\nativePC\plugins` folder.
4. Install any actions you want to use.


### For Developers
If you want to create your own custom actions, follow these steps:
1. Install Stracker's Loader if you haven't already. You can find it [here](https://www.nexusmods.com/monsterhunterworld/mods/1982).
2. Download the latest release from the [releases page](https://github.com/Fexty12573/mhw-action-cloner/releases).

You can either use the Visual Studio project template or manually set up your project.
The project template already has the header files and `loader.lib` (For logging to Stracker's Loader console) linked, so you don't need to do anything else.

#### Visual Studio Project Template
3. Put the project template in your Visual Studio templates folder. This is usually `C:\Users\{username}\Documents\Visual Studio {version}\Templates\ProjectTemplates`.
4. Open VS2022, create a new project and select the template.
5. Fill in the details and click create.

#### Manual Setup
3. Create a new C++ dll project in Visual Studio.
4. Add the header files from the zip to your project.
5. Create a new .cpp file and add the following code:

```cpp
#include "mh.h"

ACTION_EXPORT void initialize_actions(CustomActionList* list) {
    
}
```

### How do actions work?
Each action has 4 functions which are called at different points in the action's lifecycle:

| Function | Description |
| --- | --- |
| `OnInitialize` | Called when the action is first created (at the same time the monster is created). This is where you should set up any variables you need. |
| `OnExecute` | Called when the action is executed. This is usually where the first LMT call happens |
| `OnUpdate` | Called every frame. This is where you should put any logic that needs to happen every frame. |
| `OnEnd` | Called when the action is ended. This is where you should perform any necessary clean up. |

You need to provide each one of these functions in your action. If you don't need to do anything in a function, you can just leave it empty.

### How do I create an action?
You register your custom actions in the `initialize_actions` function. You can register as many actions as you want.

In this function you tell the action cloner:
- Which monster the action is for
- Which action you want to use as a base
- The name of the action
- The functions you want to use for the action

```cpp
#include "mh.h"

ACTION_EXPORT void initialize_actions(CustomActionList* list) {
    list->Monster = Monster::RuinerNergigante;
    list->Variant = 5;

    list->Actions.push_back({
        .ActionName = "MY_CUSTOM_ACTION",
        .Flags = 0,
        .BaseActionId = 180,
        .OnInitialize = &MyCustomAction::OnInitialize,
        .OnExecute = &MyCustomAction::OnExecute,
        .OnUpdate = &MyCustomAction::OnUpdate,
        .OnEnd = &MyCustomAction::OnEnd
    });
}
```
This code registers a new action by the name `MY_CUSTOM_ACTION`. The ID is the next higher unused ID.

Check the RuinerNergigante project for a full example.
