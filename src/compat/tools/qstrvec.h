/****************************************************************************
**
** Definition of QStrVec and QStrIVec classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTRVEC_H
#define QSTRVEC_H

#ifndef QT_H
#include "qstring.h"
#include "qptrvector.h"
#include "qdatastream.h"
#endif // QT_H

class Q_COMPAT_EXPORT QStrVec : public QPtrVector<char>
{
public:
    QStrVec()  { dc = true; }
    QStrVec(uint size, bool deepc = true) : QPtrVector<char>(size) {dc=deepc;}
   ~QStrVec()  { clear(); }
private:
    Item         newItem(Item d)        { return dc ? qstrdup((const char*)d) : d; }
    void deleteItem(Item d)        { if (dc) delete[] (char*)d; }
    int         compareItems(Item s1, Item s2)
                                { return qstrcmp((const char*)s1,
                                                (const char*)s2); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read(QDataStream &s, Item &d)
                                { s >> (char *&)d; return s; }
    QDataStream &write(QDataStream &s, Item d) const
                                { return s << (const char*)d; }
#endif
    bool dc;
};


class Q_COMPAT_EXPORT QStrIVec : public QStrVec        // case insensitive string vec
{
public:
    QStrIVec() {}
    QStrIVec(uint size, bool dc = true) : QStrVec(size, dc) {}
   ~QStrIVec() { clear(); }
private:
    int         compareItems(Item s1, Item s2)
                                { return qstricmp((const char*)s1,
                                                 (const char*)s2); }
};


#endif // QSTRVEC_H
