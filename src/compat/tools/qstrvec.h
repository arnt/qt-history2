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
    Item         newItem(Item d)        { return dc ? qstrdup(static_cast<char*>(d)) : d; }
    void deleteItem(Item d)        { if (dc) delete[] static_cast<char*>(d); }
    int         compareItems(Item s1, Item s2)
                                { return qstrcmp(static_cast<char*>(s1),
                                                 static_cast<char*>(s2)); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read(QDataStream &s, Item &d)
                                { s >> reinterpret_cast<char *&>(d); return s; }
    QDataStream &write(QDataStream &s, Item d) const
                                { return s << static_cast<char*>(d); }
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
                                { return qstricmp(static_cast<char*>(s1),
                                                  static_cast<char*>(s2)); }
};


#endif // QSTRVEC_H
