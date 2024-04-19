export module os;

import std;
import configuration;

#ifdef _WIN32
	import "Windows.h";
#elif defined(__linux__) || defined(__APPLE__)
	import <unistd.h>;
	import <sys/syscall.h>;
#else
	import std;
#endif

export namespace lwlog::details::os
{
	template<typename ThreadIdPolicy>
	std::uint64_t get_thread_id()
	{
		std::uint64_t thread_id{};

		#if defined(_WIN32)
				thread_id = static_cast<std::uint64_t>(::GetCurrentThreadId());
		#elif defined(__linux__)
				thread_id = static_cast<std::uint64_t>(::syscall(SYS_gettid));
		#elif defined(__APPLE__)
				::pthread_threadid_np(NULL, &thread_id);
		#else
				thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
		#endif

		return thread_id;
	}

	template<>
	std::uint64_t get_thread_id<disable_thread_id>()
	{
		return {};
	}

	template<typename ProcessIdPolicy>
	std::uint64_t get_process_id()
	{
		#if defined(_WIN32)
				return static_cast<std::uint64_t>(::GetCurrentProcessId());
		#elif defined(__linux__) || defined(__APPLE__)
				return static_cast<std::uint64_t>(::getpid());
		#endif
	}

	template<>
	std::uint64_t get_process_id<disable_process_id>()
	{
		return {};
	}

	export struct execution_context_base
	{
		virtual ~execution_context_base() = default;

		virtual std::uint64_t thread_id() const = 0;
		virtual std::uint64_t process_id() const = 0;
	};

	export template<typename ThreadIdPolicy, typename ProcessIdPolicy>
	struct execution_context : public execution_context_base
	{
		std::uint64_t thread_id() const override
		{
			return get_thread_id<ThreadIdPolicy>();
		}

		std::uint64_t process_id() const override
		{
			return get_process_id<ProcessIdPolicy>();
		}
	};

	bool are_ansi_colors_enabled()
	{
		#ifdef _WIN32
				const ::HANDLE handle{ ::GetStdHandle(STD_OUTPUT_HANDLE) };
				::DWORD mode{};

				return ::GetConsoleMode(handle, &mode) &&
					(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		#else
				return true;
		#endif
	}

	void enable_ansi_colors()
	{
		#ifdef _WIN32
				const ::HANDLE handle{ ::GetStdHandle(STD_OUTPUT_HANDLE) };
				::DWORD mode{};

				if (handle == INVALID_HANDLE_VALUE) return;
				if (!::GetConsoleMode(handle, &mode)) return;

				mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

				if (!::SetConsoleMode(handle, mode)) return;
		#endif
	}
}