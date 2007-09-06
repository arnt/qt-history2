/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#ifndef QSourceLocation_h
#define QSourceLocation_h

#include <QtCore/QUrl>

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QSourceLocationPrivate;

class Q_XML_EXPORT QSourceLocation
{
public:
    QSourceLocation();
    QSourceLocation(const QSourceLocation &other);
    QSourceLocation(const QUrl &uri, int line, int column);
    ~QSourceLocation();
    QSourceLocation &operator=(const QSourceLocation &other);
    bool operator==(const QSourceLocation &other) const;
    bool operator!=(const QSourceLocation &other) const;

    qint64 column() const;
    void setColumn(qint64 newColumn);

    qint64 line() const;
    void setLine(qint64 newLine);

    QUrl uri() const;
    void setUri(const QUrl &newUri);
    bool isNull() const;

private:
    friend QDebug &operator<<(QDebug debug, const QSourceLocation &sourceLocation);
    union
    {
        qint64 m_line;
        QSourceLocationPrivate *m_ptr;
    };
    qint64 m_column;
    QUrl m_uri;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug &operator<<(QDebug debug, const QSourceLocation &sourceLocation);
#endif

Q_DECLARE_TYPEINFO(QSourceLocation, Q_MOVABLE_TYPE);

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
