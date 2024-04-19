export module sink;

import level;
import pattern;
import flush_policy;
import threading_policy;
import sink_interface;
import attribute;
import os;
import std;

export namespace lwlog::sinks
{
	template<bool EnableAnsiColors, typename ThreadingPolicy>
	class sink : public interface::sink
	{
	private:
		using Mutex = typename ThreadingPolicy::mutex_t;
		using Lock = typename ThreadingPolicy::lock;

	public:
		sink()
		{
			if (!details::os::are_ansi_colors_enabled())
				details::os::enable_ansi_colors();

			m_pattern.set_pattern("[%d, %T] [%l] [%n]: %v");
			m_pattern.request_flag_formatters();

			this->add_attribute(".level(", m_current_level, [&]() -> const char*
				{
					switch (m_current_level)
					{
					case level::info:		return "\u001b[37m";
					case level::warning:	return "\u001b[33;1m";
					case level::error:		return "\u001b[31;1m";
					case level::critical:	return "\u001b[41;1m";
					case level::debug:		return "\u001b[37;1m";
					}
				});
		}

	public:
		void set_pattern(std::string_view pattern) override
		{
			Lock lock(m_mtx);
			m_pattern.set_pattern(pattern);
			m_pattern.parse_alignment_flags();
			m_pattern.request_flag_formatters();
			m_pattern.process_color_flags(EnableAnsiColors);
		}

		void add_attribute(std::string_view flag, details::attrib_value value) override
		{
			Lock lock(m_mtx);
			m_pattern.add_attribute(flag, value);
		}

		void add_attribute(std::string_view flag, details::attrib_value value, details::attrib_callback_t fn) override
		{
			Lock lock(m_mtx);
			m_pattern.add_attribute(flag, value, fn);
		}

		void set_level_filter(level level_filter) override
		{
			Lock lock(m_mtx);
			m_level_filter = level_filter;
		}

		bool should_sink(level log_level) const override
		{
			Lock lock(m_mtx);
			if (level_details::has_level(m_level_filter, log_level) ||
				level_details::has_level(m_level_filter, level::all))
				return true;
			return false;
		}

	protected:
		mutable Mutex m_mtx;
		details::pattern m_pattern;
		level m_current_level{ level::all };
		level m_level_filter{ level::all };
	};
}