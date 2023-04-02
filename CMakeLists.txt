cmake_minimum_required(VERSION 3.20)
project(sigil LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wno-literal-suffix)
add_compile_options(-Wswitch)

set(${PROJECT_NAME}_HEADERS
    include/sigil/CharSet.h
    include/sigil/Dfa.h
    include/sigil/DfaScannerDriver.h
    include/sigil/DfaSimulation.h
    include/sigil/DfaTableScannerDriver.h
    include/sigil/FilePosition.h
    include/sigil/FileRange.h
    include/sigil/Grammar.h
    include/sigil/Nfa.h
    include/sigil/RegExp.h
    include/sigil/RegexParser.h
    include/sigil/ScannerDriver.h
    include/sigil/SpecialTokenType.h
    include/sigil/Specification.h
    include/sigil/StaticTable.h
    include/sigil/StaticTableScannerDriver.h
    include/sigil/Token.h
    include/sigil/Types.h
)

set(${PROJECT_NAME}_SOURCES
    src/CharSet.cpp
    src/Dfa.cpp
    src/DfaScannerDriver.cpp
    src/DfaSimulation.cpp
    src/DfaTableScannerDriver.cpp
    src/FileRange.cpp
    src/Grammar.cpp
    src/Nfa.cpp
    src/RegExp.cpp
    src/RegexParser.cpp
    src/ScannerDriver.cpp
    src/Specification.cpp
    src/StaticTable.cpp
    src/StaticTableScannerDriver.cpp
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_HEADERS}
    ${${PROJECT_NAME}_SOURCES}
)

add_subdirectory(libraries)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME}
    core
)

add_executable(${PROJECT_NAME}-test
    test/main.cpp
)
target_include_directories(${PROJECT_NAME}-test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME}-test
    core
    ${PROJECT_NAME}
)
