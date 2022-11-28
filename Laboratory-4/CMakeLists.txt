ADD_PACS_EXECUTABLE(TARGET smallpt_thread_pool SOURCES smallpt_thread_pool.cpp)
ADD_PACS_EXECUTABLE(TARGET benchmarks SOURCES main.cpp)
ADD_PACS_EXECUTABLE(TARGET test SOURCES test.cpp)
target_include_directories(smallpt_thread_pool
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(benchmarks
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(test
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")