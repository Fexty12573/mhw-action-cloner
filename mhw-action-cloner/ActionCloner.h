#pragma once

#include <filesystem>
#include <unordered_map>
#include <memory>
#include <vector>

#include <asmjit/asmjit.h>

#include "Action.h"
#include "FunctionHook.h"

struct ActionTable;

struct InternalAction {
    CustomAction* Action = nullptr;
    std::unique_ptr<utility::FunctionHook> OnExecuteHook;
    std::unique_ptr<utility::FunctionHook> OnUpdateHook;
    std::unique_ptr<utility::FunctionHook> OnEndHook;

    OnExecuteFunc OriginalOnExecute = nullptr;
    OnUpdateFunc OriginalOnUpdate = nullptr;
    OnEndFunc OriginalOnEnd = nullptr;

    bool OnExecuteCalled = false;
    bool OnUpdateCalled = false;
    bool OnEndCalled = false;

    ~InternalAction();
};

struct DynamicHook {
    void* Code;
    size_t OriginalOffset;
};

struct InternalActionList {
    std::unordered_map<Action*, InternalAction> InternalActions;
};

class ActionCloner {
public:
    static std::shared_ptr<ActionCloner> get();

    ActionCloner();

    void initialize();
    void load_actions();
    void unload_actions();

    void destroy_dynamic_hook(void* code);

private:
    static void set_action_set_hook(Monster* thisptr, u32 set_id, ActionTable* table, u32 count, s32 ac_idx);
    static void monster_dtor_hook(Monster* thisptr);
    static void text_input_hook(const wchar_t* text);
    static void on_execute_hook(Action* thisptr, InternalAction* action);
    static bool on_update_hook(Action* thisptr, InternalAction* action);
    static void on_end_hook(Action* thisptr, InternalAction* action);

    static void add_new_action_list(CustomActionList* list);

    static void set_action_set_fixup(Monster* m);

    static InternalAction* find_action(Monster* m, Action* a, Action::VtableIndex func_type);
    DynamicHook generate_hook(void* follow_up, Action::VtableIndex func_type);

    void load_actions_from(const std::filesystem::path& dir);
    void load_actions_from(HMODULE dll, const std::filesystem::path& file);

private:
    std::unordered_map<Monster*, InternalActionList> m_active_actions;
    std::vector<CustomActionList> m_custom_action_lists;
    std::vector<HMODULE> m_loaded_action_dlls;
    std::unique_ptr<utility::FunctionHook> m_set_action_set_hook;
    std::unique_ptr<utility::FunctionHook> m_monster_dtor_hook;
    std::unique_ptr<utility::FunctionHook> m_text_input_hook;

    asmjit::JitRuntime m_runtime{};

    static inline const OnInitFunc s_base_on_initialize = (OnInitFunc)0x142f14a58;
    static inline const OnExecuteFunc s_base_on_execute = (OnExecuteFunc)0x142f14a60;
    static inline const OnUpdateFunc s_base_on_update = (OnUpdateFunc)0x142f14a68;
    static inline const OnEndFunc s_base_on_end = (OnEndFunc)0x142f14a70;

    static constexpr inline const char* s_action_dll_path = "./nativePC/plugins/actions";
    static std::shared_ptr<ActionCloner> s_instance;
};

