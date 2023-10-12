# mhw-action-cloner
A plugin/framework to create new monster actions for Monster Hunter World.

**Warning:** This tool does not aim to be an easy-to-use way of copy-pasting actions from one monster to another. The target audience is people who add new monsters to the game or modify existing ones (on a more extensive level).
To properly make use of this you will **have to** do some reverse engineering to look at how existing actions work, or how to make use of the base action. See [Animations](#animations) for more details.

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
***Note:** The syntax used in these examples requires C++20.*

You register your custom actions in the `initialize_actions` function. You can register as many actions as you want.

In this function you tell the action cloner:
- Which monster the action is for
- Which action you want to use as a base
- The name of the action
- The functions you want to use for the action

```cpp
#include "mh/mh.h"

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

### Multiple Monsters per dll
Normally one `CustomActionList` can only hold actions for one monster. However you can provide custom actions for multiple monsters from a single dll, through the use of the `get_action_list_count` function.

```cpp
#include "mh/mh.h"

ACTION_EXPORT unsigned int get_action_list_count() {
    return 3;
}

ACTION_EXPORT void initialize_actions(CustomActionList* lists) {
    // Option 1
    lists[0] = CustomActionList{
        .Monster = Monster::RuinerNergigante,
        .Variant = 5,
        .Actions = { /* ... */ }
    };

    // Option 2
    lists[1].Monster = Monster::Teostra;
    lists[1].Variant = 0;

    lists[1].Actions.push_back({ /* ... */ });

    lists[2].Monster = Monster::FuriousRajang;
    lists[2].Variant = 5;

    // ...
}
```
This code tells the action cloner that the dll wants to provide custom actions for 3 different monsters.

### Dynamically adding new actions
Through the use of the functions detailed above you can only add custom actions at startup. However there is one more function that lets you add custom actions at any point during execution.
By exporting the `initialize_action_api` you can get access to a function that lets you register custom actions at runtime.

```cpp
#include "mh/mh.h"

static AddNewActionListFunc g_add_new_action_list = nullptr;

ACTION_EXPORT void initialize_action_api(AddNewActionListFunc add_new_action_list) {
    g_add_new_action_list = add_new_action_list;
}

// Somewhere
void some_function() {
    // ActionCloner copies the list so there is no need to heap allocate it.
    CustomActionList list{
        .Monster = ...;
        .Variant = ...;
        .Actions = { ... };
    };
    
    g_add_new_action_list(&list);
}
```

### Animations
Unfortunately actions do not have a given way of when and how animations are executed. Each action does is slightly different.

For example, if you take a look at the [RuinerNergigante](https://github.com/Fexty12573/mhw-action-cloner/blob/027b66ebaf04da986ec6b9af3b571e991d3bd88b/RuinerNergigante/CustomActions.cpp#L16) example in the repo, you can see this code:
```cpp
void special_arm_l::on_execute(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func) {
    action->get<u32>(0x1B0) = Action::make_animation_id(1, 23);
    did_spikes = false;
    parent_func(action);
}
```

Here an animation id [1, 23] is assigned to `0x1B0` inside the action. For this particular action, this is where the animation Id is stored. By calling the `parent_func` inside the `on_initialize`, this animation is executed instead of the one that it normally would. However as said before, this doesn't apply to all actions. Actions that make use of multiple animations for example usually have an array of all the animation ids stored somewhere inside the action.
