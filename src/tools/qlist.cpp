#include "qlist.h"
#include "qtools_p.h"
#include <string.h>


/*!

Technicalities

QList as an array-list combines the easy-of-use of a random access
interface with fast list operations and the low memory management
overhead of an array. Accessing elements by index, appending,
prepending and removing elements from both the front and the back all
happen in constant time O(1). Inserting or removing elements at random
index positions \ai happens in linear time, or more precisly in
O(min{i,n-i}) <= O(n/2), with n being the number of elements in the
list.

*/

QListData::Data QListData::shared_null = { Q_ATOMIC_INIT(1), 0, 0, 0, 0, {0} };

int QListData::grow (int size)
{
    return qAllocMore(size * sizeof(void*), sizeof(DataHeader)) / sizeof(void*);
}

QListData::Data *QListData::detach()
{
    Q_ASSERT( d->ref != 1 );
    Data *x = (Data*)qMalloc(sizeof(DataHeader)+d->alloc*sizeof(void*));
    ::memcpy(x, d, sizeof(DataHeader) + d->alloc*sizeof(void*));
    x->alloc = d->alloc;
    if (x->autoDelete != this)
	x->autoDelete = 0;
    x->ref = 1;
    if (!x->alloc)
	x->begin = x->end = 0;

    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	return x;
    return 0;
}

void QListData::realloc(int alloc)
{
    Q_ASSERT( d->ref == 1 );
    d = (Data*)qRealloc(d, sizeof(DataHeader)+alloc*sizeof(void*));
    d->alloc = alloc;
    if (!alloc)
	d->begin = d->end = 0;
}


void **QListData::append()
{
    Q_ASSERT( d->ref == 1 );
    if (d->end + 1 >  d->alloc)
	realloc(grow(d->alloc + 1));
    return d->array + d->end++;
}

void **QListData::append(const QListData& l)
{
    Q_ASSERT( d->ref == 1 );
    int e = d->end;
    int n = l.d->end - l.d->begin;
    if (n) {
	if (e + n >  d->alloc)
	    realloc(grow(e + l.d->end - l.d->begin));
	::memcpy(d->array + d->end, l.d->array + l.d->begin,
		  n * sizeof(void*));
	d->end += n;
    }
    return d->array + e;
}


void **QListData::prepend()
{
    Q_ASSERT( d->ref == 1 );
    if ( d->begin == 0) {
	realloc(grow(d->alloc + 1));
	int offset = d->alloc - d->end;
	::memmove(d->array + d->begin + offset, d->array + d->begin,
		   (d->end-d->begin) * sizeof(void*));
	d->begin += offset;
	d->end += offset;
    }
    return d->array +  --d->begin;
}

void **QListData::insert(int i)
{
    Q_ASSERT( d->ref == 1 );
    if (i <= 0)
	return prepend();
    if (i >= d->end - d->begin)
	return append();
    if ( d->end + 1 >  d->alloc)
	realloc(grow(d->alloc + 1));
    i += d->begin;
    ::memmove(d->array + i + 1, d->array + i,
	       (d->end-i) * sizeof(void*));
    d->end++;
    return d->array + i;
}


void QListData::remove(int i)
{
    Q_ASSERT( d->ref == 1 );
    i += d->begin;
    if (i - d->begin < d->end - i) {
	::memmove(d->array + d->begin + 1,  d->array + d->begin,
		   (i - d->begin) * sizeof(void*));
	d->begin++;
    } else {
	::memmove(d->array + i,  d->array + i + 1,
		   (d->end - i - 1) * sizeof(void*));
	d->end--;
    }
}


void QListData::remove(int i, int n)
{
    Q_ASSERT( d->ref == 1 );
    i += d->begin;
    int middle = i + n/2;
    if (middle - d->begin < d->end - middle) {
	::memmove(d->array + d->begin + n,  d->array + d->begin,
		   (i - d->begin) * sizeof(void*));
	d->begin += n;
    } else {
	::memmove(d->array + i,  d->array + i + n,
		   (d->end - i - n) * sizeof(void*));
	d->end -= n;
    }
}

void **QListData::erase(void **xi)
{
    Q_ASSERT( d->ref == 1 );
    int i = xi - (d->array + d->begin);
    remove(i);
    return d->array + d->begin + i;
}


