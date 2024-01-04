#include <iostream>
#include <string>
#include "parse/driver.hpp"
#include "util.hpp"
#include "ast/ast.hpp"
#include "CompileState.hpp"

int main(int argc, char *argv[]) {
    /* SECTION: Parsing */

    Driver drv(argv[0]);
    int res = 0;
    bool parsedSomeFiles = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("-p")) { drv.traceParsing = true; }
        else if (argv[i] == std::string("-s")) { drv.traceScanning = true; }
        else {
            int res = drv.parse(argv[i]);
            parsedSomeFiles = true;
        }
    }
    if (!parsedSomeFiles) { int res = drv.parse("-"); }
    if (res != 0 ) { return res; }
    if (drv.res != 0 ) { return drv.res; }

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
