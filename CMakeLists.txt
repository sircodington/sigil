project(sigil LANGUAGES C CXX)

set(${PROJECT_NAME}_HEADERS
    Specification.h
    Grammar.h
)

set(${PROJECT_NAME}_SOURCES
    Specification.cpp
    Grammar.cpp
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_HEADERS}
    ${${PROJECT_NAME}_SOURCES}
)
target_link_libraries(${PROJECT_NAME}
    core
)
