export module lwlog;

import logger;
import file_sink;
import stdout_sink;
import stderr_sink;
import topic;
import configuration;
import threading_policy;
import flush_policy;
import log_policy;
import logger_interface;
import log_message;
import level;
import registry;

export namespace lwlog
{
	template<template<typename, typename> typename... Sinks>
	using basic_logger = logger<default_config, synchronous_policy, immediate_flush_policy,
		single_threaded_policy, Sinks...>;

	template<template<typename, typename> typename... Sinks>
	using basic_logger_mt = logger<default_config, synchronous_policy, immediate_flush_policy,
		multi_threaded_policy, Sinks...>;

	template<template<typename, typename> typename... Sinks>
	using async_logger = logger<default_config, asynchronous_policy<>, immediate_flush_policy,
		single_threaded_policy, Sinks...>;

	template<template<typename, typename> typename... Sinks>
	using async_logger_mt = logger<default_config, asynchronous_policy<>, immediate_flush_policy,
		multi_threaded_policy, Sinks...>;

	using console_logger = basic_logger<sinks::stdout_sink>;
	using console_logger_mt = basic_logger_mt<sinks::stdout_sink>;
	using file_logger = basic_logger<sinks::file_sink>;
	using file_logger_mt = basic_logger_mt<sinks::file_sink>;

	using async_console_logger = async_logger<sinks::stdout_sink>;
	using async_console_logger_mt = async_logger_mt<sinks::stdout_sink>;
	using async_file_logger = async_logger<sinks::file_sink>;
	using async_file_logger_mt = async_logger_mt<sinks::file_sink>;

	using default_overflow_policy = block_overflow_policy;
	using default_flush_policy = immediate_flush_policy;
}

namespace lwlog
{
	void init_default_logger()
	{
		registry::init_default_logger();
	}

	void register_logger(logger_ptr logger)
	{
		registry::instance().register_logger(logger);
	}

	void set_automatic_registry(bool is_registry_automatic)
	{
		registry::instance().set_automatic_registry(is_registry_automatic);
	}

	void drop(std::string_view logger_name)
	{
		registry::instance().drop(logger_name);
	}

	void drop_all()
	{
		registry::instance().drop_all();
	}

	void apply_to_all(const std::function<void(logger_ptr)>& function)
	{
		registry::instance().apply_to_all(function);
	}

	bool is_registry_automatic()
	{
		return registry::instance().is_registry_automatic();
	}

	logger_ptr get(std::string_view logger_name)
	{
		return registry::instance().get(logger_name);
	}

	void start_topic(std::string_view name)
	{
		details::topic_registry::push(name);
	}
	void end_topic()
	{
		details::topic_registry::pop();
	}

	template<typename... Args> void info(const details::log_message& log_msg, Args&&... args)
	{
		registry::default_logger()->info(log_msg, std::forward<Args>(args)...);
	}

	template<typename... Args> void warning(const details::log_message& log_msg, Args&&... args)
	{
		registry::default_logger()->warning(log_msg, std::forward<Args>(args)...);
	}

	template<typename... Args> void error(const details::log_message& log_msg, Args&&... args)
	{
		registry::default_logger()->error(log_msg, std::forward<Args>(args)...);
	}

	template<typename... Args> void critical(const details::log_message& log_msg, Args&&... args)
	{
		registry::default_logger()->critical(log_msg, std::forward<Args>(args)...);
	}

	template<typename... Args> void debug(const details::log_message& log_msg, Args&&... args)
	{
		registry::default_logger()->debug(log_msg, std::forward<Args>(args)...);
	}
}