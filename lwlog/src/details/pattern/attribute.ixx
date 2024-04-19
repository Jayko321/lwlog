export module attribute;
import std;
import level;

export namespace lwlog::details
{
	struct flag_pair
	{
		std::string_view verbose;
		std::string_view shortened;
	};

	template<typename... Ts>
	using ref_variant = std::variant<std::reference_wrapper<Ts>...>;

	using attrib_value = ref_variant<int, float, double, std::string, std::string_view, level>;
	using attrib_callback_t = std::function<const char* ()>;

	struct attribute
	{
		attribute(std::string_view flag, attrib_value value, attrib_callback_t callback)
			: flag{ flag }
			, value{ value }
			, callback{ std::move(callback) }
		{}

		attribute(std::string_view flag, attrib_value value)
			: flag{ flag }
			, value{ value }
		{
			callback = [value]() -> const char* 
			{
				switch (value.index())
				{
				case 0:	return std::to_string(std::get<0>(value).get()).data();
				case 1:	return std::to_string(std::get<1>(value).get()).data();
				case 2:	return std::to_string(std::get<2>(value).get()).data();
				case 3:	return std::get<3>(value).get().data();
				case 4:	return std::get<4>(value).get().data();
				}
			};
		}

		std::string_view flag;
		attrib_value value;
		attrib_callback_t callback;
	};
}