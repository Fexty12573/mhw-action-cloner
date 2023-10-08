#pragma once

namespace utility {

class FunctionHook {
public:
    FunctionHook(void* target, void* hook);
    FunctionHook(const FunctionHook&) = delete;
    FunctionHook(FunctionHook&& f) noexcept;
    ~FunctionHook();

    FunctionHook& operator=(const FunctionHook&) = delete;
    FunctionHook& operator=(FunctionHook&& f) noexcept;

    void enable() const;
    void disable() const;

    bool is_valid() const { return m_original != nullptr; }

    void* get_target() const { return m_target; }
    void* get_hook() const { return m_hook; }
    template <class Func = void> Func* get_original() const { return (Func*)m_original; }
    template <class Ret, class... Args> Ret call_original(Args... args) { return get_original<Ret(Args...)>()(args...); }

private:
    void* m_target;
    void* m_original;
    void* m_hook;

    static bool s_minhook_initialized;
};

}
