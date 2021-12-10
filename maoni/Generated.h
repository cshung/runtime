// This is generated from GeneratedFromGCSrc.cpp
#ifndef __GENERATED_H__
#define __GENERATED_H__
#include <wtypes.h>
#include <cstdint>
#include "SimBuiltIn.h"

#pragma warning(disable:26812) // C26812	The enum type 'gc_tuning_point' is unscoped.Prefer 'enum class' over 'enum'
#pragma warning(disable:26451) // C26451	Arithmetic overflow : Using operator '*' on a 4 byte value and then casting the result to a 8 byte value.
#pragma warning(disable:4244)  // C4244     '=': conversion from 'double' to 'float', possible loss of data

// These are methods generated from GeneratedFromGCSrc.cpp.
class gc_heap
{
    BOOL dt_estimate_reclaim_space_p(gc_tuning_point tp, int gen_number);
    
    BOOL dt_estimate_high_frag_p(gc_tuning_point tp, int gen_number, uint64_t available_mem);

    size_t min_reclaim_fragmentation_threshold(uint32_t memory_load, uint32_t num_heaps);

    uint64_t min_high_fragmentation_threshold(uint64_t available_mem, uint32_t num_heaps);

    size_t estimated_reclaim(int gen_number);

public:
    gc_heap() {}

    // GC#0
    void check_frag_high_mem(uint32_t memory_load, BOOL low_memory_detected, uint64_t available_physical,
        BOOL& high_fragmentation, BOOL& high_memory_load, BOOL& v_high_memory_load);

    // GC#1
    int condemned_gen_check_frag(int n, BOOL high_fragmentation, BOOL high_memory_load, BOOL v_high_memory_load,
        BOOL* blocking_collection_p);
};
#endif //__GENERATED_H__