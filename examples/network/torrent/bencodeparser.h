/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BENCODEPARSER_H
#define BENCODEPARSER_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QVariant>

template<typename T> class QList;

typedef QMap<QByteArray,QVariant> Dictionary;
Q_DECLARE_METATYPE(Dictionary)

class BencodeParser
{
public:
    BencodeParser();
    
    bool parse(const QByteArray &content);
    QString errorString() const;

    QMap<QByteArray, QVariant> dictionary() const;
    QByteArray infoSection() const;

private:
    bool getByteString(QByteArray *byteString);
    bool getInteger(qint64 *integer);
    bool getList(QList<QVariant> *list);
    bool getDictionary(QMap<QByteArray, QVariant> *dictionary);

    QMap<QByteArray, QVariant> dictionaryValue;

    QString errString;
    QByteArray content;
    int index;

    int infoStart;
    int infoLength;
};

#endif
