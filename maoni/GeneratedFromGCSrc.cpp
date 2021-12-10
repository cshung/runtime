#include "Generated.h"
#include <cassert>

// @@TUNING_FUNC_PER_HEAP
inline BOOL
gc_heap::dt_estimate_reclaim_space_p(gc_tuning_point tp, int gen_number)
{
    BOOL ret = FALSE;

    switch (tp)
    {
    case tuning_deciding_condemned_gen:
    {
        if (gen_number == max_generation)
        {
            size_t est_maxgen_free = estimated_reclaim(gen_number);

            uint32_t num_heaps = 1;
#ifdef MULTIPLE_HEAPS
            num_heaps = gc_heap::n_heaps;
#endif //MULTIPLE_HEAPS

            size_t min_frag_th = min_reclaim_fragmentation_threshold(settings.entry_memory_load, num_heaps);
            dprintf(GTC_LOG, ("h%d, min frag is %Id", heap_number, min_frag_th));
            ret = (est_maxgen_free >= min_frag_th);
        }
        else
        {
            assert(0);
        }
        break;
    }

    default:
        break;
    }

    return ret;
}

// @@TUNING_FUNC_PER_HEAP, dd_current_size(s), dd_fragmentation(2), dd_desired_allocation(2), dd_new_allocation(2)
// DTREVIEW: Right now we only estimate gen2 fragmentation.
// on 64-bit though we should consider gen1 or even gen0 fragmentation as
// well
inline BOOL
gc_heap::dt_estimate_high_frag_p(gc_tuning_point tp, int gen_number, uint64_t available_mem)
{
    BOOL ret = FALSE;

    switch (tp)
    {
    case tuning_deciding_condemned_gen:
    {
        if (gen_number == max_generation)
        {
            dynamic_data* dd = dynamic_data_of(gen_number);
            float est_frag_ratio = 0;
            if (dd_current_size(dd) == 0)
            {
                est_frag_ratio = 1;
            }
            else if ((dd_fragmentation(dd) == 0) || (dd_fragmentation(dd) + dd_current_size(dd) == 0))
            {
                est_frag_ratio = 0;
            }
            else
            {
                est_frag_ratio = (float)dd_fragmentation(dd) / (float)(dd_fragmentation(dd) + dd_current_size(dd));
            }

            size_t est_frag = (dd_fragmentation(dd) + (size_t)((dd_desired_allocation(dd) - dd_new_allocation(dd)) * est_frag_ratio));
            dprintf(GTC_LOG, ("h%d: gen%d: current_size is %Id, frag is %Id, est_frag_ratio is %d%%, estimated frag is %Id",
                heap_number,
                gen_number,
                dd_current_size(dd),
                dd_fragmentation(dd),
                (int)(est_frag_ratio * 100),
                est_frag));

            uint32_t num_heaps = 1;

#ifdef MULTIPLE_HEAPS
            num_heaps = gc_heap::n_heaps;
#endif //MULTIPLE_HEAPS
            uint64_t min_frag_th = min_high_fragmentation_threshold(available_mem, num_heaps);
            //dprintf (GTC_LOG, ("h%d, min frag is %I64d", heap_number, min_frag_th));
            ret = (est_frag >= min_frag_th);
        }
        else
        {
            assert(0);
        }
        break;
    }

    default:
        break;
    }

    return ret;
}

// @@TUNING_FUNC_PER_HEAP GC#0
void gc_heap::check_frag_high_mem(uint32_t memory_load, BOOL low_memory_detected, uint64_t available_physical,
                                  BOOL& high_fragmentation, BOOL& high_memory_load, BOOL& v_high_memory_load)
{
    if (memory_load >= high_memory_load_th || low_memory_detected)
    {
#ifdef SIMPLE_DPRINTF
        // stress log can't handle any parameter that's bigger than a void*.
        if (heap_number == 0)
        {
            dprintf(GTC_LOG, ("tp: %I64d, ap: %I64d", total_physical_mem, available_physical));
        }
#endif //SIMPLE_DPRINTF

        high_memory_load = TRUE;

        if (memory_load >= v_high_memory_load_th || low_memory_detected)
        {
            // TODO: Perhaps in 64-bit we should be estimating gen1's fragmentation as well since
            // gen1/gen0 may take a lot more memory than gen2.
            if (!high_fragmentation)
            {
                high_fragmentation = dt_estimate_reclaim_space_p(tuning_deciding_condemned_gen, max_generation);
            }
            v_high_memory_load = TRUE;
        }
        else
        {
            if (!high_fragmentation)
            {
                high_fragmentation = dt_estimate_high_frag_p(tuning_deciding_condemned_gen, max_generation, available_physical);
            }
        }
    }
}

// @@TUNING_FUNC_PER_HEAP GC#1
int gc_heap::condemned_gen_check_frag(int n, BOOL high_fragmentation, BOOL high_memory_load, BOOL v_high_memory_load,
    BOOL* blocking_collection_p)
{
    if (high_fragmentation)
    {
        //elevate to max_generation
        n = max_generation;
        dprintf(GTC_LOG, ("h%d: f full", heap_number));

#ifdef BACKGROUND_GC
        if (high_memory_load || v_high_memory_load)
        {
            // For background GC we want to do blocking collections more eagerly because we don't
            // want to get into the situation where the memory load becomes high while we are in
            // a background GC and we'd have to wait for the background GC to finish to start
            // a blocking collection (right now the implemenation doesn't handle converting
            // a background GC to a blocking collection midway.
            dprintf(GTC_LOG, ("h%d: bgc - BLOCK", heap_number));
            *blocking_collection_p = TRUE;
        }
#else
        if (v_high_memory_load)
        {
            dprintf(GTC_LOG, ("h%d: - BLOCK", heap_number));
            *blocking_collection_p = TRUE;
        }
#endif //BACKGROUND_GC
    }
    else
    {
        n = max(n, max_generation - 1);
        dprintf(GTC_LOG, ("h%d: nf c %d", heap_number, n));
    }

    return n;
}

// @@TUNING_FUNC_PER_HEAP, generation_size(2)
inline
size_t gc_heap::min_reclaim_fragmentation_threshold(uint32_t memory_load, uint32_t num_heaps)
{
    // if the memory load is higher, the threshold we'd want to collect gets lower.
    size_t min_mem_based_on_available =
        (500 - (memory_load - high_memory_load_th) * 40) * 1024 * 1024 / num_heaps;

    size_t ten_percent_size = (size_t)((float)generation_size(max_generation) * 0.10);
    uint64_t three_percent_mem = mem_one_percent * 3 / num_heaps;

#ifdef SIMPLE_DPRINTF
    dprintf(GTC_LOG, ("min av: %Id, 10%% gen2: %Id, 3%% mem: %I64d",
        min_mem_based_on_available, ten_percent_size, three_percent_mem));
#endif //SIMPLE_DPRINTF
    return (size_t)(min(min_mem_based_on_available, min(ten_percent_size, three_percent_mem)));
}

// @@TUNING_FUNC_PER_HEAP
inline
uint64_t gc_heap::min_high_fragmentation_threshold(uint64_t available_mem, uint32_t num_heaps)
{
    return min(available_mem, (256 * 1024 * 1024)) / num_heaps;
}

// @@TUNING_FUNC_PER_HEAP, dd_desired_allocation(2), dd_new_allocation(2), dd_current_size(2), dd_surv(2), dd_fragmentation(2)
size_t gc_heap::estimated_reclaim(int gen_number)
{
    dynamic_data* dd = dynamic_data_of(gen_number);
    size_t gen_allocated = (dd_desired_allocation(dd) - dd_new_allocation(dd));
    size_t gen_total_size = gen_allocated + dd_current_size(dd);
    size_t est_gen_surv = (size_t)((float)(gen_total_size)*dd_surv(dd));
    size_t est_gen_free = gen_total_size - est_gen_surv + dd_fragmentation(dd);

    dprintf(GTC_LOG, ("h%d gen%d total size: %Id, est dead space: %Id (s: %d, allocated: %Id), frag: %Id",
        heap_number, gen_number,
        gen_total_size,
        est_gen_free,
        (int)(dd_surv(dd) * 100),
        gen_allocated,
        dd_fragmentation(dd)));

    return est_gen_free;
}
