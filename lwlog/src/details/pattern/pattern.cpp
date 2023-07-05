#include "pattern.h"
#include "format_data.h"
#include "color_format_data.h"

namespace lwlog::details
{
	std::string pattern::compile(const record& record) const
	{
		std::string compiled = m_pattern;
		for (const auto& formatter : m_formatters)
			formatter->format(compiled, record);

		for (const auto& [flags, value, callback] : m_attributes)
			formatter::format_attribute(compiled, flags, callback());

		for (const auto& spec : m_alignment_specs)
			alignment_formatter::format(compiled, spec);

		return compiled;
	}

	void pattern::parse_alignment_specs()
	{
		std::size_t flag_start_pos{ 0 };
		while ((flag_start_pos = m_pattern.find_first_of("<>^", flag_start_pos)) != std::string::npos)
		{
			flag_start_pos -= 1;

			const std::size_t flag_end_pos{ m_pattern.find(' ', flag_start_pos) };

			m_alignment_specs.emplace_back(m_pattern.substr(flag_start_pos,
				(flag_end_pos == std::string::npos ? m_pattern.size() : flag_end_pos) - flag_start_pos));

			const std::string& flag_to_align{ m_alignment_specs.back().flag_to_align };
			const std::size_t flag_to_align_pos{ m_pattern.find(flag_to_align, flag_start_pos) };

			m_pattern.replace(flag_to_align_pos, flag_to_align.size(),
				flag_to_align + alignment_specification::end_flag);

			flag_start_pos += flag_to_align.size();
		}
	}

	void pattern::request_flag_formatters()
	{
		const auto verbose_flags{ this->parse_verbose_flags() };
		const auto short_flags{ this->parse_short_flags() };

		for (const auto& flag : verbose_flags)
			if (verbose_data[flag])
				m_formatters.push_back(verbose_data[flag]);

		for (const auto& flag : short_flags)
			if (shortened_data[flag])
				m_formatters.push_back(shortened_data[flag]);
	}

	void pattern::process_color_flags(bool use_color)
	{
		if (std::strchr(m_pattern.data(), '.'))
		{
			while (std::strchr(m_pattern.data(), ')'))
				m_pattern.replace(m_pattern.find(')'), 1, use_color ? "\u001b[0m" : "");

			for (const auto& [key, value] : color_data)
				while (std::strstr(m_pattern.data(), key.data()))
					m_pattern.replace(m_pattern.find(key), key.length(), use_color ? value : "");
		}
	}

	void pattern::set_pattern(std::string_view pattern)
	{
		m_pattern = pattern;
	}

	void pattern::add_attribute(std::string_view flag, attrib_value value)
	{
		m_attributes.emplace_back(flag, value);
	}

	void pattern::add_attribute(std::string_view flag, attrib_value value, details::attrib_callback_t fn)
	{
		m_attributes.emplace_back(flag, value, fn);
	}

	std::vector<std::string_view> pattern::parse_verbose_flags() const
	{
		const std::string_view pattern_view{ m_pattern };

		std::vector<std::string_view> flags;
		flags.reserve(std::count(pattern_view.begin(), pattern_view.end(), '{'));

		std::size_t flag_start_pos{ 0 };
		while ((flag_start_pos = pattern_view.find('{', flag_start_pos)) != std::string_view::npos)
		{
			const std::size_t flag_end_pos{ pattern_view.find('}', flag_start_pos + 1) };
			const std::size_t flag_size{ flag_end_pos - flag_start_pos + 1 };

			flags.emplace_back(pattern_view.substr(flag_start_pos, flag_size));
			flag_start_pos = pattern_view.find('{', flag_start_pos + 1);
		}
		return flags;
	}

	std::vector<std::string_view> pattern::parse_short_flags() const
	{
		constexpr std::uint8_t flag_size{ 2 };
		const std::string_view pattern_view{ m_pattern };

		std::vector<std::string_view> flags;
		flags.reserve(pattern_view.size() / flag_size);

		std::size_t flag_start_pos{ 0 };
		while ((flag_start_pos = pattern_view.find('%', flag_start_pos)) != std::string_view::npos)
		{
			flags.emplace_back(pattern_view.substr(flag_start_pos, flag_size));
			flag_start_pos += flag_size;
		}
		return flags;
	}
}