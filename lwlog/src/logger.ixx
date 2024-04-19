export module logger;

import log_policy;
import logger_interface;
import sink_interface;
import attribute;
import argument_format;
import sink;
import sink_factory;
import registry;
import std;
import configuration;
import flush_policy;
import threading_policy;
import stdout_sink;

export namespace lwlog
{
	template<typename Config, typename LogExecutionPolicy, typename FlushPolicy,
		typename ThreadingPolicy, template<typename, typename> typename... Sinks>
	class logger : public interface::logger
	{
	public:
		template<typename... SinkParams>
		logger(std::string_view name, SinkParams&&... params)
		{
			LogExecutionPolicy::template init<Config>(m_backend);

			if (registry::instance().is_registry_automatic())
				registry::instance().register_logger(this);

			m_backend.sink_storage = { sinks::sink_factory<Sinks<FlushPolicy, ThreadingPolicy>>::request(
				std::forward<SinkParams>(params)...
			)... };

			this->add_attribute("{name}", m_name);
			this->add_attribute("%n", m_name);
		}

		template<typename Iterator, typename... SinkParams>
		logger(std::string_view name, Iterator begin, Iterator end, SinkParams&&... params)
		{
			m_backend.sink_storage.insert(m_backend.sink_storage.end(), begin, end);
		}

		template<typename... SinkParams>
		logger(std::string_view name, sink_list sink_list, SinkParams&&... params)
			: logger{ name, sink_list.begin(), sink_list.end(), params... }
		{}

		template<typename... SinkParams>
		logger(std::string_view name, sink_ptr sink, SinkParams&&... params)
			: logger{ name, { sink }, params... }
		{}

	public:
		void add_sink(sink_ptr sink)
		{
			m_backend.sink_storage.push_back(sink);
		}

		void remove_sink(sink_ptr sink)
		{
			for (std::size_t i = 0; i < m_backend.sink_storage.size(); ++i)
			{
				if (m_backend.sink_storage[i] == sink)
					m_backend.sink_storage.erase(m_backend.sink_storage.begin() + i);
			}
		}

		void set_pattern(std::string_view pattern) override
		{
			for (const auto& sink : m_backend.sink_storage)
			{
				sink->set_pattern(pattern);
			}
		}

		void add_attribute(std::string_view flag, details::attrib_value value) override
		{
			for (const auto& sink : m_backend.sink_storage)
			{
				sink->add_attribute(flag, value);
			}
		}

		void add_attribute(std::string_view flag, details::attrib_value value, details::attrib_callback_t fn) override
		{
			for (const auto& sink : m_backend.sink_storage)
			{
				sink->add_attribute(flag, value, fn);
			}
		}

		void set_level_filter(level log_level) override
		{
			for (const auto& sink : m_backend.sink_storage)
			{
				sink->set_level_filter(log_level);
			}
		}

		std::string_view name() const override
		{
			return m_name;
		}

		std::vector<sink_ptr>& sinks()
		{
			return m_backend.sink_storage;
		}

	private:
		void log(const details::log_message& log_msg, level log_level, details::format_args_list args) override
		{
			LogExecutionPolicy::template log<Config>(m_backend, log_msg.message, log_level, log_msg.meta, args);
		}

		void info_impl(const details::log_message& log_msg, details::format_args_list args) override
		{
			this->log(log_msg, level::info, args);
		}

		void warning_impl(const details::log_message& log_msg, details::format_args_list args) override
		{
			this->log(log_msg, level::warning, args);
		}

		void error_impl(const details::log_message& log_msg, details::format_args_list args) override
		{
			this->log(log_msg, level::error, args);
		}

		void critical_impl(const details::log_message& log_msg, details::format_args_list args) override
		{
			this->log(log_msg, level::critical, args);
		}

		void debug_impl(const details::log_message& log_msg, details::format_args_list args) override
		{
			this->log(log_msg, level::debug, args);
		}

	private:
		std::string_view m_name;
		typename LogExecutionPolicy::template backend<
			typename ThreadingPolicy::concurrency_model_policy> m_backend;
	};
}