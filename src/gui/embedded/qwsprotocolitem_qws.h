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

#ifndef QWSPROTOCOLITEM_QWS_H
#define QWSPROTOCOLITEM_QWS_H

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/

struct QWSProtocolItem
{
    // ctor - dtor
    QWSProtocolItem(int t, int len, char *ptr) : type(t),
        simpleLen(len), rawLen(-1), deleteRaw(false), simpleDataPtr(ptr),
        rawDataPtr(0), bytesRead(0) { }
    virtual ~QWSProtocolItem();

    // data
    int type;
    int simpleLen;
    int rawLen;
    bool deleteRaw;

    // functions
#ifndef QT_NO_QWS_MULTIPROCESS
    void write(QIODevice *s);
    bool read(QIODevice *s);
#endif
    void copyFrom(const QWSProtocolItem *item);

    virtual void setData(const char *data, int len, bool allocateMem = true);

    char *simpleDataPtr;
    char *rawDataPtr;
    // temp variables
    int bytesRead;
};

#endif // QWSPROTOCOLITEM_QWS_H
