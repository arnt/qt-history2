#ifndef GENPORTRULES_H
#define GENPORTRULES_H

#include <QString>


struct SymbolRename
{
    QString from;
    QString to;
    bool operator==(const SymbolRename &other)
    {
        return (other.from == from && other.to == to);
    }
};

#endif

