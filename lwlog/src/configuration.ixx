export module configuration;

export namespace lwlog
{
    export struct enable_time {}; 
    export struct disable_time {};

    export struct enable_thread_id {}; 
    export struct disable_thread_id {};
    export struct enable_process_id {}; 
    export struct disable_process_id {};

    export struct enable_local_time {}; 
    export struct disable_local_time {};
    export struct enable_precise_units {}; 
    export struct disable_precise_units {};

    export template<typename TimePolicy, typename LocalTimePolicy, typename PreciseUnitsPolicy,
        typename ThreadIdPolicy, typename ProcessIdPolicy>
    struct configuration
    {
        using time_t = TimePolicy;
        using local_time_t = LocalTimePolicy;
        using precise_units_t = PreciseUnitsPolicy;
        using thread_id_t = ThreadIdPolicy;
        using process_id_t = ProcessIdPolicy;
    };

    export using default_config = configuration<enable_time, disable_local_time,
        enable_precise_units, enable_thread_id, enable_process_id>;
}