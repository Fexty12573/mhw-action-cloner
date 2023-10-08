#include "pch.h"
#include <Windows.h>

#include "CustomActions.h"

ACTION_EXPORT void initialize_actions(CustomActionList* list) {
    list->Monster = Monster::RuinerNergigante;
    list->Variant = 5;

    CustomAction a = {
        .ActionName = "CUSTOM_SPECIAL_ARM_L",
        .Flags = 1,
        .BaseActionId = 180,
        .OnInitialize = special_arm_l::on_initialize,
        .OnExecute = special_arm_l::on_execute,
        .OnUpdate = special_arm_l::on_update,
        .OnEnd = special_arm_l::on_end
    };

    list->Actions.push_back(a);

    list->Actions.push_back({
        .ActionName = "GENERIC_1",
        .Flags = 1,
        .BaseActionId = 113,
        .OnInitialize = generic::on_initialize,
        .OnExecute = generic::on_execute,
        .OnUpdate = generic::on_update,
        .OnEnd = generic::on_end
    });
    list->Actions.push_back({
        .ActionName = "GENERIC_2",
        .Flags = 1,
        .BaseActionId = 113,
        .OnInitialize = generic::on_initialize,
        .OnExecute = generic::on_execute,
        .OnUpdate = generic::on_update,
        .OnEnd = generic::on_end
    });
}

BOOL APIENTRY DllMain(HMODULE hDll, DWORD dwReason, LPVOID lpReserved) {
	return TRUE;
}
