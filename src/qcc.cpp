#include <iostream>
#include <string>
#include "parse/driver.hpp"
#include "util.hpp"
#include "ast/ast.hpp"
#include "CompileState.hpp"

int main(int argc, char *argv[]) {
    /* SECTION: Parsing */

    CompileState cs(std::cout);
    Driver drv(argv[0], &cs);
    int res = 0;
    bool parsedSomeFiles = false;
    bool debug = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("-p")) { drv.traceParsing = true; }
        else if (argv[i] == std::string("-s")) { drv.traceScanning = true; }
        else if (argv[i] == std::string("-d")) { debug = true; }
        else {
            int res = drv.parse(argv[i]);
            parsedSomeFiles = true;
        }
    }
    if (!parsedSomeFiles) { int res = drv.parse("-"); }
    if (res != 0 ) { return res; }
    if (drv.res != 0 ) { return drv.res; }

    if (debug) {
        for (auto *fnDefNode : drv.fnDefNodes) {
            std::cerr << *fnDefNode << '\n';
        }
    }

    /* SECTION: Emission */
    if (debug) {
        std::cerr << "\n=== OUTPUT ===\n";
    }

    std::ostream &os = cs.os;
    IndentedStream ios(os, cs.indent);

    ios << ".text\n";

    for (auto *fnDefNode : drv.fnDefNodes) {
        fnDefNode->emit(cs);
    }

    for (auto &builtin : cs.usedBuiltinFns) {
        os << BUILTIN_FN_DEFS.at(builtin);
    }

    for (auto *staticData : cs.staticData) {
        staticData->emit(cs);
    }
}
