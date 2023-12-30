#include <iostream>
#include "parse/driver.hpp"
#include "util.hpp"
#include "ast.hpp"
#include "CompileState.hpp"

int main() {
    /* SECTION: Parsing */

    Driver drv;
    int res = drv.parse();
    if (res != 0) {
        return res;
    }

    for (auto *fnDefNode : drv.fnDefNodes) {
        std::cerr << *fnDefNode << '\n';
    }

    /* SECTION: Emission */

    std::cerr << "\n=== OUTPUT ===\n";

    CompileState cs(std::cout);
    std::ostream &os = cs.os;
    IndentedStream ios(os, cs.indent);

    ios << ".globl _main\n";
    ios << ".text\n";
    ios << ".align 4\n\n";

    for (auto *fnDefNode : drv.fnDefNodes) {
        fnDefNode->emit(cs);
    }
}
