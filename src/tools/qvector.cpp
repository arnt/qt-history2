#include "qvector.h"
#include "qtools_p.h"
#include <string.h>

QVectorData QVectorData::shared_null = { Q_ATOMIC_INIT(1), 0, 0 };

QVectorData* QVectorData::malloc(int size, int sizeofT)
{
    return (QVectorData*)qMalloc(sizeof(QVectorData) + size * sizeofT);
}

QVectorData* QVectorData::malloc(int size, int sizeofT, QVectorData* init)
{
    QVectorData* p = (QVectorData *)qMalloc(sizeof(QVectorData) + size * sizeofT);
    ::memcpy(p, init, sizeof(QVectorData)+qMin(size, init->alloc)*sizeofT);
    return p;
}

QVectorData* QVectorData::realloc(int size, int sizeofT)
{
    return (QVectorData*)qRealloc(this, sizeof(QVectorData) + size * sizeofT);
}

int QVectorData::grow(int size, int sizeofT, bool excessive)
{
    if (excessive)
 	return size + size/2;
    return qAllocMore(size * sizeofT,
		       sizeof(QVectorData)) / sizeofT;
}
