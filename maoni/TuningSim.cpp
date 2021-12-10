// GeneratedFromGCSrc.cpp is the functions that are marked with // @@TUNING_FUNC_PER_HEAP in gc.cpp
//

#include <windows.h>
#include <stdio.h>

#include "Generated.h"

dynamic_data dynamic_data_table[total_generation_count];
size_t generation_table[total_generation_count];
gc_mechanisms settings;

uint32_t memory_load;
BOOL low_memory_detected;
uint64_t available_physical;
BOOL high_fragmentation;
BOOL high_memory_load; 
BOOL v_high_memory_load;

BOOL blocking_collection_p; 
char input_buf[1024];

gc_heap* g_heap;

// Assumes the string is in this format -
// number,number,number
long get_next_long(char** str)
{
    char* start = *str;
    char next_char;
    while (((next_char = *start) != '\0') && (next_char != '\n'))
    {
        if (next_char == ',')
        {
            start++;
        }
        else
        {
            char* next;
            long l = strtol(start, &next, 10);
            *str = next;
            return l;
        }
    }

    printf("no number to parse!\n");
    return -1;
}

void run_one_gc()
{
    // Reinit relevant fields
    settings.condemned_generation = 0;
    blocking_collection_p = FALSE;
    high_fragmentation = FALSE;
    high_memory_load = FALSE;
    v_high_memory_load = FALSE;

    settings.gc_index++;
    printf("GC#[%10Id]\n", settings.gc_index);

    // Calling GC#0
    printf("memory_load: ");
    fgets(input_buf, sizeof(input_buf), stdin);
    memory_load = atoi(input_buf);
    settings.entry_memory_load = memory_load;
    
    printf("low_memory_detected: ");
    fgets(input_buf, sizeof(input_buf), stdin);
    low_memory_detected = atoi(input_buf);
    
    printf("available_physical: ");
    fgets(input_buf, sizeof(input_buf), stdin);
    available_physical = atoi(input_buf);
    
    printf("gen2 data, , separated\n");
    int gen_number = max_generation;
    dynamic_data* dd_maxgen = dynamic_data_of(gen_number);
    printf("budget, left in budget, current size, surv (in %%), frag\n");
    fgets(input_buf, sizeof(input_buf), stdin);
    char* str = input_buf;
    dd_desired_allocation(dd_maxgen) = get_next_long(&str);
    dd_new_allocation(dd_maxgen) = get_next_long(&str);
    dd_current_size(dd_maxgen) = get_next_long(&str);
    dd_surv(dd_maxgen) = (float)get_next_long(&str) / 100.0;
    dd_fragmentation(dd_maxgen) = get_next_long(&str);
    generation_table[gen_number] = dd_current_size(dd_maxgen) + dd_fragmentation(dd_maxgen);
    printf("memory_load: %d, low_memory_detected: %d, available_physical: %I64d\n",
        memory_load, low_memory_detected, available_physical);
    printf("gen2 budget %Id, left in budget %Id, current size %Id, surv %.3f, frag %Id\n",
        dd_new_allocation(dd_maxgen),
        dd_new_allocation(dd_maxgen),
        dd_current_size(dd_maxgen),
        dd_surv(dd_maxgen),
        dd_fragmentation(dd_maxgen));
    g_heap->check_frag_high_mem(memory_load, low_memory_detected, available_physical, high_fragmentation, high_memory_load, v_high_memory_load);

    // calling GC#1
    settings.condemned_generation = g_heap->condemned_gen_check_frag(settings.condemned_generation, high_fragmentation, high_memory_load, v_high_memory_load, 
        &blocking_collection_p);

    printf("GC#[%10Id]: condemned gen %d, blocking %s\n", settings.gc_index, settings.condemned_generation,
        ((settings.condemned_generation == max_generation) ? (blocking_collection_p ? "Y" : "N") : "Y"));
}

int main()
{
    g_heap = new gc_heap();

    run_one_gc();
    run_one_gc();
    return 1;
}

