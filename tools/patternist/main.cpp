/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QtDebug>
#include <QTextCodec>
#include <QTextStream>
#include <QUrl>
#include <QVariant>

#include "ColoringMessageHandler.h"
#include "qxmlquery.h"
#include "qserializationsettings.h"

#include "main.h"

QT_BEGIN_NAMESPACE

typedef QVector<QPair<QString, QString> > Bindings;

/*!
 \internal
 \since 4.4
 */
static Bindings parseBindings(const QStringList &input,
                              bool &ok)
{
    Bindings result;
    int len = input.size();
    result.reserve(len);

    for(int i = 0; i < len; ++i)
    {
        const QString &at = input.at(i);
        const int assign = at.indexOf(QLatin1Char('='));

        if(assign == -1)
        {
            QTextStream(stderr) << PatternistCLI::tr("Each binding must contain an equal sign.");
            ok = false;
            return Bindings();
        }

        const QString name(at.left(assign));
        const QString value(at.mid(assign + 1));

        /* TODO check that name is valid. */
        result.append(qMakePair(name, value));
    }

    ok = true;
    return result;
}

int runPatternist(int argc, char **argv)
{
    enum ExitCode
    {
        Success = 0,
        QueryFailure,
        InvalidArguments,
        InvalidURI,
        QueryInexistent,
        OpenFailure,
        StdOutFailure,
        VariableBindingParseFailure
    };

    const QCoreApplication app(argc, argv);
    const QStringList arguments(app.arguments());
    const int argCount = arguments.count() - 1;
    Q_ASSERT(argCount >= 0);

    if(argCount < 1)
    {
        QTextStream(stderr) << PatternistCLI::tr("%1 arguments cannot be supplied, "
                                                 "only one: the filename identifying the XQuery query.").arg(argCount) << endl;
        return InvalidArguments;
    }

    const QUrl queryURI(QUrl::fromLocalFile(QDir::currentPath() + QLatin1Char('/')).resolved(QUrl::fromLocalFile(arguments.at(1))));

    if(!queryURI.isValid())
    {
        QTextStream(stderr) << PatternistCLI::tr("%1 is an invalid URI.").arg(queryURI.toString()) << endl;
        return InvalidURI;
    }

    const QFileInfo fileInfo(queryURI.toLocalFile());

    if(!fileInfo.exists())
    {
        QTextStream(stderr) << PatternistCLI::tr("%1 does not exist.").arg(fileInfo.filePath()) << endl;
        return QueryInexistent;
    }

    QFile queryDevice(fileInfo.absoluteFilePath());

    if(!queryDevice.open(QIODevice::ReadOnly))
    {
        QTextStream(stderr) << PatternistCLI::tr("Failed to open file %1.").arg(fileInfo.filePath()) << endl;
        return OpenFailure;
    }

    QFile myStdout;

    if(!myStdout.open(stdout, QIODevice::WriteOnly))
    {
        QTextStream(stderr) << PatternistCLI::tr("Failed to open standard out.") << endl;
        return StdOutFailure;
    }

    bool ok;
    const Bindings bindings(parseBindings(arguments.mid(2), ok));

    if(!ok)
        return VariableBindingParseFailure;

    QAbstractMessageHandler::Ptr messageHandler(new ColoringMessageHandler());
    QXmlQuery query;
    query.setMessageHandler(messageHandler);
    query.setQuery(&queryDevice, queryURI);

    const int len = bindings.size();
    for(int i = 0; i < len; ++i)
        query.bindVariable(bindings.at(i).first, bindings.at(i).second);

    if(query.isValid())
        query.serialize(&myStdout);
    else
        return QueryFailure;

    if(query.hasEvaluationError())
        return QueryFailure;
    else
        return Success;
}

QT_END_NAMESPACE

int main(int argc, char **argv)
{
    return QT_ADD_NAMESPACE(runPatternist)(argc, argv);
}

// vim: et:ts=4:sw=4:sts=4
