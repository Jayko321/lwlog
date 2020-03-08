#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_set>
#include <regex>

namespace details::utilities
{
	static std::vector<std::string> populate_with_regex_matches_from_str(std::regex reg, std::string str)
	{
		std::smatch matches;
		std::string temp = str;

		std::vector<std::string> vec;

		while (std::regex_search(temp, matches, reg))
		{
			vec.push_back(matches.str(1));
			temp = matches.suffix().str();
		}

		return vec;
	}

	static void replace_in_string(std::string& str, std::string_view to_replace, std::string_view replace_with)
	{
		while (str.find(to_replace) != std::string::npos)
		{
			str.replace(str.find(to_replace), to_replace.length(), replace_with);
		}
	}

	template<typename... Args>
	void populate_vec_with_variadic_params(std::vector<std::string>& v, Args&& ... args)
	{
		std::vector<std::string> vec =
		{
			[](auto&& arg)
			{
				if constexpr (std::is_same_v<std::remove_reference_t<decltype(arg)>, char>)
				{
					return std::string(1, arg);
				}
				else if constexpr (std::is_arithmetic_v<std::remove_reference_t<decltype(arg)>>)
				{
					return std::to_string(std::forward<decltype(arg)>(arg));
				}
				else
				{
					return arg;
				}
			}(std::forward<Args>(args))...
		};

		for (const auto& i : vec)
		{
			v.push_back(i);
		}
	}

	template<typename T, typename T1>
	void string_to_numeric_vec(std::vector<T> sv, std::vector<T1>& nv, const char* chars_to_remove)
	{
		for (int i = 0; i < sv.size(); ++i)
		{
			std::string temp = sv[i];

			for (int i = 0; i < strlen(chars_to_remove); ++i)
			{
				temp.erase(std::remove(temp.begin(), temp.end(), chars_to_remove[i]), temp.end());
			}
			nv.push_back(std::stoi(temp));
		}
	}

	template<typename T>
	void remove_duplicates_in_vec(std::vector<T>& vec)
	{
		std::unordered_set<T> s;
		for (const auto& i : vec)
		{
			s.insert(i);
		}

		vec.assign(s.begin(), s.end());
	}
}
