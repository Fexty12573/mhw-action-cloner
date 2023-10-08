#pragma once

#include "Action.h"

namespace special_arm_l {

void on_initialize(Action* action, OnInitFunc parent_func, OnInitFunc base_func);
void on_execute(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func);
bool on_update(Action* action, OnUpdateFunc parent_func, OnUpdateFunc base_func);
void on_end(Action* action, OnEndFunc parent_func, OnEndFunc base_func);

}

namespace generic {

void on_initialize(Action* action, OnInitFunc parent_func, OnInitFunc base_func);
void on_execute(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func);
bool on_update(Action* action, OnUpdateFunc parent_func, OnUpdateFunc base_func);
void on_end(Action* action, OnEndFunc parent_func, OnEndFunc base_func);

}
