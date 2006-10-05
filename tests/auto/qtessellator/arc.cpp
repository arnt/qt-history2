#include "arc.h"

#include "dataparser.h"
#include <QString>

FullData arcData()
{
    return parseFile(":/arc.data");
}
