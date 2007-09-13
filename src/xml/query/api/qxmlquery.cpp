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

#include "qpullbridge_p.h"
#include "qpushbridge_p.h"
#include "qserializer_p.h"

#include "qxmlitemiterator.h"
#include "qxmlitemiterator_p.h"
#include "qxmlquery.h"
#include "qxmlquery_p.h"

QT_BEGIN_NAMESPACE

// TODO Mention XML chars and QString.

/*!
  \class QXmlQuery
  \brief Evaluates XQuery queries.
  \reentrant
  \since 4.4

  QXmlQuery is the central point for running XQuery queries. First setQuery() is called
  specifying the source code of the query, and one of the following members
  are called for the actual evaluation.

  serialize() evaluates the query and writes the result as XML to a QIODevice. For instance,
  the result is indented and then written to a file.

  evaluateUsingPullProvider() returns a pointer to a QAbstractXmlPullProvider that the
  user use like an iterator. This is conceptually like QXmlStreamReader.

  evaluateToPushCallback() takes a pointer to a QAbstractXmlPushCallback, whose members
  will be called to appropriately mirror the result of the query. This is
  conceptually like the SAX API.

  \section1 Variable Bindings

  Apart using the \a fn:doc() function to retrieve data into the query for processing,
  one can bind variables using bindVariable(). When having called bindVariable(), a variable binding will
  be available to the query by the specified name. It is not necessary to declare the variable as external
  inside the query.

  \section1 Thread Support

  Multiple queries can run in multiple threads, which is achieved by simply copying QXmlQuery and in
  the new instance modify it accordingly, such as changing the variable bindings or evaluate it
  with a different method. Behind the scenes, QXmlQuery will reuse resources such as opened files and
  compiled queries between threads, to the extent possible.

  \section1 Error Handling
  During the evaluation of a query, an error can occur. A dynamic type error, or failing to load
  a file are examples of uch runtime errors.
  When an evaluation error occur, the following happens:
  
  \list
    \o Error messages are sent to the messageHandler()
    \o hasEvaluationError() will return \c true
    \o The content produced, such as the bytes written out using serialize(), or the events
        sent to the QAbstractXmlPullProvider or QAbstractXmlPushCallback, is undefined
  \endlist

  \section1 Resource Management

  A query potentially creates nodes, opens documents, and in other ways allocate resources. These
  are automatically managed, and will be deallocated as soon they aren't needed anymore.
  If it is of interest to deallocate the resources a query has allocated, make sure the relevant
  QXmlQuery and QAbstractMessageHandler/QAbstractXmlPullProvider instances have been destructed.

 */

/*
 * TODO document the implementation of this class. E.g, the compilation caching.
 */


/*!
  Constructs an invalid query that cannot be used. setQuery() must be called.
 */
QXmlQuery::QXmlQuery() : d(new QXmlQueryPrivate())
{
}

/*!
  Constructs an QXmlQuery instance that is a copy of \a other.
 */
QXmlQuery::QXmlQuery(const QXmlQuery &other) : d(new QXmlQueryPrivate(*other.d))
{
}

/*!
  Destructs this QXmlQuery instance.
 */
QXmlQuery::~QXmlQuery()
{
    delete d;
}

/*!
  Assigns \a other to this QXmlQuery instance.
 */
QXmlQuery &QXmlQuery::operator=(const QXmlQuery &other)
{
    if(this != &other)
    {
        // TODO
        Q_ASSERT(false);
    }

    return *this;
}

/*!
  QPreparationContext does not claim ownership of \a aMessageHandler, unless the default
  message handler is used.

  The default message handler will write messages to \c stderr.
 */
void QXmlQuery::setMessageHandler(const QAbstractMessageHandler::Ptr &aMessageHandler)
{
    d->messageHandler = aMessageHandler;
}

/*!
  Returns the message handler that is being used.
 */
QAbstractMessageHandler::Ptr QXmlQuery::messageHandler() const
{
    return d->messageHandler;
}

/*!
  Sets this QXmlQuery instance to use the query contained in \a sourceCode. The
  encoding of \a sourceCode will be detected as according to XQuery's rules.

  \a sourceCode must be opened with at least the QIODevice::ReadOnly flag.

  \a documentURI should be set to the location of the \a sourceCode. It is used for message reporting,
  and is used for resolving some of relative URIs in the query(it is the default value of the
  static base URI, to be specific). \a documentURI must be empty or valid. If it's empty, the application's
  current working directory is used.

  If the query contains a static error such as a syntax error, descriptive messages are sent to messageHandler()
  and isValid() will return \c false.

  \a documentURI should be the location of the query. It is used for resolving relative URIs appearing in the query,
  and for message reporting. To be specific, it is the static base URI. If \a documentURI is not empty or valid,
  behavior is undefined. If \a documentURI is empty, the application's executable path is used.

  \sa isValid()
 */
void QXmlQuery::setQuery(QIODevice *sourceCode, const QUrl &documentURI)
{
    Q_ASSERT_X(sourceCode, Q_FUNC_INFO, "A null QIODevice pointer cannot be passed.");
    Q_ASSERT_X(sourceCode->isReadable(), Q_FUNC_INFO, "The device must be readable.");

    // FIXME do encoding sniffing
    setQuery(QString::fromUtf8(sourceCode->readAll().constData()), documentURI);
}

/*!
  \overload
  Equivalent to setQuery(QIODevice *, bool &), with the difference that it takes a QString
  for convenience.

  The same behaviors and obligations apply as for the prepareQuery() version that takes a QIODevice.

  No encoding detection will be done, since \a sourceCode is already a Unicode string.
 */
void QXmlQuery::setQuery(const QString &sourceCode, const QUrl &documentURI)
{
    Q_ASSERT_X(documentURI.isEmpty() || documentURI.isValid(), Q_FUNC_INFO,
               "The document URI must be valid.");

    d->componentsForUpdate = QXmlQueryPrivate::QuerySource;
    d->querySource = sourceCode;
    d->queryURI = QXmlQueryPrivate::normalizeQueryURI(documentURI);
}

// TODO addd overload that takes a QUrl.

/*!
 Makes a variable called \a name that has value \a value, avalable to the query.

 If \a value is invalid, any existing binding by name \a name is erased.

 If a variable by name \a name has previously been bound, the previous binding
 is replaced.

 \sa QVariant::isValid(), {QtXDM}{How QVariant maps to XQuery's Data Model}
 */
void QXmlQuery::bindVariable(const QXmlName &name, const QVariant &value)
{
    Q_ASSERT_X(!name.isNull(), Q_FUNC_INFO, "The name cannot be null.");

    const Patternist::QName poolName(QXmlQueryPrivate::toPoolName(name));

    if(d->variableBindings.contains(poolName))
    {
        /* If the type of the variable changed(as opposed to only the value),
         * we will have to recompile. */
        if(d->variableBindings.value(poolName).type() != value.type())
            d->componentsForUpdate = QXmlQueryPrivate::VariableBindings;
    }

    d->variableBindings.insert(poolName, value);
}

/*!
 \overload

 Same as above, but constructs a QXmlName that has an empty namespace and local name \a localName.
 This is convenience for calling:

 \code
    query.bindVariable(query.createName(localName), value);
 \endcode

 */
void QXmlQuery::bindVariable(const QString &localName, const QVariant &value)
{
    bindVariable(createName(localName), value);
}

/*!
  Returns \c true if an error occured while evaluating this query, otherwise \c false.
 */
bool QXmlQuery::hasEvaluationError() const
{
    return !isValid() || d->hasEvaluationError;
}

/*!
  Creates a QXmlName and returns it, from \a localName, \a namespaceURI and \a prefix.

  \a localName is mandatory and must be a valid local name, such as "p" or "body". "xhtml:p"
  is an invalid local name. The \a prefix follows the same syntactical rules.

  \sa {What is a Node Name?}
 */
QXmlName QXmlQuery::createName(const QString &localName,
                               const QString &namespaceURI,
                               const QString &prefix)
{
    return QXmlQueryPrivate::fromPoolName(d->namePool->allocateQName(namespaceURI, localName, prefix));
}

/*!
  Evaluates this query, and returns the result as a QAbstractXmlPullProvider.

  The user has ownership of the returned QAbstractXmlPullProvider, not QXmlQuery.

  If this query is invalid, behavior is undefined. If a runtime error occurs, it
  is undefined what events the returned QAbstractXmlPullProvider delivers.
 */
QAbstractXmlPullProvider *QXmlQuery::evaluateUsingPullProvider() const
{
    if(!isValid())
    {
        qWarning("This QXmlQuery is not valid. It cannot be evaluated. See QXmlQuery::isValid().");
        return 0;
    }

    try
    {
        Patternist::GenericDynamicContext::Ptr dynContext(d->dynamicContext());
        return new PullBridge(d->expression()->evaluateSequence(dynContext));
    }
    catch(const Patternist::Exception &)
    {
        return 0;
    }
}

/*!
  Evaluates this query and sends the result as a stream of events to \a callback.

  If \a callback is null or if this query is invalid, behavior is undefined.

  QXmlQuery does not claim ownership of \a callback.

  If an error occur during evaluation, messages are sent to messageHandler() and hasEvaluationError()
  will return \c true.

  \sa QAbstractXmlPushCallback
 */
void QXmlQuery::evaluateToPushCallback(QAbstractXmlPushCallback *callback) const
{
    Q_ASSERT_X(callback, Q_FUNC_INFO,
               "A valid callback must be passed. Otherwise the result cannot be sent anywhere.");

    try
    {
        Patternist::GenericDynamicContext::Ptr dynContext(d->dynamicContext());
        dynContext->setOutputReceiver(Patternist::SequenceReceiver::Ptr(new PushBridge(callback)));
        d->expression()->evaluateToSequenceReceiver(dynContext);
    }
    catch(const Patternist::Exception &)
    {
        d->hasEvaluationError = true;
    }
}

QXmlItemIterator QXmlQuery::evaluateUsingItemIterator() const
{
    const Patternist::GenericDynamicContext::Ptr dynContext(d->dynamicContext());
    return QXmlItemIterator(new QXmlItemIteratorPrivate(dynContext, d->expression()));
}

/*!
  Evaluates this query, and serializes the result to \a outputDevice, using the serialization
  \a settings.

  \a outputDevice must be a valid, writable QIODevice. If not, result is undefined.

  If this query is invalid, behavior is undefined.

  If an error occur during evaluation, messages are sent to messageHandler() and hasEvaluationError()
  will return \c true.

  \sa QSerializationSettings
 */
void QXmlQuery::serialize(QIODevice *outputDevice,
                          const QSerializationSettings &settings) const
{
    Q_ASSERT_X(outputDevice, Q_FUNC_INFO, "A non-null QIODevice pointer must be supplied.");
    Q_ASSERT_X(outputDevice->isWritable(), Q_FUNC_INFO, "An opened, writable QIODevice must be supplied.");
    Q_ASSERT_X(d->isValid(), Q_FUNC_INFO, "This query must be valid.");

    Patternist::GenericDynamicContext::Ptr context(d->dynamicContext());
    Patternist::Serializer::Ptr serializer(new Patternist::Serializer(context, d->expression().get()));
    serializer->setOutputDevice(outputDevice);
    serializer->setCodec(settings.codec());

    try
    {
        context->setOutputReceiver(serializer);
        d->expression()->evaluateToSequenceReceiver(context);
    }
    catch(const Patternist::Exception &)
    {
        d->hasEvaluationError = true;
    }
}

/*!
  Returns \c true if this query is valid, otherwise \c false.

  An invalid query is for instance a query that contains a syntax error, or a QXmlQuery instance
  for which setQuery() hasn't been called.
 */
bool QXmlQuery::isValid() const
{
    return d->isValid();
}

/*!
  Sets the URI resolver to \a resolver.

  \sa uriResolver()
 */
void QXmlQuery::setUriResolver(const QAbstractUriResolver::Ptr &resolver)
{
    d->uriResolver = resolver;
}

/*!
  Returns the URI resolver in use. If no URI resolver has been set, Patternist
  will use the URI in queries as is.

  The URI resolver provides a level of abstraction or "polymorphic URIs". For instance,
  a query can operate on "logical" URIs which a URI resolver rewrites to physical ones,
  or 

  Patternist calls the URI resolver for all URIs it encounters, except for
  namespaces. This means more specically:

  \list
    \o All builtin functions that deals with URIs. This is \c fn:doc(),
       and \c fn:doc-available()
    \o The URI for module imports
  \endlist

   For instance, in the case of \c fn:doc(), the
  absolute URI is the base URI in the static context(which most likely is the location
  of the query). Instead of using the URI the user specified, the return value
  of QAbstractUriResolver::resolve() will be used.

  When Patternist calls QAbstractUriResolver::resolve() the absolute URI is
  the URI which the XQuery language mandates should be used, and the relative URI
  is the URI which the user specified.

  \sa setUriResolver()
 */
QAbstractUriResolver::Ptr QXmlQuery::uriResolver() const
{
    return d->uriResolver;
}

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
