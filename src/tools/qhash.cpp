#include "qhash.h"

#include <qbytearray.h>
#include <qstring.h>

/*
    These functions are based on Peter J. Weinberger's hash function
    (from the Dragon Book). The constant 24 in the original function
    was replaced with 23 to produce fewer collisions on input such as
    "a", "aa", "aaa", "aaaa", ...
*/

ulong qHash(const QByteArray& key)
{
    const uchar *p = (const uchar *) key.data();
    int n = key.size();
    uint h = 0;
    uint g;

    while (n--) {
	h = (h << 4) + *p++;
	if ((g = (h & 0xf0000000)) != 0)
	    h ^= g >> 23;
	h &= ~g;
    }
    return h;
}

ulong qHash(const QString& key)
{
    const QChar *p = key.unicode();
    int n = key.length();
    uint h = 0;
    uint g;

    while (n--) {
	h = (h << 4) + (*p++).unicode();
	if ((g = (h & 0xf0000000)) != 0)
	    h ^= g >> 23;
	h &= ~g;
    }
    return h;
}

/*
    The prime_deltas array is a table of selected prime values, even
    though it doesn't look like one. The primes we are using are 1,
    2, 5, 11, 17, 37, 67, 131, 257, ..., i.e. primes in the immediate
    surrounding of a power of two.

    The primeForNumBits() function returns the prime associated to a
    power of two. For example, primeForNumBits(8) returns 257.
*/

static const uchar prime_deltas[] = {
    0,  0,  1,  3,  1,  5,  3,  3,  1,  9,  7,  5,  3,  9, 25,  3,
    1, 21,  3, 21,  7, 15,  9,  5,  3, 29, 15
};

static inline int primeForNumBits(int numBits)
{
    return (1 << numBits) + prime_deltas[numBits];
}

/*
    Returns the smallest integer n such that
    primeForNumBits(n) >= hint.
*/
static int countBits(int hint)
{
    int numBits = 0;
    int bits = hint;

    while (bits > 1) {
	bits >>= 1;
	numBits++;
    }

    if (numBits >= (int) sizeof(prime_deltas)) {
	numBits = sizeof(prime_deltas) - 1;
    } else if (primeForNumBits(numBits) < hint) {
	numBits++;
    }
    return numBits;
}

/*
    A QHash has initially around pow(2, MinNumBits) buckets. If
    MinNumBits is 4, it has 17 buckets.
*/
const int MinNumBits = 4;

QHashData QHashData::shared_null = { 0, 0, Q_ATOMIC_INIT(1), 0, MinNumBits, 0, 0, 0 };

QHashData *QHashData::detach_helper(Node *(*node_duplicate)(Node *))
{
    union {
	QHashData *d;
	Node *e;
    };
    d = new QHashData;
    d->fakeNext = 0;
    d->buckets = 0;
    d->ref = 1;
    d->size = size;
    d->userNumBits = userNumBits;
    d->numBits = numBits;
    d->numBuckets = numBuckets;
    d->autoDelete = autoDelete;

    if (numBuckets) {
	d->buckets = new Node *[numBuckets];
	Node *this_e = (Node *) this;
	for (int i = 0; i < numBuckets; ++i) {
	    d->buckets[i] = e;
	    Node *cur = buckets[i];
	    while (cur != this_e) {
		Node *dup = node_duplicate(cur);
		dup->h = cur->h;
		    Node **bucket = &d->buckets[i];
		    dup->next = *bucket;
		    *bucket = dup;
		    cur = cur->next;
	    }
	}
    }
    return d;
}

void QHashData::rehash(int hint)
{
    if (hint < 0) {
	hint = countBits(-hint);
	if (hint < MinNumBits)
	    hint = MinNumBits;
	userNumBits = hint;
	if (size > (1 << userNumBits))
	    return;
    } else if (hint == 1) {
	hint = MinNumBits;
    }

    if (numBits != hint) {
	Node *e = (Node *) this;
	Node **oldBuckets = buckets;
	int oldNumBuckets = numBuckets;

	numBits = hint;
	numBuckets = primeForNumBits(hint);
	buckets = new Node *[numBuckets];
	for (int i = 0; i < numBuckets; i++)
	    buckets[i] = e;

	for (int i = 0; i < oldNumBuckets; i++) {
	    Node *node = oldBuckets[i];
	    while (node != e) {
		Node *oldNext = node->next;
		Node **nextNode_ptr = &buckets[node->h % numBuckets];
		while (*nextNode_ptr != e)
		    nextNode_ptr = &(*nextNode_ptr)->next;
		node->next = *nextNode_ptr;
		*nextNode_ptr = node;
		node = oldNext;
	    }
	}
	delete [] oldBuckets;
    }
}
