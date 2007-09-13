/****************************************************************************
 * ** * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QXmlStreamReader>

#include "main.h"
#include "ColorOutput.h"

#include "ColoringMessageHandler.h"

QT_BEGIN_NAMESPACE

ColoringMessageHandler::ColoringMessageHandler()
{
    m_classToColor.insert(QLatin1String("XQuery-data"), Data);
    m_classToColor.insert(QLatin1String("XQuery-expression"), Keyword);
    m_classToColor.insert(QLatin1String("XQuery-function"), Keyword);
    m_classToColor.insert(QLatin1String("XQuery-keyword"), Keyword);
    m_classToColor.insert(QLatin1String("XQuery-type"), Keyword);
    m_classToColor.insert(QLatin1String("XQuery-uri"), Data);

    /* If you're tuning the colors, take it easy laddie. Take into account:
     *
     * - Get over your own taste, there's others too on this planet
     * - Make sure it works well on black & white
     * - Make sure it works well on white & black
     */
    insertMapping(Location, CyanForeground);
    insertMapping(ErrorCode, RedForeground);
    insertMapping(Keyword, BlueForeground);
    insertMapping(Data, BlueForeground);
    insertMapping(RunningText, DefaultColor);
}

void ColoringMessageHandler::handleMessage(QtMsgType type,
                                           const QString &description,
                                           const QUrl &identifier,
                                           const QSourceLocation &sourceLocation)
{
    switch(type)
    {
        case QtWarningMsg:
        {
            writeUncolored(PatternistCLI::tr("Warning in %1, at line %2, column %3: %4").arg(QString::fromLatin1(sourceLocation.uri().toEncoded()),
                                                                                             QString::number(sourceLocation.line()),
                                                                                             QString::number(sourceLocation.column()),
                                                                                             colorifyDescription(description)));
            break;
        }
        case QtFatalMsg:
        {
            const QString errorCode(identifier.fragment());
            Q_ASSERT(!errorCode.isEmpty());
            QUrl uri(identifier);
            uri.setFragment(QString());

            QString errorId;
            /* If it's a standard error code, we don't want to output the
             * whole URI. */
            if(uri.toString() == QLatin1String("http://www.w3.org/2005/xqt-errors"))
                errorId = errorCode;
            else
                errorId = QString::fromLatin1(identifier.toEncoded());

            writeUncolored(PatternistCLI::tr("Error %1 in %2, at line %3, column %4: %5").arg(colorify(errorId, ErrorCode),
                                                                                              colorify(QString::fromLatin1(sourceLocation.uri().toEncoded()), Location),
                                                                                              colorify(QString::number(sourceLocation.line()), Location),
                                                                                              colorify(QString::number(sourceLocation.column()), Location),
                                                                                              colorifyDescription(description)));
            break;
        }
        case QtCriticalMsg:
        /* Fallthrough. */
        case QtDebugMsg:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "message() is not supposed to receive QtCriticalMsg or QtDebugMsg.");
            return;
        }
    }
}

QString ColoringMessageHandler::colorifyDescription(const QString &in) const
{
    QXmlStreamReader reader(in);
    QString result;
    result.reserve(in.size());
    ColorType currentColor = RunningText;

    while(!reader.atEnd())
    {
        reader.readNext();

        switch(reader.tokenType())
        {
            case QXmlStreamReader::StartElement:
            {
                if(reader.name() == QLatin1String("span"))
                {
                    Q_ASSERT(m_classToColor.contains(reader.attributes().value(QLatin1String("class")).toString()));
                    currentColor = m_classToColor.value(reader.attributes().value(QLatin1String("class")).toString());
                }

                continue;
            }
            case QXmlStreamReader::Characters:
            {
                result.append(colorify(reader.text().toString(), currentColor));
                continue;
            }
            case QXmlStreamReader::EndElement:
            {
                currentColor = RunningText;
                continue;
            }
            /* Fallthrough, */
            case QXmlStreamReader::StartDocument:
            /* Fallthrough, */
            case QXmlStreamReader::EndDocument:
                continue;
            default:
                Q_ASSERT_X(false, Q_FUNC_INFO,
                           "Unexpected node.");
        }
    }

    Q_ASSERT_X(!reader.hasError(), Q_FUNC_INFO,
               "The output from Patternist must be well-formed.");
    return result;
}

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
