#include <iostream>
#include <streambuf>
#include <sstream>
#include "util.hpp"

IndentedStreamBuffer::IndentedStreamBuffer(
        std::streambuf* buf, int indentWidth
    ) : buf(buf), indentWidth(indentWidth), shouldIndent(true) {}

int IndentedStreamBuffer::overflow(int c) {
    if (c == '\n') {
        shouldIndent = true;
    } else if (shouldIndent) {
        for (int i = 0; i < indentWidth; ++i) {
            if (buf->sputc(' ') == std::char_traits<char>::eof()) {
                return std::char_traits<char>::eof();
            }
        }
        shouldIndent = false;
    }
    return buf->sputc(c);
}


IndentedStream::IndentedStream(std::ostream& os, int indentWidth)
    : std::ostream(&buffer), buffer(os.rdbuf(), indentWidth) {}

unsigned long util::log2(unsigned long size) {
    switch (size) {
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        case 16: return 4;
        case 32: return 5;
        case 64: return 6;
        case 128: return 7;
        case 256: return 8;
    }

    unsigned long i = 0;
    while (size > 1) {
        size = size >> 1;
        i++;
    }
    return i;
}
