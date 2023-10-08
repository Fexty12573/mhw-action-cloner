#pragma once

#include <memory>

#include "Monster.h"

#define ACTION_EXPORT extern "C" __declspec(dllexport)

struct CustomActionList;
struct Action;

using TActionFunc = void(void*);

using TOnInitFunc = TActionFunc;
using TOnExecuteFunc = TActionFunc;
using TOnUpdateFunc = bool(void*);
using TOnEndFunc = TActionFunc;

using OnInitFunc = TOnInitFunc*;
using OnExecuteFunc = TOnExecuteFunc*;
using OnUpdateFunc = TOnUpdateFunc*;
using OnEndFunc = TOnEndFunc*;

using TOnInitCallback = void(Action* action, OnInitFunc parent_func, OnInitFunc base_func);
using TOnExecuteCallback = void(Action* action, OnExecuteFunc parent_func, OnExecuteFunc base_func);
using TOnUpdateCallback = bool(Action* action, OnUpdateFunc parent_func, OnUpdateFunc base_func);
using TOnEndCallback = void(Action* action, OnEndFunc parent_func, OnEndFunc base_func);

using OnInitCallback = TOnInitCallback*;
using OnExecuteCallback = TOnExecuteCallback*;
using OnUpdateCallback = TOnUpdateCallback*;
using OnEndCallback = TOnEndCallback*;

using TAddNewActionListFunc = void(CustomActionList* list);

using AddNewActionListFunc = TAddNewActionListFunc*;

struct CustomAction {
    // This can be anything and doesn't have any impact on the behavior of the action
    std::string ActionName;

    // Flags for the action, leave as -1 to use the flags of the base action
    s32 Flags = -1;

    // The Id of the action to use as a base for the custom action. If you set this
    // leave BaseDti as nullptr
    s32 BaseActionId = -1;

    // The DTI of the action to use as a base for the custom action.
    // Use this if the action you want to use as a base is from a different monster.
    // If you set this BaseActionId will be ignored
    MtDTI<void>* BaseDti = nullptr;


    // The OnInitialize function will be called after the action object is created
    OnInitCallback OnInitialize = nullptr;

    // The OnExecute function will be called each time the action is executed
    OnExecuteCallback OnExecute = nullptr;

    // The OnUpdate function will be called once per frame while the action is active
    OnUpdateCallback OnUpdate = nullptr;

    // The OnEnd function will be called when the action ends
    OnEndCallback OnEnd = nullptr;
};

struct CustomActionList {
    // The id and subspecies id of the monster to add the custom actions to
    Monster::Id Monster;
    u32 Variant;

    std::vector<CustomAction> Actions;
};

enum class DesireCategory : u32 {
    Hunger,
    Sunbath,
    Sleep,
    Territory,
    Thirst,
    Escape
};

struct Action {
    enum VtableIndex : size_t;

    void** vft;

    Monster* parent() const { return get<Monster*>(0x30); }

    const char* name() const { return get<const char*>(0x20); }

    bool& disable_combat_transition() const { return get<bool>(0xB8); }
    s32& push_group() const { return get<s32>(0xBC); }
    u32& desire_points(DesireCategory category) const { return get<u32>(0x140 + (u32)category * 4); }

    u32& footprint_type() const { return get<u32>(0x15C); }
    float& footprint_dist_min() const { return get<float>(0x160); }
    float& footprint_dist_max() const { return get<float>(0x164); }

    u32& moved_trace_type() const { return get<u32>(0x168); }
    float& moved_trace_dist_min() const { return get<float>(0x16C); }
    float& moved_trace_dist_max() const { return get<float>(0x170); }

    bool& disable_friend_hit() const { return get<bool>(0x1A0); }

    static u32 make_animation_id(u32 lmt, u32 anim) {
        return (lmt << 12) | anim & 0xFFF;
    }

    template<typename T> T& get(size_t offset) const { return *(T*)((u64)this + offset); }

    enum VtableIndex : size_t {
        Destructor,
        CreateUi,
        IsEnableInstance,
        CreateProperty,
        GetDti,
        OnInitialize,
        OnExecute,
        OnUpdate,
        OnEnd
    };
};
