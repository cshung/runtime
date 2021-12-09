// GeneratedFromGCSrc.cpp is the functions that are marked with // @@TUNING_FUNC_PER_HEAP in gc.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include "Generated.h"


enum FieldType
{
    FT_INT32,
    FT_INT64,
    FT_FLOAT,
};

enum FieldKind
{
    FK_GLOBAL,
    FK_DD_TABLE,
    FK_GEN_TABLE,
    FK_GC_INDEX,
    FK_GEN_INDEX,
    FK_HEAP_INDEX,
};

struct FieldDesc
{
    const char* fieldName;
    FieldType   fieldType;
    FieldKind   fieldKind;
    ptrdiff_t   fieldOffset;
};

struct InputField
{
    dynamic_data dynamic_data_table[MAX_HEAPS][total_generation_count];
    generation generation_table[MAX_HEAPS][total_generation_count];
    gc_mechanisms settings;

    uint32_t memory_load;
    BOOL low_memory_detected;
    uint64_t available_physical;
    BOOL high_fragmentation;
    BOOL high_memory_load;
    BOOL v_high_memory_load;
};

static FieldDesc fieldTable[] =
{
    { "GC",                     FT_INT32,       FK_GC_INDEX,    offsetof(InputField,   settings.gc_index)         },
    { "settings.reason",        FT_INT32,       FK_GLOBAL,      offsetof(InputField,   settings.reason)           },
    { "Gen",                    FT_INT32,       FK_GEN_INDEX,   -1                                                },
    { "Heap",                   FT_INT32,       FK_HEAP_INDEX,  -1                                                },
    { "memory_load",            FT_INT32,       FK_GLOBAL,      offsetof(InputField,   memory_load)               },
    { "low_memory_detected",    FT_INT32,       FK_GLOBAL,      offsetof(InputField,   low_memory_detected)       },
    { "available_physical",     FT_INT64,       FK_GLOBAL,      offsetof(InputField,   available_physical)        },
    { "budget",                 FT_INT64,       FK_DD_TABLE,    offsetof(dynamic_data, desired_allocation)        },
    { "left_in_budget",         FT_INT64,       FK_DD_TABLE,    offsetof(dynamic_data, new_allocation)            },
    { "surv",                   FT_FLOAT,       FK_DD_TABLE,    offsetof(dynamic_data, surv)                      },
    { "fragmentation",          FT_INT64,       FK_DD_TABLE,    offsetof(dynamic_data, fragmentation)             },
    { "survived_size",          FT_INT64,       FK_DD_TABLE,    offsetof(dynamic_data, survived_size)             },
    { "free_list_space",        FT_INT64,       FK_GEN_TABLE,   offsetof(generation,   free_list_space)           },
    { "free_obj_space",         FT_INT64,       FK_GEN_TABLE,   offsetof(generation,   free_obj_space)            },
    { "free_list_allocated",    FT_INT64,       FK_GEN_TABLE,   offsetof(generation,   free_list_allocated)       },
};

static const int MAX_GC_NUMBER = 32 * 1024;
static InputField inputTable[MAX_GC_NUMBER];

dynamic_data dynamic_data_table[MAX_HEAPS][total_generation_count];
generation generation_table[MAX_HEAPS][total_generation_count];
gc_mechanisms settings;

uint32_t memory_load;
BOOL low_memory_detected;
uint64_t available_physical;
BOOL high_fragmentation;
BOOL high_memory_load; 
BOOL v_high_memory_load;

BOOL blocking_collection_p; 

int conserve_mem_setting;
int g_low_memory_status;
int generation_skip_ratio;
int gc_heap::generation_skip_ratio_threshold;
size_t total_ephemeral_size;
size_t soh_segment_size;
size_t segment_info_size;
bool gc_can_use_concurrent;
struct gc_history_global gc_heap::gc_data_global;
int gc_heap::last_gc_before_oom;
int gc_heap::should_expand_in_full_gc;
bool gc_heap::provisional_mode_triggered;
size_t gc_heap::heap_hard_limit;
size_t  gc_heap::current_total_committed;
int latency_level = latency_level_balanced;
gc_heap* pGenGCHeap;

char input_buf[1024];

gc_heap* g_heap;
int firstGcIndex = -1;
int lastGcIndex = -1;

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

static bool ParseInputFile(FILE* in)
{
    int gcIndex = 0;
    int genIndex = 0;
    int heapIndex = 0;

    while (true)
    {
        if (fgets(input_buf, sizeof(input_buf), in) == nullptr)
            break;
        char* colon = strchr(input_buf, ':');
        if (colon == nullptr)
        {
            printf("<fieldname>:<value> expected\n");
            return false;
        }
        char* valueString = colon + 1;
        char* fieldNameEnd = colon - 1;
        while (fieldNameEnd > input_buf && isspace(*fieldNameEnd))
        {
            fieldNameEnd--;
        }
        fieldNameEnd[1] = '\0';
        char* fieldNameStart = input_buf;
        while (isspace(*fieldNameStart))
        {
            fieldNameStart++;
        }
        char* endValue = nullptr;
        long long value = strtoll(valueString, &endValue, 10);
        bool found = false;
        for (int i = 0; i < _countof(fieldTable); i++)
        {
            if (strcmp(fieldNameStart, fieldTable[i].fieldName) != 0)
                continue;

            found = true;
            ptrdiff_t fieldOffset = fieldTable[i].fieldOffset;
            char* fieldAddress = nullptr;
            switch (fieldTable[i].fieldKind)
            {
            case    FK_GC_INDEX:
                gcIndex = value;
                if (firstGcIndex == -1)
                    firstGcIndex = gcIndex;
                lastGcIndex = gcIndex;
                // fall thru
            case    FK_GLOBAL:
                fieldAddress = ((char*)&inputTable[gcIndex]) + fieldOffset;
                break;

            case    FK_GEN_INDEX:
                genIndex = value;
                break;

            case    FK_HEAP_INDEX:
                if (0 <= value && value < MAX_HEAPS)
                    heapIndex = value;
                else
                    printf("Heap index %Id out of range", value);
                break;

            case    FK_DD_TABLE:
                fieldAddress = ((char*)&inputTable[gcIndex].dynamic_data_table[heapIndex][genIndex]) + fieldOffset;
                break;

            case    FK_GEN_TABLE:
                fieldAddress = ((char*)&inputTable[gcIndex].generation_table[heapIndex][genIndex]) + fieldOffset;
                break;
            }
            if (fieldAddress != nullptr)
            {
                switch (fieldTable[i].fieldType)
                {
                case    FT_FLOAT:
                    *(float*)fieldAddress = strtof(valueString, &endValue);
                    break;

                case    FT_INT32:
                    *(long*)fieldAddress = value;
                    break;

                case    FT_INT64:
                    *(long long*)fieldAddress = value;
                    break;
                }
            }
            break;
        }
        if (!found)
        {
            printf("Field name not recognized: '%s'\n", fieldNameStart);
            return false;
        }
    }
    return true;
}

size_t gc_heap::generation_size(int gen_number)
{
    dynamic_data* gen = dynamic_data_of(gen_number);
    return dd_current_size(gen) + dd_fragmentation(gen);
}

generation* gc_heap::generation_of(int gen_number)
{
    return &generation_table[heap_number][gen_number];
}

dynamic_data* gc_heap::dynamic_data_of(int gen_number)
{
    return &dynamic_data_table[heap_number][gen_number];
}

size_t gc_heap::get_total_gen_size(int gen_number)
{
    return g_heap->generation_size(gen_number);
}

gc_history_per_heap* gc_heap::get_gc_data_per_heap()
{
#ifdef BACKGROUND_GC
    return (settings.concurrent ? &bgc_data_per_heap : &gc_data_per_heap);
#else
    return &gc_data_per_heap;
#endif //BACKGROUND_GC
}

// This is for methods that need to iterate through all SOH heap segments/regions.
inline
int get_start_generation_index()
{
#ifdef USE_REGIONS
    return 0;
#else
    return max_generation;
#endif //USE_REGIONS
}

inline
int get_stop_generation_index(int condemned_gen_number)
{
#ifdef USE_REGIONS
    return 0;
#else
    return condemned_gen_number;
#endif //USE_REGIONS
}

size_t gc_heap::committed_size()
{
    size_t total_committed = 0;

    for (int i = get_start_generation_index(); i < total_generation_count; i++)
    {
        dynamic_data* dd = dynamic_data_of(i);
        total_committed += dd_begin_data_size(dd) + dd_fragmentation(dd);
    }

#ifdef USE_REGIONS
    total_committed += committed_in_free;
#endif //USE_REGIO

    return total_committed;
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
    dynamic_data* dd_maxgen = g_heap->dynamic_data_of(gen_number);
    printf("budget, left in budget, current size, surv (in %%), frag\n");
    fgets(input_buf, sizeof(input_buf), stdin);
    char* str = input_buf;
    dd_desired_allocation(dd_maxgen) = get_next_long(&str);
    dd_new_allocation(dd_maxgen) = get_next_long(&str);
    dd_current_size(dd_maxgen) = get_next_long(&str);
    dd_surv(dd_maxgen) = (float)get_next_long(&str) / 100.0;
    dd_fragmentation(dd_maxgen) = get_next_long(&str);
    generation_table[gen_number];// = dd_current_size(dd_maxgen) + dd_fragmentation(dd_maxgen);
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

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t)-1)
#endif

#ifndef SSIZE_T_MAX
#define SSIZE_T_MAX ((ptrdiff_t)(SIZE_T_MAX / 2))
#endif

// Things we need to manually initialize:
// gen0 min_size - based on cache
// gen0/1 max_size - based on segment size
static static_data static_data_table[latency_level_last - latency_level_first + 1][total_generation_count] =
{
    // latency_level_memory_footprint
    {
        // gen0
        {0, 0, 40000, 0.5f, 9.0f, 20.0f, (1000 * 1000), 1},
        // gen1
        {160 * 1024, 0, 80000, 0.5f, 2.0f, 7.0f, (10 * 1000 * 1000), 10},
        // gen2
        {256 * 1024, SSIZE_T_MAX, 200000, 0.25f, 1.2f, 1.8f, (100 * 1000 * 1000), 100},
        // loh
        {3 * 1024 * 1024, SSIZE_T_MAX, 0, 0.0f, 1.25f, 4.5f, 0, 0},
        // poh
        {3 * 1024 * 1024, SSIZE_T_MAX, 0, 0.0f, 1.25f, 4.5f, 0, 0},
    },

    // latency_level_balanced
    {
        // gen0
        {0, 0, 40000, 0.5f,
#ifdef MULTIPLE_HEAPS
            20.0f, 40.0f,
#else
            9.0f, 20.0f,
#endif //MULTIPLE_HEAPS
            (1000 * 1000), 1},
            // gen1
            {256 * 1024, 0, 80000, 0.5f, 2.0f, 7.0f, (10 * 1000 * 1000), 10},
            // gen2
            {256 * 1024, SSIZE_T_MAX, 200000, 0.25f, 1.2f, 1.8f, (100 * 1000 * 1000), 100},
            // loh
            {3 * 1024 * 1024, SSIZE_T_MAX, 0, 0.0f, 1.25f, 4.5f, 0, 0},
            // poh
            {3 * 1024 * 1024, SSIZE_T_MAX, 0, 0.0f, 1.25f, 4.5f, 0, 0}
        },
};

int main(int argc, char* argv[])
{
    g_heap = new gc_heap();
    pGenGCHeap = g_heap;

    if (argc <= 1)
    {
        run_one_gc();
        run_one_gc();
    }
    else
    {
        FILE* in = nullptr;
        if (fopen_s(&in, argv[1], "r") == 0)
        {
            if (ParseInputFile(in))
            {
                for (int i = firstGcIndex; i <= lastGcIndex; i++)
                {
                    InputField& inputField = inputTable[i];
                    memcpy(dynamic_data_table, inputField.dynamic_data_table, sizeof(dynamic_data_table));
                    memcpy(generation_table, inputField.generation_table, sizeof(generation_table));
                    for (int heap_number = 0; heap_number < MAX_HEAPS; heap_number++)
                    {
                        for (int gen_number = soh_gen0; gen_number < total_generation_count; gen_number++)
                        {
                            dynamic_data_table[heap_number][gen_number].sdata = &static_data_table[latency_level][gen_number];
                        }
                    }

                    settings = inputField.settings;
                    memory_load = inputField.memory_load;
                    low_memory_detected = inputField.low_memory_detected;
                    available_physical = inputField.available_physical;
                    high_fragmentation = inputField.high_fragmentation;
                    high_memory_load = inputField.high_memory_load;
                    v_high_memory_load = inputField.v_high_memory_load;

                    int gen_number = max_generation;

                    BOOL blocking_collection;
                    BOOL elevation_requested;
                    settings.condemned_generation = g_heap->generation_to_condemn(0, &blocking_collection, &elevation_requested, FALSE);

                    settings.condemned_generation = g_heap->joined_generation_to_condemn(elevation_requested, 0, settings.condemned_generation , &blocking_collection);

                    for (int gen = 0; gen <= settings.condemned_generation; gen++)
                    {
                        g_heap->compute_new_dynamic_data(gen);
                    }
                }
            }
            else
            {
                printf("error parsing input file\n");
            }
            fclose(in);
        }
        else
        {
            printf("could not find input file %s\n", argv[1]);
        }
    }
    return 1;
}

