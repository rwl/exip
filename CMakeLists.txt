cmake_minimum_required (VERSION 3.21)
project(exip)

include(GNUInstallDirs) # Defines the ${CMAKE_INSTALL_INCLUDEDIR} variable.

# This is the default install directory for config-file package cmake files
# It is relative to install prefix.
set(exip_INSTALL_CMAKEDIR cmake CACHE PATH "Installation directory for config-file package cmake files")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB LIB_INCLUDE_FILES "./include/*.h")
file(GLOB COMMON_SRC "./src/common/src/*.c")
file(GLOB CONTENTIO_SRC "./src/contentIO/src/*.c")
file(GLOB GRAMMAR_SRC "./src/grammar/src/*.c")
file(GLOB GRAMMARGEN_SRC "./src/grammarGen/src/*.c")
file(GLOB STREAMIO_SRC "./src/streamIO/src/*.c")
file(GLOB STRINGTABLES_SRC "./src/stringTables/src/*.c")
file(GLOB SCHEMAHANDLING_SRC "./utils/schemaHandling/*.c")
file(GLOB SCHEMAHANDLINGOUTPUT_SRC "./utils/schemaHandling/output/*.c")

set(SOURCES ${COMMON_SRC} ${CONTENTIO_SRC} ${GRAMMAR_SRC} ${GRAMMARGEN_SRC} 
    ${STREAMIO_SRC} ${STRINGTABLES_SRC} ${SCHEMAHANDLING_SRC}
    ${SCHEMAHANDLINGOUTPUT_SRC}
    )

add_library(exip STATIC ${SOURCES})

include_directories("./include")
include_directories("./src/common/include")
include_directories("./src/contentIO/include")
include_directories("./src/grammar/include")
include_directories("./src/grammarGen/include")
include_directories("./src/streamIO/include")
include_directories("./src/stringTables/include")
include_directories("./utils/schemaHandling/include")
include_directories("./build/gcc/pc")
list(APPEND LIB_INCLUDE_FILES "./build/gcc/pc/exipConfig.h")

if (WIN32)
    target_compile_definitions(exip PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(exip
          PUBLIC 
          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
          "$<INSTALL_INTERFACE:include>"
          )
          
set_target_properties(exip PROPERTIES POSITION_INDEPENDENT_CODE ON)

# install the target and create export-set
install(TARGETS exip 
    EXPORT exip_export   
    INCLUDES DESTINATION include
)

# install header file
install(FILES ${LIB_INCLUDE_FILES} DESTINATION include)

# generate and install export file
install(EXPORT exip_export
        FILE exip-config.cmake
        DESTINATION ${exip_INSTALL_CMAKEDIR}
)
