#ifndef PARAGDATA_H
#define PARAGDATA_H

#include "parenmatcher.h"

struct ParagData
{
    enum MarkerType { NoMarker, Error, Brekapoint, Bookmark };

    ParagData() : lastLengthForCompletion( -1 ), marker( NoMarker ) {}
    ParenList parenList;
    int lastLengthForCompletion;
    MarkerType marker;

};

#endif
