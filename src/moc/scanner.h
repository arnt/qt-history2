#ifndef SCANNER_H
#define SCANNER_H

#include "symbols.h"

class Scanner {
public:
    static Symbols scan(const QByteArray &input);
    static Symbols scan2(const QByteArray &input);
};

#endif // SCANNER_H
