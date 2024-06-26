export module log_message;

import source_meta;

export namespace lwlog::details
{
	struct log_message
	{
		template<typename T>
		log_message(T msg, source_meta meta = source_meta::current())
			: message{ msg }
			, meta{ meta }
		{}

		const char* message{};
		source_meta meta{};
	};
}