#pragma once

#include <string>
#include <unordered_map>

#include "duplex.h"

namespace lwlog::details
{
	struct pattern_attribute
	{
		std::string verbose;
		std::string shortened;
		std::string attribute;
	};

	class formatter
	{
	public:
		static std::string format(std::string pattern, bool should_color);
		static void insert_pattern_data(std::initializer_list<pattern_attribute> pattern_data);

	private:
		static std::unordered_map<duplex, std::string> m_pattern_data;
		static std::unordered_map<std::string_view, std::string_view> m_color_data;
	};
}