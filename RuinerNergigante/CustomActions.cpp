#include "pch.h"
#include "CustomActions.h"

#include <loader.h>

using namespace loader;

namespace special_arm_l {
static bool did_spikes = false;
}

void special_arm_l::on_initialize(Action* action, OnInitFunc parent_func, OnInitFunc base_func) {
    parent_func(action);
}

void special_arm_l::on_execute(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func) {
    action->get<u32>(0x1B0) = Action::make_animation_id(1, 23);
    did_spikes = false;
    parent_func(action);
}

bool special_arm_l::on_update(Action* action, OnUpdateFunc parent_func, OnUpdateFunc base_func) {
    if (!did_spikes) {
        if (action->parent()->animation_frame() > 20.0f) {
            LOG(INFO) << "Spawned Effect 2007:0";
            action->parent()->spawn_effect(2007, 0);
            did_spikes = true;
        }
    }

    return parent_func(action);
}

void special_arm_l::on_end(Action* action, OnEndFunc parent_func, OnEndFunc base_func) {
    parent_func(action);
}

void generic::on_initialize(Action* action, OnInitFunc parent_func, OnInitFunc base_func) {
    LOG(INFO) << "OnInit: " << action->name();
    parent_func(action);
}

void generic::on_execute(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func) {
    LOG(INFO) << "OnExec: " << action->name();
    parent_func(action);
}

bool generic::on_update(Action* action, OnUpdateFunc parent_func, OnUpdateFunc base_func) {
    return parent_func(action);
}

void generic::on_end(Action* action, OnEndFunc parent_func, OnEndFunc base_func) {
    LOG(INFO) << "OnEnd: " << action->name();
    parent_func(action);
}
