# Build a debug or release version
# Debug:
# cmake -DCMAKE_BUILD_TYPE=Debug .
# Release:
# cmake -DCMAKE_BUILD_TYPE=Release .

cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR) # Required for C++11 support

set(VERSION "2.0.0")
project (CRAAM)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED)

# **** Build type ****
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IS_DEBUG TRUE)
endif()

# **** Find packages ****
find_package(OpenMP)
find_package(Boost COMPONENTS unit_test_framework ) 
find_package(Doxygen)
if(${Boost_FOUND} LESS 1)
    message(WARNING "Unit tests (testit) require Boost unit test library and may not compile otherwise." )
endif()

# DEBUG
# Prints all variables (for debugging)
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
# DEBUG
#set(CMAKE_VERBOSE_MAKEFILE on) - just run make 

# **** TURN FEATURES ON AND OFF ****

option (BUILD_TESTS "Build tests (requires Boost)" ON)
option (BUILD_DOCUMENTATION "Build source code documentation" ${DOXYGEN_FOUND})

# **** CONFIGURATION ****

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# use position-independent code
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# use OpenMP if possible
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# set gcc-specific parameters
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9.0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wuninitialized")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic") # -D_GLIBCXX_DEBUG -- incompatible with boost :(
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native")
endif()

# **** PROCESS CONFIGURATION FILE ****

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/craam/config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/craam/config.hpp
)

# **** SOURCE FILES ****
include_directories (. )
# do not report warnings from included libraries
include_directories (SYSTEM include)
set (SRCS 
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/Action.hpp  
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/definitions.hpp  
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/RMDP.hpp  
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/State.hpp  
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/Transition.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/modeltools.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/algorithms/values.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/algorithms/robust_values.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/algorithms/occupancies.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/Simulation.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/Samples.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/craam/ImMDP.hpp)

set (TSTS ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/test/core_tests.hpp 
           ${CMAKE_CURRENT_SOURCE_DIR}/test/implementable_tests.hpp
           ${CMAKE_CURRENT_SOURCE_DIR}/test/simulation_tests.hpp
)
set (DEV ${CMAKE_CURRENT_SOURCE_DIR}/test/dev.cpp)
set (BENCH ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark.cpp)

# **** LIBRARY ****
#add_library (craam STATIC ${SRCS} )

# **** DEVELOPMENT EXECUTABLE ****
add_executable (develop_exe ${DEV})
target_link_libraries(develop_exe)
add_custom_target (develop COMMAND develop_exe
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
                        COMMENT "Running executable")

# **** TEST ****
if (BUILD_TESTS)
    add_executable (unit_tests ${TSTS} )
    target_link_libraries(unit_tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    #cotire(unit_tests)
    add_custom_target (testit   COMMAND unit_tests --detect_memory_leaks --detect_fp_exceptions --color-output -l unit_scope
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
                        COMMENT "Running unit tests")
endif (BUILD_TESTS)

# **** COMMAND-LINE EXECUTABLE ****
add_executable(craam-cli EXCLUDE_FROM_ALL ${BENCH} )

# **** DOCUMENTATION ****
if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Needs Doxygen to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.config.in)
    set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile.config)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    #install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()
