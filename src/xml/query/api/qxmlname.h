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

#ifndef QXMLNAME_H
#define QXMLNAME_H

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Xml)

class QXmlName;
class QXmlNamePrivate;
class QXmlQuery;
static inline uint qHash(const QXmlName &name);

/* This namespace is not public API. */
namespace QPrivateDetails
{
    /* These cannot be enums because they are too large. */
    static const quint64 InvalidCode = Q_UINT64_C(1) << 63;
    static const quint64 ExpandedNameMask = (Q_UINT64_C(1) << 50) - 1;
};

class Q_XML_EXPORT QXmlName
{
public:
    inline QXmlName() : m_code(QPrivateDetails::InvalidCode)
    {
    }

    inline bool operator==(const QXmlName &other) const
    {
        return (m_code & QPrivateDetails::ExpandedNameMask) == (other.m_code & QPrivateDetails::ExpandedNameMask);
    }

    inline bool operator!=(const QXmlName &other) const
    {
        return !operator==(other);
    }

    bool isNull() const;

    QString namespaceUri(const QXmlQuery &query) const;
    QString prefix(const QXmlQuery &query) const;
    QString localName(const QXmlQuery &query) const;

    QString toClarkName(const QXmlQuery &query) const;
private:
    friend uint qHash(const QXmlName &);
    friend class QXmlQueryPrivate;
    union
    {
        QXmlNamePrivate *d;
        quint64 m_code;
    };
};

static inline uint qHash(const QXmlName &name)
{
    return name.m_code & QPrivateDetails::ExpandedNameMask;
}

Q_DECLARE_TYPEINFO(QXmlName, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
