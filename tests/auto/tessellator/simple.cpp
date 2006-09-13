#include "simple.h"

#include "dataparser.h"
#include <QString>

FullData simpleData()
{
    return parseFile(":/simple.data");
}
