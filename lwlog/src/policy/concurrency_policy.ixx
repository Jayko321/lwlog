module;

#include <xmmintrin.h>
#include <emmintrin.h>

export module concurrency_policy;
import std;

export namespace lwlog
{
    export struct spsc_model_policy {};
    export struct mpsc_model_policy {};

    export struct block_overflow_policy
    {
        static void handle_overflow() { _mm_pause(); }
        static void handle_underflow() { _mm_pause(); }
        static bool should_discard() { return false; }
    };

    export struct overwrite_last_overflow_policy
    {
        static void handle_overflow() {}
        static void handle_underflow() {}
        static bool should_discard() { return false; }
    };

    export struct discard_new_overflow_policy
    {
        static void handle_overflow() { m_is_full.store(true, std::memory_order_relaxed); }
        static void handle_underflow() { m_is_full.store(false, std::memory_order_relaxed); }
        static bool should_discard() { return m_is_full.load(std::memory_order_relaxed); }

    private:
        static inline std::atomic_bool m_is_full{ false };
    };
}