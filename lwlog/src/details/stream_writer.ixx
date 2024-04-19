module;
export module stream_writer;

import std;

export namespace lwlog::details 
{
	template<typename FlushPolicy>
	class stream_writer
	{
	public:
		explicit stream_writer(std::FILE* stream) : m_stream{ stream }
		{
			std::setvbuf(m_stream, nullptr, 0, FlushPolicy::buffer_size);
		}

		explicit stream_writer(std::string_view path) : m_path{ path }
		{
			if (!std::filesystem::exists(m_path.parent_path()))
			{
				std::filesystem::create_directory(m_path.parent_path());
			}

			m_stream = std::fopen(m_path.string().data(), "a");

			if (m_stream != nullptr)
			{
				std::setvbuf(m_stream, nullptr, 0, FlushPolicy::buffer_size);
			}
		}

		virtual ~stream_writer()
		{
			if (!m_path.empty())
			{
				if (m_stream != nullptr)
				{
					std::fclose(m_stream);
					m_stream = nullptr;
				}
			}
		}

	public:
		void write(std::string_view message) const
		{
			if (m_stream != nullptr)
			{
				std::fwrite(message.data(), message.size(), 1, m_stream);
				std::fwrite("\n", 1, 1, m_stream);
				FlushPolicy::flush(m_stream);
			}
		}

		std::FILE* handle() const
		{
			return m_stream;
		}

		std::filesystem::path& filesystem_path()
		{
			return m_path;
		}

	private:
		std::FILE* m_stream{ nullptr };
		std::filesystem::path m_path{};
	};
}