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

#ifndef QXMLTESTLOGGER_H
#define QXMLTESTLOGGER_H

#include "QtTest/private/qabstracttestlogger_p.h"

class QXmlTestLogger : public QAbstractTestLogger
{
public:
    enum XmlMode { Complete = 0, Light };

    QXmlTestLogger(XmlMode mode = Complete);
    ~QXmlTestLogger();

    void startLogging();
    void stopLogging();

    void enterTestFunction(const char *function);
    void leaveTestFunction();

    void addIncident(IncidentTypes type, const char *description,
                     const char *file = 0, int line = 0);

    void addMessage(MessageTypes type, const char *message,
                    const char *file = 0, int line = 0);

private:
    XmlMode xmlmode;
};

#endif
