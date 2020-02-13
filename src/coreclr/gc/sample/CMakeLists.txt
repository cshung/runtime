if (CLR_CMAKE_TARGET_ARCH_AMD64 AND CLR_CMAKE_TARGET_WIN32)
  set ( SOURCES
    ${SOURCES}
    ../vxsort/isa_detection.cpp
    ../vxsort/do_vxsort_avx2.cpp
    ../vxsort/do_vxsort_avx512.cpp
    ../vxsort/machine_traits.avx2.cpp
    ../vxsort/smallsort/bitonic_sort.AVX2.int64_t.generated.cpp
    ../vxsort/smallsort/bitonic_sort.AVX2.int32_t.generated.cpp
    ../vxsort/smallsort/bitonic_sort.AVX512.int64_t.generated.cpp
    ../vxsort/smallsort/bitonic_sort.AVX512.int32_t.generated.cpp
    ../vxsort/smallsort/avx2_load_mask_tables.cpp
)
endif (CLR_CMAKE_TARGET_ARCH_AMD64 AND CLR_CMAKE_TARGET_WIN32)

add_definitions(-DVERIFY_HEAP)

