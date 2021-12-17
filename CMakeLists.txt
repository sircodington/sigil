project(sigil LANGUAGES C CXX)

set(${PROJECT_NAME}_HEADERS
    CharSet.h
    Dfa.h
    DfaScannerDriver.h
    DfaSimulation.h
    DfaTableScannerDriver.h
    Grammar.h
    Nfa.h
    RegExp.h
    RegexParser.h
    ScannerDriver.h
    Specification.h
    StaticTable.h
    StaticTableScannerDriver.h
    test.h
    Types.h
)

set(${PROJECT_NAME}_SOURCES
    CharSet.cpp
    Dfa.cpp
    DfaScannerDriver.cpp
    DfaSimulation.cpp
    DfaTableScannerDriver.cpp
    Grammar.cpp
    Nfa.cpp
    RegExp.cpp
    RegexParser.cpp
    ScannerDriver.cpp
    Specification.cpp
    StaticTable.cpp
    StaticTableScannerDriver.cpp
    test.cpp
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_HEADERS}
    ${${PROJECT_NAME}_SOURCES}
)
target_link_libraries(${PROJECT_NAME}
    core
)
