module;

export module registry;

import std;
import logger_interface;
import configuration;
import threading_policy;
import log_policy;
import stdout_sink;
import flush_policy;

export namespace lwlog
{
	class registry
	{
	public:
		registry() = default;
		registry(const registry&) = delete;
		registry(registry&&) = delete;
		registry& operator=(const registry&) = delete;
		registry& operator=(registry&&) = delete;

	public:
		static registry& instance()
		{
			static registry s_instance;
			return s_instance;
		}

		static void init_default_logger();



	public:
		void register_logger(logger_ptr logger)
		{
			if (logger->name() != "DEFAULT") m_loggers[logger->name()] = logger;
		}

		void set_automatic_registry(bool is_registry_automatic)
		{
			m_is_registry_automatic = is_registry_automatic;
		}

		void drop(std::string_view logger_name)
		{
			m_loggers.erase(logger_name);
		}

		void drop_all()
		{
			m_loggers.clear();
		}

		void apply_to_all(const std::function<void(logger_ptr)>& fn) const
		{
			for (const auto& [name, logger] : m_loggers) fn(logger);
		}

	public:
		bool is_registry_automatic() const
		{
			return m_is_registry_automatic;
		}

		const std::unordered_map<std::string_view, logger_ptr>& loggers() const
		{
			return m_loggers;
		}

		logger_ptr get(std::string_view logger_name)
		{
			return m_loggers[logger_name];
		}

	private:
		bool m_is_registry_automatic{ true };
		std::unordered_map<std::string_view, logger_ptr> m_loggers;
	};
}