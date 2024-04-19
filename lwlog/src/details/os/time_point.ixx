module;
export module time_point;

import std;
import configuration;
import datetime_utility;

#ifdef _WIN32
	import "Windows.h";
#endif


export namespace lwlog::details::os
{
	#ifdef _WIN32
		using time_point_t = ::SYSTEMTIME;
	#else
		using time_point_t = std::chrono::system_clock::time_point;
	#endif

	class time_point_base
	{
	public:
		time_point_base()
		{
			#ifdef _WIN32
						::GetSystemTime(&m_now);

						year = (uint8_t)m_now.wYear;
						month = (uint8_t)m_now.wMonth;
						weekday = (uint8_t)m_now.wDayOfWeek;
						day = (uint8_t)m_now.wDay;
						hour = (uint8_t)m_now.wHour;
						minute = (uint8_t)m_now.wMinute;
						second = (uint8_t)m_now.wSecond;
			#else
						m_now = std::chrono::system_clock::now();
						const std::time_t now_time_t{ std::chrono::system_clock::to_time_t(m_now) };
						const std::tm* details{ std::gmtime(&now_time_t) };

						year = details->tm_year + 1900;
						month = details->tm_mon + 1;
						weekday = details->tm_wday;
						day = details->tm_mday;
						hour = details->tm_hour;
						minute = details->tm_min;
						second = details->tm_sec;
			#endif
		}

		virtual ~time_point_base() = default;

	public:
		virtual std::uint16_t millisecond()	const
		{
			return {};
		}

		virtual std::uint32_t microsecond()	const
		{
			return {};
		}

		virtual std::uint32_t nanosecond() const
		{
			return {};
		}
		
		virtual std::string ampm() const
		{
			return time_point_base::hour >= 12 ? "pm" : "am";
		}

	public:
		virtual std::string to_string(std::string_view unit) const
		{
			return unit.data();
		}

		virtual std::string to_string(std::uint32_t unit) const
		{
			return datetime::ensure_two_digit_format(unit);
		}

		virtual std::string build_timestamp(std::uint32_t first_unit, std::uint32_t second_unit, char delimeter) const
		{
			return datetime::ensure_two_digit_format(first_unit) + delimeter
				+ datetime::ensure_two_digit_format(second_unit);
		}

		virtual std::string build_timestamp(std::uint32_t first_unit, std::uint32_t second_unit, std::uint32_t third_unit, char delimeter) const
		{
			return datetime::ensure_two_digit_format(first_unit) + delimeter
				+ datetime::ensure_two_digit_format(second_unit) + delimeter
				+ datetime::ensure_two_digit_format(third_unit);
		}

	public:
		std::uint16_t year;
		std::uint8_t month;
		std::uint8_t weekday;
		std::uint8_t day;
		std::uint8_t hour;
		std::uint8_t minute;
		std::uint8_t second;

	protected:
		time_point_t m_now;
	};

	template<typename TimePolicy, typename LocalTimePolicy, typename PreciseUnitsPolicy>
	class time_point : public time_point_base
	{
	public:
		time_point()
		{
			time_point_base::hour = datetime::handle_timezone<LocalTimePolicy>(time_point_base::hour);
		}
	};

	template<typename LocalTimePolicy, typename PreciseUnitsPolicy>
	class time_point<disable_time, LocalTimePolicy, PreciseUnitsPolicy>
		: public time_point_base
	{
	public:
		time_point() = default;

	public:
		std::string ampm() const override
		{
			return {};
		}

		std::string to_string(std::string_view unit) const override
		{
			return {};
		}

		std::string to_string(std::uint32_t unit) const override
		{
			return {};
		}

		std::string build_timestamp(std::uint32_t first_unit, std::uint32_t second_unit, char delimeter) const override
		{
			return {};
		}

		std::string build_timestamp(std::uint32_t first_unit, std::uint32_t second_unit, std::uint32_t third_unit, char delimeter) const override
		{
			return {};
		}
	};

	template<typename LocalTimePolicy>
	class time_point<enable_time, LocalTimePolicy, enable_precise_units>
		: public time_point_base
	{
	public:
		time_point()
		{
			#ifdef _WIN32
						::FILETIME now_ft;
						::GetSystemTimePreciseAsFileTime(&now_ft);

						::ULARGE_INTEGER ticks_since_windows_epoch;
						ticks_since_windows_epoch.LowPart = now_ft.dwLowDateTime;
						ticks_since_windows_epoch.HighPart = now_ft.dwHighDateTime;

						const std::uint64_t ticks_since_last_second{ ticks_since_windows_epoch.QuadPart % 10'000'000 };

						m_millisecond = m_now.wMilliseconds;
						m_microsecond = (ticks_since_last_second / 10) % 1'000'000;
						m_nanosecond = ticks_since_last_second * 100 % 1'000'000'000;
			#else
						const auto nanoseconds_since_unix_epoch{ std::chrono::duration_cast<std::chrono::nanoseconds>(m_now.time_since_epoch()).count() };

						m_millisecond = (nanoseconds_since_unix_epoch / 1'000'000) % 1'000;
						m_microsecond = (nanoseconds_since_unix_epoch / 1'000) % 1'000'000;
						m_nanosecond = nanoseconds_since_unix_epoch % 1'000'000'000;
			#endif

			time_point_base::hour = datetime::handle_timezone<LocalTimePolicy>(time_point_base::hour);
		}


	public:
		std::uint16_t millisecond() const override
		{
			return m_millisecond;
		}

		std::uint32_t microsecond() const override
		{
			return m_microsecond;
		}

		std::uint32_t nanosecond() const override
		{
			return m_nanosecond;
		}

	private:
		std::uint16_t m_millisecond;
		std::uint32_t m_microsecond;
		std::uint32_t m_nanosecond;
	};
}