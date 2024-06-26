export module datetime_utility;

import std;

#ifdef _WIN32
	import "Windows.h";
#else
	#include <ctime>
#endif

import configuration;


export namespace lwlog::details::os::datetime
{
	inline const std::int16_t cached_timezone_offset = []() 
	{
		#ifdef _WIN32
				::TIME_ZONE_INFORMATION tz_info;
				::GetTimeZoneInformation(&tz_info);

				return -(int16_t)tz_info.Bias / 60;
		#else
				std::time_t now{ std::time(nullptr) };
				std::tm gm_time{ *std::gmtime(&now) };
				std::tm local_time{ *std::localtime(&now) };

				const std::time_t local_epoch{ std::mktime(&local_time) };
				const std::time_t gm_epoch{ std::mktime(&gm_time) };

				std::int16_t difference = std::difftime(local_epoch, gm_epoch) / 3600;
				return difference;
		#endif
	}();

	inline const std::array<const char*, 12> month_name = { "January", "February", "March", "April", "May",
		"June", "July", "August", "September", "October", "November", "December" };

	inline const std::array<const char*, 12> month_name_short = { "Jan", "Feb", "Mar", "Apr", "May","Jun",
		"Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };

	inline const std::array<const char*, 7> weekday_name = { "Monday", "Tuesday", "Wednesday",
		"Thursday","Friday", "Saturday", "Sunday" };

	inline const std::array<const char*, 7> weekday_name_short = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

	template<typename LocalTimePolicy>
	std::uint8_t handle_timezone(std::uint8_t hour)
	{
		return hour;
	}

	template<>
	std::uint8_t handle_timezone<enable_local_time>(std::uint8_t hour)
	{
		hour += cached_timezone_offset;
		hour = (hour <= 23) ? hour : hour - 24;

		return hour;
	}

	std::uint16_t to_12h(std::uint8_t hour)
	{
		return hour > 12 ? hour - 12 : hour;
	}

	std::string ensure_two_digit_format(std::uint32_t digit)
	{
		return (digit <= 9 ? "0" : "") + std::to_string(digit);
	}
}