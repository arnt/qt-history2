/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <QtCore/qglobal.h>

#include "QtTest/private/qxmltestlogger_p.h"
#include "QtTest/private/qtestresult_p.h"

namespace QTest {

    static const char* xmlMessageType2String(QAbstractTestLogger::MessageTypes type)
    {
        switch (type) {
        case QAbstractTestLogger::Warn:
            return "warn";
        case QAbstractTestLogger::QSystem:
            return "system";
        case QAbstractTestLogger::QDebug:
            return "qdebug";
        case QAbstractTestLogger::QWarning:
            return "qwarn";
        case QAbstractTestLogger::QFatal:
            return "qfatal";
        case QAbstractTestLogger::Skip:
            return "skip";
        case QAbstractTestLogger::Info:
            return "info";
        }
        return "??????";
    }

    static const char* xmlIncidentType2String(QAbstractTestLogger::IncidentTypes type)
    {
        switch (type) {
        case QAbstractTestLogger::Pass:
            return "pass";
        case QAbstractTestLogger::XFail:
            return "xfail";
        case QAbstractTestLogger::Fail:
            return "fail";
        case QAbstractTestLogger::XPass:
            return "xpass";
        }
        return "??????";
    }

}


QXmlTestLogger::QXmlTestLogger(XmlMode mode ):
    xmlmode(mode)
{

}

QXmlTestLogger::~QXmlTestLogger()
{

}


void QXmlTestLogger::startLogging()
{
    QAbstractTestLogger::startLogging();
    char buf[1024];

    if (xmlmode == QXmlTestLogger::Complete) {
        QTest::qt_snprintf(buf, sizeof(buf),
                "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                "<TestCase name=\"%s\">\n", QTestResult::currentTestObjectName());
        outputString(buf);
    }

    QTest::qt_snprintf(buf, sizeof(buf),
            "<Environment>\n"
            "    <QtVersion>%s</QtVersion>\n"
            "    <QTestVersion>"QTEST_VERSION_STR"</QTestVersion>\n"
            "</Environment>\n", qVersion());
    outputString(buf);
}

void QXmlTestLogger::stopLogging()
{
    if (xmlmode == QXmlTestLogger::Complete) {
        outputString("</TestCase>\n");
    }

    QAbstractTestLogger::stopLogging();
}

void QXmlTestLogger::enterTestFunction(const char *function)
{
    char buf[1024];
    QTest::qt_snprintf(buf, sizeof(buf), "<TestFunction name=\"%s\">\n", function);
    outputString(buf);
}

void QXmlTestLogger::leaveTestFunction()
{
    outputString("</TestFunction>\n");
}

namespace QTest
{

inline static bool isEmpty(const char *str)
{
    return !str || !str[0];
}

static const char *incidentFormatString(bool noDescription, bool noTag)
{
    if (noDescription) {
        if (noTag)
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\" />\n";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s%s]]></DataTag>\n"
                   "</Incident>\n";
    } else {
        if (noTag)
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s%s%s]]></Description>\n"
                   "</Incident>\n";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Incident>\n";
    }
}

static const char *messageFormatString(bool noDescription, bool noTag)
{
    if (noDescription) {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\" />\n";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s%s]]></DataTag>\n"
                   "</Message>\n";
    } else {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s%s%s]]></Description>\n"
                   "</Message>\n";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Message>\n";
    }
}

} // namespace

void QXmlTestLogger::addIncident(IncidentTypes type, const char *description,
                                 const char *file, int line)
{
    char buf[1536];
    const char *tag = QTestResult::currentDataTag();
    const char *gtag = QTestResult::currentGlobalDataTag();
    const char *filler = (tag && gtag) ? ":" : "";
    const bool notag = QTest::isEmpty(tag) && QTest::isEmpty(gtag);

    QTest::qt_snprintf(buf, sizeof(buf),
            QTest::incidentFormatString(QTest::isEmpty(description), notag),
            QTest::xmlIncidentType2String(type),
            file ? file : "", line,
            gtag ? gtag : "",
            filler,
            tag ? tag : "",
            description ? description : "");

    outputString(buf);
}

void QXmlTestLogger::addMessage(MessageTypes type, const char *message,
                                const char *file, int line)
{
    char buf[1536];
    char msgbuf[1024];
    const char *tag = QTestResult::currentDataTag();
    const char *gtag = QTestResult::currentGlobalDataTag();
    const char *filler = (tag && gtag) ? ":" : "";
    const bool notag = QTest::isEmpty(tag) && QTest::isEmpty(gtag);

    QTest::qt_snprintf(msgbuf, sizeof(msgbuf), "%s",
                        message ? message : "");

    QTest::qt_snprintf(buf, sizeof(buf),
            QTest::messageFormatString(QTest::isEmpty(message), notag),
            QTest::xmlMessageType2String(type),
            file ? file : "", line,
            gtag ? gtag : "",
            filler,
            tag ? tag : "",
            msgbuf);

    outputString(buf);
}

