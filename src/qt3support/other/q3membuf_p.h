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

#ifndef Q3MEMBUF_P_H
#define Q3MEMBUF_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qbytearray.h>
#include <qlist.h>

class Q_COMPAT_EXPORT Q3Membuf
{
public:
    Q3Membuf();
    ~Q3Membuf();

    void append(QByteArray *ba);
    void clear();

    bool consumeBytes(Q_ULONG nbytes, char *sink);
    QByteArray readAll();
    bool scanNewline(QByteArray *store);
    bool canReadLine() const;

    int ungetch(int ch);

    qint64 size() const;

private:

    QList<QByteArray *> buf;
    qint64 _size;
    qint64 _index;
};

inline void Q3Membuf::append(QByteArray *ba)
{ buf.append(ba); _size += ba->size(); }

inline void Q3Membuf::clear()
{ qDeleteAll(buf); buf.clear(); _size=0; _index=0; }

inline QByteArray Q3Membuf::readAll()
{ QByteArray ba; ba.resize(_size); consumeBytes(_size,ba.data()); return ba; }

inline bool Q3Membuf::canReadLine() const
{ return const_cast<Q3Membuf*>(this)->scanNewline(0); }

inline qint64 Q3Membuf::size() const
{ return _size; }

#endif
