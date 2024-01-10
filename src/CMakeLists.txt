set(QCC_SOURCES
    qcc.cpp
    util.cpp
    CompileState.cpp
    StackFrame.cpp
    Reservation.cpp
    ast/ast.cpp
    ast/FnDeclNode.cpp
    ast/FnDefNode.cpp
    ast/TypeNode.cpp
    ast/ParamNode.cpp
    ast/StatementNode.cpp
    ast/IfNode.cpp
    ast/WhileNode.cpp
    ast/FnCallNode.cpp
    ast/ExprNode.cpp
    ast/LiteralNode.cpp
    parse/driver.cpp)


find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(PARSE_OUTPUT "${CMAKE_BINARY_DIR}/parse")
file(MAKE_DIRECTORY "${PARSE_OUTPUT}")

flex_target(QCCLexer parse/qcc.l "${PARSE_OUTPUT}/qcc.l.cpp")
bison_target(QCCParser parse/qcc.y "${PARSE_OUTPUT}/qcc.y.cpp"
             DEFINES_FILE "${PARSE_OUTPUT}/qcc.y.hpp"
             COMPILE_FLAGS "-Wcounterexamples")
add_flex_bison_dependency(QCCLexer QCCParser)


add_executable(qcc "${QCC_SOURCES}"
    "${FLEX_QCCLexer_OUTPUTS}" "${BISON_QCCParser_OUTPUTS}")

target_include_directories(qcc PUBLIC .)
target_include_directories(qcc PUBLIC "${PARSE_OUTPUT}")

foreach(SRC IN LISTS QCC_SOURCES)
    set_source_files_properties("${QCC_SOURCES}" PROPERTIES
        COMPILE_FLAGS "-Wall -Wextra -pedantic"
    )
endforeach()

# These are commented out because we don't want warnings for generated files
# target_compile_options(qcc PUBLIC -Wall)
# target_compile_options(qcc PUBLIC -Wextra)
# target_compile_options(qcc PUBLIC -pedantic)
