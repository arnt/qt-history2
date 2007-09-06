/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#ifndef QXmlQuery_h
#define QXmlQuery_h

#include <QtCore/QUrl>

#include "qabstractmessagehandler.h"
#include "qabstracturiresolver.h"
#include "qserializationsettings.h"

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QAbstractXmlPullProvider;
class QAbstractXmlPushCallback;
class QStringList;
class QVariant;
class QXmlItemIterator;
class QXmlName;
class QXmlQueryPrivate;

class Q_XML_EXPORT QXmlQuery
{
public:
    QXmlQuery();
    QXmlQuery(const QXmlQuery &other);
    ~QXmlQuery();
    QXmlQuery &operator=(const QXmlQuery &other);

    void setMessageHandler(const QAbstractMessageHandler::Ptr &messageHandler);
    QAbstractMessageHandler::Ptr messageHandler() const;

    void setQuery(const QString &sourceCode, const QUrl &documentURI = QUrl());
    void setQuery(QIODevice *sourceCode, const QUrl &documentURI = QUrl());

    void bindVariable(const QXmlName &name, const QVariant &value);
    void bindVariable(const QString &localName, const QVariant &value);

    bool hasEvaluationError() const;
    bool isValid() const;

    QXmlName createName(const QString &localName,
                        const QString &namespaceURI = QString(),
                        const QString &prefix = QString());

    QAbstractXmlPullProvider *evaluateUsingPullProvider() const;
    QXmlItemIterator evaluateUsingItemIterator() const;

    void evaluateToPushCallback(QAbstractXmlPushCallback *callback) const;
    void serialize(QIODevice *outputDevice,
                   const QSerializationSettings &settings = QSerializationSettings()) const;
    void setUriResolver(const QAbstractUriResolver::Ptr &resolver);
    QAbstractUriResolver::Ptr uriResolver() const;

private:
    friend class QXmlName;
    QXmlQueryPrivate *d;
};

QT_END_HEADER
#endif
// vim: et:ts=4:sw=4:sts=4
