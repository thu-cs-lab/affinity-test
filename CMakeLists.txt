cmake_minimum_required(VERSION 3.18)
project(affinity-test)

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(affinity-test main.cpp) 
target_link_libraries(affinity-test MPI::MPI_C MPI::MPI_CXX OpenMP::OpenMP_CXX)
install(TARGETS affinity-test)

