#include <iostream>
#include "parse/driver.hpp"

int main() {
    Driver drv;
    int res = drv.parse();
    if (res != 0) {
        return res;
    }

    for (auto &fnDefNode : drv.fnDefNodes) {
        std::cout << *fnDefNode << '\n';
    }
}
