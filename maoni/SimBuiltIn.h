// This is the stuff that's required to run in the generated files

#ifndef __SIM_BUILT_IN_H__
#define __SIM_BUILT_IN_H__

#define BACKGROUND_GC

#define max_generation 2
#define dprintf(l, x)
#define high_memory_load_th 90
#define v_high_memory_load_th 97
#define total_generation_count 5

// TEMP! Should get this from input
#define mem_one_percent ((size_t)10 * 1024 * 1024)

enum gc_tuning_point
{
    tuning_deciding_condemned_gen = 0,
    tuning_deciding_full_gc = 1,
    tuning_deciding_compaction = 2,
    tuning_deciding_expansion = 3,
    tuning_deciding_promote_ephemeral = 4,
    tuning_deciding_short_on_seg = 5
};

class dynamic_data
{
public:
    ptrdiff_t new_allocation;
    ptrdiff_t gc_new_allocation; // new allocation at beginning of gc
    float     surv;
    size_t    desired_allocation;

    // # of bytes taken by objects (ie, not free space) at the beginning
    // of the GC.
    size_t    begin_data_size;
    // # of bytes taken by survived objects after mark.
    size_t    survived_size;
    // # of bytes taken by survived pinned plugs after mark.
    size_t    pinned_survived_size;
    size_t    artificial_pinned_survived_size;
    size_t    added_pinned_size;

#ifdef SHORT_PLUGS
    size_t    padding_size;
#endif //SHORT_PLUGS
#if defined (RESPECT_LARGE_ALIGNMENT) || defined (FEATURE_STRUCTALIGN)
    // # of plugs that are not pinned plugs.
    size_t    num_npinned_plugs;
#endif //RESPECT_LARGE_ALIGNMENT || FEATURE_STRUCTALIGN
    //total object size after a GC, ie, doesn't include fragmentation
    size_t    current_size;
    size_t    collection_count;
    size_t    promoted_size;
    size_t    freach_previous_promotion;
    size_t    fragmentation;    //fragmentation when we don't compact
    size_t    gc_clock;         //gc# when last GC happened
    uint64_t  time_clock;       //time when last gc started
    uint64_t  previous_time_clock; // time when previous gc started
    size_t    gc_elapsed_time;  // Time it took for the gc to complete
    float     gc_speed;         //  speed in bytes/msec for the gc to complete

    size_t    min_size;
};

class gc_mechanisms
{
public:
    size_t gc_index; // starts from 1 for the first GC, like dd_collection_count
    int condemned_generation;
    BOOL promotion;
    BOOL compaction;
    BOOL loh_compaction;
    BOOL heap_expansion;
    uint32_t concurrent;
    BOOL demotion;
    BOOL card_bundles;
    int  gen0_reduction_count;
    BOOL should_lock_elevation;
    int elevation_locked_count;
    BOOL elevation_reduced;
    BOOL minimal_gc;
    BOOL found_finalizers;

#ifdef STRESS_HEAP
    BOOL stress_induced;
#endif // STRESS_HEAP

    // These are opportunistically set
    uint32_t entry_memory_load;
    uint64_t entry_available_physical_mem;
    uint32_t exit_memory_load;
};

extern dynamic_data dynamic_data_table[total_generation_count];
extern size_t generation_table[total_generation_count];
extern gc_mechanisms settings;

inline
dynamic_data* dynamic_data_of(int gen_number)
{
    return &dynamic_data_table[gen_number];
}

inline
size_t generation_size(int gen_number)
{
    return generation_table[gen_number];
}

inline
size_t& dd_current_size(dynamic_data* inst)
{
    return inst->current_size;
}

inline
size_t& dd_fragmentation(dynamic_data* inst)
{
    return inst->fragmentation;
}

inline
ptrdiff_t& dd_new_allocation(dynamic_data* inst)
{
    return inst->new_allocation;
}

inline
size_t& dd_desired_allocation(dynamic_data* inst)
{
    return inst->desired_allocation;
}

inline
float& dd_surv(dynamic_data* inst)
{
    return inst->surv;
}
#endif //__SIM_BUILT_IN_H__