project(sigil LANGUAGES C CXX)

set(${PROJECT_NAME}_HEADERS
    CharSet.h
    Dfa.h
    DfaSimulation.h
    Grammar.h
    Nfa.h
    RegExp.h
    RegexParser.h
    Specification.h
    test.h
)

set(${PROJECT_NAME}_SOURCES
    CharSet.cpp
    Dfa.cpp
    DfaSimulation.cpp
    Grammar.cpp
    Nfa.cpp
    RegExp.cpp
    RegexParser.cpp
    Specification.cpp
    test.cpp
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_HEADERS}
    ${${PROJECT_NAME}_SOURCES}
)
target_link_libraries(${PROJECT_NAME}
    core
)
