/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstringmatcher.h"
#include "qunicodetables_p.h"

static void bm_init_skiptable(const QChar *uc, int l, uint *skiptable, Qt::CaseSensitivity cs)
{
    int i = 0;
    register uint *st = skiptable;
    while (i++ < 256 / 8) {
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
    }
    if (cs == Qt::CaseSensitive) {
        while (l--) {
            skiptable[uc->cell()] = l;
            uc++;
        }
    } else {
        while (l--) {
            skiptable[::lower(*uc).cell()] = l;
            uc++;
        }
    }
}

static inline int bm_find(const QChar *uc, uint l, int index, const QChar *puc, uint pl,
                          const uint *skiptable, Qt::CaseSensitivity cs)
{
    if (pl == 0)
        return index > (int)l ? -1 : index;
    const uint pl_minus_one = pl - 1;

    register const QChar *current = uc + index + pl_minus_one;
    const QChar *end = uc + l;
    if (cs == Qt::CaseSensitive) {
        while (current < end) {
            uint skip = skiptable[current->cell()];
            if (!skip) {
                // possible match
                while (skip < pl) {
                    if (*(current - skip) != puc[pl_minus_one-skip])
                        break;
                    skip++;
                }
                if (skip > pl_minus_one) // we have a match
                    return (current - uc) - skip + 1;

                // in case we don't have a match we are a bit inefficient as we only skip by one
                // when we have the non matching char in the string.
                if (skiptable[(current - skip)->cell()] == pl)
                    skip = pl - skip;
                else
                    skip = 1;
            }
            if (current > end - skip)
                break;
            current += skip;
        }
    } else {
        while (current < end) {
            uint skip = skiptable[::lower(*current).cell()];
            if (!skip) {
                // possible match
                while (skip < pl) {
                    if (::lower(*(current - skip)) != ::lower(puc[pl_minus_one-skip]))
                        break;
                    skip++;
                }
                if (skip > pl_minus_one) // we have a match
                    return (current - uc) - skip + 1;
                // in case we don't have a match we are a bit inefficient as we only skip by one
                // when we have the non matching char in the string.
                if (skiptable[::lower(*(current - skip)).cell()] == pl)
                    skip = pl - skip;
                else
                    skip = 1;
            }
            if (current > end - skip)
                break;
            current += skip;
        }
    }
    return -1; // not found
}

QStringMatcher::QStringMatcher()
    : d_ptr(0)
{
    qMemSet(q_skiptable, 0, sizeof(q_skiptable));
}

QStringMatcher::QStringMatcher(const QString &pattern, Qt::CaseSensitivity cs)
    : d_ptr(0), q_pattern(pattern), q_cs(cs)
{
    bm_init_skiptable(pattern.unicode(), pattern.size(), q_skiptable, cs);
}

QStringMatcher::QStringMatcher(const QStringMatcher &other)
    : d_ptr(0)
{
    operator=(other);
}

QStringMatcher::~QStringMatcher()
{
}

QStringMatcher &QStringMatcher::operator=(const QStringMatcher &other)
{
    q_pattern = other.q_pattern;
    q_cs = other.q_cs;
    qMemCopy(q_skiptable, other.q_skiptable, sizeof(q_skiptable));
    return *this;
}

void QStringMatcher::setPattern(const QString &pattern)
{
    bm_init_skiptable(pattern.unicode(), pattern.size(), q_skiptable, q_cs);
    q_pattern = pattern;
}

void QStringMatcher::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    if (cs == q_cs)
        return;
    bm_init_skiptable(q_pattern.unicode(), q_pattern.size(), q_skiptable, cs);
    q_cs = cs;
}

int QStringMatcher::indexIn(const QString &str, int from) const
{
    // ### what if (from < 1)
    return bm_find(str.unicode(), str.size(), from, q_pattern.unicode(), q_pattern.size(),
                   q_skiptable, q_cs);
}
