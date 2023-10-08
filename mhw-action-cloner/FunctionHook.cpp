#include "pch.h"
#include "FunctionHook.h"

#include <MinHook.h>
#include <stdexcept>

bool utility::FunctionHook::s_minhook_initialized = false;

#define MH_ASSERT_OK(CALL) if (const auto _result = CALL; _result != MH_OK) throw std::runtime_error(MH_StatusToString(_result))

utility::FunctionHook::FunctionHook(void* target, void* hook)
    : m_target(target)
    , m_original(nullptr)
    , m_hook(hook) {

    if (!s_minhook_initialized && MH_Initialize() == MH_OK) {
        s_minhook_initialized = true;
    }

    MH_ASSERT_OK(MH_CreateHook(m_target, m_hook, &m_original));
}

utility::FunctionHook::FunctionHook(FunctionHook&& f) noexcept {
    m_target = f.m_target;
    m_original = f.m_original;
    m_hook = f.m_hook;
    f.m_target = nullptr;
    f.m_original = nullptr;
    f.m_hook = nullptr;
}

utility::FunctionHook& utility::FunctionHook::operator=(FunctionHook&& f) noexcept {
    if (this != &f) {
        m_target = f.m_target;
        m_original = f.m_original;
        m_hook = f.m_hook;
        f.m_target = nullptr;
        f.m_original = nullptr;
        f.m_hook = nullptr;
    }
    
    return *this;
}

utility::FunctionHook::~FunctionHook() {
    disable();
    MH_ASSERT_OK(MH_RemoveHook(m_target));
}

void utility::FunctionHook::enable() const {
    if (!is_valid()) {
        return;
    }

    MH_ASSERT_OK(MH_EnableHook(m_target));
}

void utility::FunctionHook::disable() const {
    if (!is_valid()) {
        return;
    }

    MH_ASSERT_OK(MH_DisableHook(m_target));
}
