export module log_policy;

import std;
import level;
import sink;
import bounded_queue;
import argument_format;
import source_meta;
import concurrency_policy;
import sink_interface;
import record;

export namespace lwlog
{
	inline constexpr std::size_t default_async_queue_size{ 1024 };

	struct synchronous_policy
	{
		template<typename ConcurrencyModelPolicy>
		struct backend
		{
			std::vector<sink_ptr> sink_storage;
		};

		template<typename Config, typename ConcurrencyModelPolicy>
		static void init(backend<ConcurrencyModelPolicy>&) {}

		template<typename Config, typename ConcurrencyModelPolicy>
		static void log(backend<ConcurrencyModelPolicy>& backend, std::string_view message,
			level log_level, const details::source_meta& meta, details::format_args_list args)
		{
			const auto formatted_message{ details::format_args(message, args) };
			const details::record<Config> record{ formatted_message, log_level, meta };

			for (const auto& sink : backend.sink_storage)
			{
				if (sink->should_sink(record.log_level))
				{
					sink->sink_it(record);
				}
			}
		}
	};

	template<
		std::size_t Capacity = default_async_queue_size,
		typename OverflowPolicy = block_overflow_policy
	>
	struct asynchronous_policy
	{
		template<typename ConcurrencyModelPolicy>
		struct backend
		{
			~backend()
			{
				shutdown.store(true, std::memory_order_relaxed);

				if (worker_thread.joinable())
				{
					worker_thread.join();
				}
			}

			std::vector<sink_ptr> sink_storage;

			std::atomic_bool shutdown;
			std::thread worker_thread;

			struct queue_item
			{
				std::string_view			message;
				level						log_level;
				details::source_meta		meta;
				details::format_args_list	args;
			};

			details::bounded_queue<
				Capacity,
				queue_item,
				OverflowPolicy,
				ConcurrencyModelPolicy
			> queue;
		};

		template<typename Config, typename ConcurrencyModelPolicy>
		static void init(backend<ConcurrencyModelPolicy>& backend)
		{
			backend.shutdown.store(false, std::memory_order_relaxed);
			backend.worker_thread = std::thread([&backend]()
				{
					while (true)
					{
						if (backend.shutdown.load(std::memory_order_relaxed)
							&& backend.queue.is_empty()) break;

						if (!backend.queue.is_empty())
						{
							const auto item{ backend.queue.dequeue() };
							const auto formatted_message{ details::format_args(item.message, item.args) };
							const details::record<Config> record{ formatted_message, item.log_level, item.meta };

							for (const auto& sink : backend.sink_storage)
							{
								if (sink->should_sink(record.log_level))
								{
									sink->sink_it(record);
								}
							}
						}
					}
				});
		}

		template<typename Config, typename ConcurrencyModelPolicy>
		static void log(backend<ConcurrencyModelPolicy>& backend, std::string_view message,
			level log_level, const details::source_meta& meta, details::format_args_list args)
		{
			backend.queue.enqueue({ message, log_level, meta, args });
		}
	};
}