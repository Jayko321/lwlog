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

			const std::string_view pattern_view{ m_pattern };
			const std::size_t flag_end_pos{ pattern_view.find(' ', flag_start_pos) };
			const std::string_view flag_view{ pattern_view.substr(flag_start_pos, flag_end_pos - flag_start_pos) };

			const bool has_fill_char{ !std::isdigit(flag_view[2]) };
			const std::uint8_t flag_length = has_fill_char ? 3 : 2;

			const std::string_view width_str{ flag_view.substr(flag_length,
				std::isdigit(flag_view[flag_length + 1]) ? 2 : 1) };

			alignment_specification alignment_spec;

			alignment_spec.fill_char = has_fill_char ? flag_view[2] : ' ';
			alignment_spec.width = static_cast<std::uint8_t>(std::stoi(width_str.data()));
			alignment_spec.alignment_flag = flag_view.substr(0, flag_length + width_str.size());

			switch (flag_view[1])
			{
			case '<': alignment_spec.side = alignment_specification::align_side::left;		break;
			case '>': alignment_spec.side = alignment_specification::align_side::right;		break;
			case '^': alignment_spec.side = alignment_specification::align_side::center;	break;
			}

			m_alignment_specs.push_back(alignment_spec);

			std::string_view flag_to_align;
			if (const std::size_t short_flag_pos{ flag_view.find('%') };
				short_flag_pos != std::string_view::npos)
			{
				flag_to_align = flag_view.substr(short_flag_pos, 2);
			}
			else if (const std::size_t verbose_flag_pos{ flag_view.find('{') };
				verbose_flag_pos != std::string_view::npos)
			{
				flag_to_align = flag_view.substr(verbose_flag_pos,
					flag_view.find('}', verbose_flag_pos) - verbose_flag_pos + 1);
			}

			m_pattern.insert(pattern_view.find(flag_to_align, flag_start_pos) + flag_to_align.size(),
				alignment_specification::flag_end_indicator);

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