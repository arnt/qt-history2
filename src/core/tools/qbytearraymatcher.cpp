#include "qbytearraymatcher.h"

static inline void bm_init_skiptable(const uchar *cc, int l, uint *skiptable)
{
    int i = 0;
    register uint *st = skiptable;
    while (i++ < 256 / 8) {
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
    }
    while (l--)
        skiptable[*cc++] = l;
}

static inline int bm_find(const uchar *cc, uint l, int index, const uchar *puc, uint pl,
                          const uint *skiptable)
{
    if (pl == 0)
        return index > (int)l ? -1 : index;
    const uint pl_minus_one = pl - 1;

    register const uchar *current = cc + index + pl_minus_one;
    const uchar *end = cc + l;
    while (current < end) {
        uint skip = skiptable[*current];
        if (!skip) {
            // possible match
            while (skip < pl) {
                if (*(current - skip) != puc[pl_minus_one - skip])
                    break;
                skip++;
            }
            if (skip > pl_minus_one) // we have a match
                return (current - cc) - skip + 1;

            // in case we don't have a match we are a bit inefficient as we only skip by one
            // when we have the non matching char in the string.
            if (skiptable[*(current - skip)] == pl)
                skip = pl - skip;
            else
                skip = 1;
        }
        current += skip;
    }
    return -1; // not found
}

/*! \class QByteArrayMatcher
    \brief The QByteArrayMatcher class ...
*/

/*!

*/
QByteArrayMatcher::QByteArrayMatcher()
    : d(0)
{
    qMemSet(q_skiptable, 0, sizeof(q_skiptable));
}

/*!

*/
QByteArrayMatcher::QByteArrayMatcher(const QByteArray &pattern)
    : d(0)
{
    setPattern(pattern);
}

/*!

*/
QByteArrayMatcher::QByteArrayMatcher(const QByteArrayMatcher &other)
    : d(0)
{
    operator=(other);
}

/*!

*/
QByteArrayMatcher::~QByteArrayMatcher()
{
}

/*!

*/
QByteArrayMatcher &QByteArrayMatcher::operator=(const QByteArrayMatcher &other)
{
    q_pattern = other.q_pattern;
    qMemCopy(q_skiptable, other.q_skiptable, sizeof(q_skiptable));
    return *this;
}

/*!

*/
void QByteArrayMatcher::setPattern(const QByteArray &pattern)
{
    bm_init_skiptable(reinterpret_cast<const uchar *>(pattern.constData()), pattern.size(),
                      q_skiptable);
    q_pattern = pattern;
}

/*!

*/
int QByteArrayMatcher::indexIn(const QByteArray &ba, int from) const
{
    // ### what if (from < 1)
    return bm_find(reinterpret_cast<const uchar *>(ba.constData()), ba.size(), from,
                   reinterpret_cast<const uchar *>(q_pattern.constData()), q_pattern.size(),
                   q_skiptable);
}
