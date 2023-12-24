#include "driver.hpp"
#include "qcc.y.hpp"

Driver::Driver() {}

int Driver::parse() {
    scan_begin();
    yy::parser parse(*this);
    int res = parse();
    scan_end();
    return res;
}
