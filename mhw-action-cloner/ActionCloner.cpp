#include "pch.h"
#include "ActionCloner.h"

#include <dti/MtWeaponInfo.h>
#include <game_functions.h>
#include <ghidra_export.h>
#include <loader.h>
#include <filesystem>
#include <ranges>
#include <dti/cActionController.h>

std::shared_ptr<ActionCloner> ActionCloner::s_instance;

InternalAction::~InternalAction() {
    const auto cloner = ActionCloner::get();
    if (OnExecuteHook) {
        cloner->destroy_dynamic_hook(OnExecuteHook->get_hook());
        OnExecuteHook.reset();
    }

    if (OnUpdateHook) {
        cloner->destroy_dynamic_hook(OnUpdateHook->get_hook());
        OnUpdateHook.reset();
    }

    if (OnEndHook) {
        cloner->destroy_dynamic_hook(OnEndHook->get_hook());
        OnEndHook.reset();
    }
}

std::shared_ptr<ActionCloner> ActionCloner::get() {
    if (!s_instance) {
        s_instance = std::make_shared<ActionCloner>();
    }

    return s_instance;
}

ActionCloner::ActionCloner() {
    m_set_action_set_hook = std::make_unique<utility::FunctionHook>(MH::ActionController::SetActionSet, &set_action_set_hook);
    m_monster_dtor_hook = std::make_unique<utility::FunctionHook>(MH::Monster::dtor, &monster_dtor_hook);
    m_text_input_hook = std::make_unique<utility::FunctionHook>(MH::sMhInputText::Dispatch, &text_input_hook);
    m_set_action_set_hook->enable();
    m_monster_dtor_hook->enable();
    m_text_input_hook->enable();
}

void ActionCloner::initialize() {
    load_actions_from(s_action_dll_path);
}

void ActionCloner::load_actions() {
    initialize();
}

void ActionCloner::unload_actions() {
    using namespace loader;

    m_active_actions.clear();
    m_custom_action_lists.clear();

    for (const auto& dll : m_loaded_action_dlls) {
        FreeLibrary(dll);
    }

    m_loaded_action_dlls.clear();
    LOG(INFO) << "ActionCloner: Unloaded Actions";
}

void ActionCloner::destroy_dynamic_hook(void* code) {
    m_runtime.release(code);
}

void ActionCloner::set_action_set_hook(Monster* thisptr, u32 set_id, ActionTable* table, u32 count, s32 ac_idx) {
    const auto cloner = ActionCloner::get();
    const auto write_void_pointer = [](void* base, u64 offset, void* value) {
        *(void**)((u64)base + offset) = value;
    };
    const auto find_hook = [cloner](const void* target, Action::VtableIndex hook_type) -> void* {
        for (const auto& list : cloner->m_active_actions | std::views::values) {
            for (const auto& action : list.InternalActions | std::views::values) {
                switch (hook_type) {
                case Action::OnExecute:
                    if (action.OnExecuteHook && action.OnExecuteHook->get_target() == target) {
                        return action.OnExecuteHook->get_original();
                    }
                    break;
                case Action::OnUpdate:
                    if (action.OnUpdateHook && action.OnUpdateHook->get_target() == target) {
                        return action.OnUpdateHook->get_original();
                    }
                    break;
                case Action::OnEnd:
                    if (action.OnEndHook && action.OnEndHook->get_target() == target) {
                        return action.OnEndHook->get_original();
                    }
                    break;
                default: break;
                }
            }
        }

        return nullptr;
    };

    if (!thisptr->GetDTI()->InheritsFrom("uEnemy")) {
        cloner->m_set_action_set_hook->call_original<void>(thisptr, set_id, table, count, ac_idx);
        return set_action_set_fixup(thisptr);
    }

    const auto list = std::ranges::find_if(cloner->m_custom_action_lists, [thisptr](const auto& l) {
        using namespace loader;
        LOG(DEBUG) << "Monster: " << thisptr->id() << " Variant: " << thisptr->custom_variant() << " List: " << l.Monster << " Variant: " << l.Variant;
        return l.Monster == thisptr->id() && l.Variant == thisptr->custom_variant();
    });
    
    if (list == cloner->m_custom_action_lists.end() || table != MH::Monster::GetActionTableAt(list->Monster)) {
        cloner->m_set_action_set_hook->call_original<void>(thisptr, set_id, table, count, ac_idx);
        return set_action_set_fixup(thisptr);
    }

    std::vector<ActionTableEntry> new_table;
    new_table.resize((size_t)count + list->Actions.size());
    std::memcpy(new_table.data(), table, sizeof(ActionTableEntry) * count);

    u32 index = count;
    for (const auto& action : list->Actions) {
        if (action.BaseDti) {
            new_table[index].mDTI = action.BaseDti;
            new_table[index].mFlags = action.Flags;
        } else {
            new_table[index].mDTI = new_table[action.BaseActionId].mDTI;
            new_table[index].mFlags = new_table[action.BaseActionId].mFlags;
        }

        new_table[index].mName = action.ActionName.c_str();
        new_table[index].mID = index;

        index++;
    }

    cloner->m_active_actions.emplace(thisptr, InternalActionList{});
    cloner->m_set_action_set_hook->call_original<void>(thisptr, set_id, new_table.data(), (u32)new_table.size(), ac_idx);

    const auto action_controller = &thisptr->get<cActionController>(0x61C8 + ac_idx * cActionController::Size);
    const auto action_list = action_controller->GetActionList(set_id).mList;

    for (auto i = count; i < new_table.size(); i++) {
        const auto custom_action = &list->Actions[i - count];
        const auto action = (Action*)action_list[i];
        const auto vtable = action->vft;

        auto& internal_action = cloner->m_active_actions[thisptr].InternalActions[action];
        internal_action.Action = custom_action;

        void* existing_hook = find_hook(vtable[Action::OnExecute], Action::OnExecute);
        if (!existing_hook) {
            const auto on_execute_dhook = cloner->generate_hook(&on_execute_hook, Action::OnExecute);
            internal_action.OnExecuteHook = std::make_unique<utility::FunctionHook>(vtable[Action::OnExecute], on_execute_dhook.Code);
            write_void_pointer(on_execute_dhook.Code, on_execute_dhook.OriginalOffset, internal_action.OnExecuteHook->get_original());
            internal_action.OnExecuteHook->enable();
            internal_action.OriginalOnExecute = internal_action.OnExecuteHook->get_original<TOnExecuteFunc>();
        } else {
            internal_action.OriginalOnExecute = (OnExecuteFunc)existing_hook;
        }

        existing_hook = find_hook(vtable[Action::OnUpdate], Action::OnUpdate);
        if (!existing_hook) {
            const auto on_update_dhook = cloner->generate_hook(&on_update_hook, Action::OnUpdate);
            internal_action.OnUpdateHook = std::make_unique<utility::FunctionHook>(vtable[Action::OnUpdate], on_update_dhook.Code);
            write_void_pointer(on_update_dhook.Code, on_update_dhook.OriginalOffset, internal_action.OnUpdateHook->get_original());
            internal_action.OnUpdateHook->enable();
            internal_action.OriginalOnUpdate = internal_action.OnUpdateHook->get_original<TOnUpdateFunc>();
        } else {
            internal_action.OriginalOnUpdate = (OnUpdateFunc)existing_hook;
        }

        existing_hook = find_hook(vtable[Action::OnEnd], Action::OnEnd);
        if (!existing_hook) {
            const auto on_end_dhook = cloner->generate_hook(&on_end_hook, Action::OnEnd);
            internal_action.OnEndHook = std::make_unique<utility::FunctionHook>(vtable[Action::OnEnd], on_end_dhook.Code);
            write_void_pointer(on_end_dhook.Code, on_end_dhook.OriginalOffset, internal_action.OnEndHook->get_original());
            internal_action.OnEndHook->enable();
            internal_action.OriginalOnEnd = internal_action.OnEndHook->get_original<TOnEndFunc>();
        } else {
            internal_action.OriginalOnEnd = (OnEndFunc)existing_hook;
        }

        custom_action->OnInitialize(action, (OnInitFunc)vtable[Action::OnInitialize], s_base_on_initialize);
    }

    return set_action_set_fixup(thisptr);
}

void ActionCloner::monster_dtor_hook(Monster* thisptr) {
    const auto cloner = get();
    if (cloner->m_active_actions.contains(thisptr)) {
        cloner->m_active_actions.erase(thisptr);
    }

    return cloner->m_monster_dtor_hook->call_original<void>(thisptr);
}

void ActionCloner::text_input_hook(const wchar_t* text) {
    const auto cloner = get();
    const std::wstring wtext(text);

    if (wtext == L"/unload") {
        cloner->unload_actions();
        return;
    }
    if (wtext == L"/reload") {
        cloner->load_actions();
        return;
    }

    return cloner->m_text_input_hook->call_original<void>(text);
}

void ActionCloner::on_execute_hook(Action* thisptr, InternalAction* action) {
    if (action->OnExecuteCalled) {
        return;
    }

    action->OnExecuteCalled = true; // Prevent infinite recursion
    action->Action->OnExecute(thisptr, action->OriginalOnExecute, s_base_on_execute);
    action->OnExecuteCalled = false;
}

bool ActionCloner::on_update_hook(Action* thisptr, InternalAction* action) {
    if (action->OnUpdateCalled) {
        return action->OriginalOnUpdate(thisptr);
    }

    action->OnUpdateCalled = true; // Prevent infinite recursion
    const bool result = action->Action->OnUpdate(thisptr, action->OriginalOnUpdate, s_base_on_update);
    action->OnUpdateCalled = false;

    return result;
}

void ActionCloner::on_end_hook(Action* thisptr, InternalAction* action) {
    if (action->OnEndCalled) {
        return;
    }

    action->OnEndCalled = true; // Prevent infinite recursion
    action->Action->OnEnd(thisptr, action->OriginalOnEnd, s_base_on_end);
    action->OnEndCalled = false;
}

void ActionCloner::add_new_action_list(CustomActionList* list) {
    using namespace loader;
    const auto cloner = get();
    cloner->m_custom_action_lists.push_back(*list);
    LOG(DEBUG) << "ActionCloner: Added new action list for monster " << list->Monster << " Variant " << list->Variant;
}

void ActionCloner::set_action_set_fixup(Monster* m) {
    ((void(*)(void*))0x141bfbe10)(m);
}

InternalAction* ActionCloner::find_action(Monster* m, Action* a, Action::VtableIndex func_type) {
    const auto cloner = get();
    const auto action_list = cloner->m_active_actions.find(m);
    if (action_list == cloner->m_active_actions.end()) { // This should never happen
        return nullptr;
    }

    const auto action = action_list->second.InternalActions.find(a);
    if (action == action_list->second.InternalActions.end()) { // This should never happen
        return nullptr;
    }

    if (func_type == Action::OnExecute && action->second.OnExecuteCalled) {
        return nullptr;
    }

    if (func_type == Action::OnUpdate && action->second.OnUpdateCalled) {
        return nullptr;
    }

    if (func_type == Action::OnEnd && action->second.OnEndCalled) {
        return nullptr;
    }

    return &action->second;
}

DynamicHook ActionCloner::generate_hook(void* follow_up, Action::VtableIndex func_type) {
    using namespace asmjit;
    using namespace asmjit::x86;

    CodeHolder code;
    code.init(m_runtime.environment());

    Assembler a(&code);
    DynamicHook hook{};

    const auto follow_up_label = a.newLabel();
    const auto find_action_label = a.newLabel();
    const auto original_label = a.newLabel();
    const auto return_original_label = a.newLabel();

    a.push(rbx); // Save rbx
    a.sub(rsp, 0x20);
    a.mov(rbx, rcx);

    a.mov(r8d, func_type); // Load function type into r8d
    a.mov(rdx, rcx); // Load action into rdx
    a.mov(rcx, qword_ptr(rbx, 48)); // Load monster into rcx
    a.call(ptr(find_action_label));
    a.test(rax, rax);
    a.jz(return_original_label);

    a.mov(rdx, rax);
    a.mov(rcx, rbx);
    a.call(ptr(follow_up_label));
    a.add(rsp, 0x20);
    a.pop(rbx);
    a.ret();

    a.bind(return_original_label);
    a.mov(rcx, rbx);
    a.call(ptr(original_label));
    a.add(rsp, 0x20);
    a.pop(rbx);
    a.ret();

    a.bind(follow_up_label);
    a.dq((uint64_t)follow_up);
    a.bind(find_action_label);
    a.dq((uint64_t)&find_action);

    a.bind(original_label);
    hook.OriginalOffset = a.offset();
    a.dq(0);

    m_runtime.add(&hook.Code, &code);
    if (!hook.Code) {
        throw std::runtime_error("Failed to generate hook");
    }

    return hook;
}

void ActionCloner::load_actions_from(const std::filesystem::path& dir) {
    namespace fs = std::filesystem;
    using namespace loader;
    using InitializeActionsFunc = void(*)(CustomActionList*);

    const auto do_load = [&](const fs::path& p) {
        LOG(INFO) << "ActionCloner: Loading custom actions from " << p.string();
        const auto dll = LoadLibraryA(p.string().c_str());
        if (dll) {
            load_actions_from(dll, p);
        }
    };

    if (!fs::exists(dir)) {
        LOG(DEBUG) << "ActionCloner: Failed to load actions from " << dir.string() << " because it does not exist";
        return;
    }

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".dll") {
            do_load(entry.path());
        } else if (entry.path().extension() == ".cfg") {
            // 'redirect' must either be a fully qualified path or a path relative to the game's root directory
            std::string redirect;
            std::ifstream file(entry.path());

            while (std::getline(file, redirect)) {
                if (redirect.ends_with(".dll") && fs::exists(redirect)) {
                    do_load(redirect);
                } else {
                    load_actions_from(redirect);
                }
            }
        }
    }
}

void ActionCloner::load_actions_from(HMODULE dll, const std::filesystem::path& file) {
    using namespace loader;
    using InitializeActionApiFunc = void(*)(AddNewActionListFunc);
    using InitializeActionsFunc = void(*)(CustomActionList*);
    using GetActionListCountFunc = u32(*)();

    const auto initialize_action_api = (InitializeActionApiFunc)GetProcAddress(dll, "initialize_action_api");
    if (initialize_action_api) {
        initialize_action_api(ActionCloner::add_new_action_list);
    }

    u32 action_list_count = 1;
    const auto get_action_list_count = (GetActionListCountFunc)GetProcAddress(dll, "get_action_list_count");
    if (get_action_list_count) {
        action_list_count = get_action_list_count();
    }

    m_loaded_action_dlls.push_back(dll);
    const auto iterator = m_custom_action_lists.insert(m_custom_action_lists.end(), action_list_count, {});
    LOG(INFO) << "ActionCloner: Retrieving initialize_actions function from " << file.string();
    const auto initialize_actions = (InitializeActionsFunc)GetProcAddress(dll, "initialize_actions");
    if (!initialize_actions) {
        LOG(ERR) << "ActionCloner: Failed to get initialize_actions function from " << file.string();
        return;
    }

    LOG(INFO) << "ActionCloner: Initializing custom actions from " << file.string();

    initialize_actions(&*iterator);
    
    LOG(INFO) << "ActionCloner: Loaded custom actions from " << file.string();
}

