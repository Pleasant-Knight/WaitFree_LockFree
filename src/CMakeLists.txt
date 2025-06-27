add_library(wfq STATIC
    wf_stack.h
    wCQ_queue.h
    ms_queue_wrapper.h
)

# Add this to expose the headers
target_include_directories(wfq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Add this to expose the headers
target_include_directories(cds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#add_executable(demo main.cpp)
#target_link_libraries(demo PRIVATE wfq cds)  # Assuming libcds is linked as cds
