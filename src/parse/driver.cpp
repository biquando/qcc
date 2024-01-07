#include "driver.hpp"
#include "qcc.y.hpp"
#include "CompileState.hpp"

Driver::Driver(std::string execName, CompileState *cs)
        : execName(execName),
          cs(cs),
          traceParsing(false),
          traceScanning(false) {}

int Driver::parse(const std::string &f) {
    file = f;
    location.initialize(&file);

    scan_begin();
    yy::parser parse(*this);
    parse.set_debug_level(traceParsing);
    int res = parse();
    scan_end();

    return res;
}
