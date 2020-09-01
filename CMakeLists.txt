cmake_minimum_required(VERSION 2.6)
project(vtu2pvtu)

find_package(OpenMP)

add_executable(vtu2pvtu vtu2pvtu.cpp)

if(OpenMP_CXX_FOUND)
	target_link_libraries(vtu2pvtu PUBLIC OpenMP::OpenMP_CXX)
elseif(OpenMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#enable_testing()
#include(CTest)
#add_subdirectory(Tests)


install(TARGETS vtu2pvtu EXPORT vtu2pvtu-targets RUNTIME DESTINATION bin)
