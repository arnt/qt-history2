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

#ifndef QPRINTENGINE_MAC_P_H
#define QPRINTENGINE_MAC_P_H

#ifndef QT_NO_PRINTER

#include "qprintengine_mac.h"
#include <private/qpaintengine_mac_p.h>

class QPrinterPrivate;

class QMacPrintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QMacPrintEngine);
public:
    QPrinter::PrinterMode mode;
    QPrinter::PrinterState state;
    PMPageFormat format;
    PMPrintSettings settings;
    PMPrintSession session;
    PMResolution resolution;
    QString outputFilename;
    bool outputToFile;
    bool fullPage;
    GWorldPtr qdHandle;
    QPaintEngine *paintEngine;
    QMacPrintEnginePrivate() : mode(QPrinter::ScreenResolution), state(QPrinter::Idle),
                               format(0), settings(0),session(0), qdHandle(0), paintEngine(0) {}
    void initialize();
    bool newPage_helper();
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
