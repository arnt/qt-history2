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

#ifndef QT_NO_PRINTER

#ifndef QPRINTER_P_H
#define QPRINTER_P_H

class QPrintEngine;

class QPrinterPrivate
{
public:
    QPrinterPrivate()
        : printEngine(0)
    {
    }

    QPrintEngine *printEngine;
};

//#include "qstring.h"
//#include "q4printer.h"
//
//#include "qprintengine_win.h"
//
//class QPrinterPrivate
//{
//public:
//    QPrinterPrivate()
//	: paintEngine(0),
//	  collateCopies(false),
//	  fullPage(false),
//	  printToFile(false),
//	  colorMode(QPrinter::GrayScale),
//	  orientation(QPrinter::Portrait),
//	  pageOrder(QPrinter::FirstPageFirst),
//	  pageSize(QPrinter::A4),
//	  paperSource(QPrinter::Auto),
//	  printerMode(QPrinter::ScreenResolution),
//	  printRange(QPrinter::AllPages),
//	  resolution(0),
//	  numCopies(0),
//	  fromPage(0),
//	  toPage(0)
//    {
//#if defined (Q_QS_WIN)
//	winPageSize = 0;
//#endif
//	docName = "document1";
//    }
//
//    ~QPrinterPrivate()
//    {
//	if (paintEngine)
//	    delete paintEngine;
//    }
//
//    QPrintEngine *printEngine() { return static_cast<QPrintEngine*>(paintEngine); }
//
//    QPaintEngine *paintEngine;
//
//    bool collateCopies;
//    bool fullPage;
//    bool printToFile;
//
//    QString printerSelection;
//    QString creator;
//    QString docName;
//    QString printProgram;
//    QString printerFileName;
//    QString printerName;
//
//    QPrinter::ColorMode colorMode;
//    QPrinter::Orientation orientation;
//    QPrinter::PageOrder pageOrder;
//    QPrinter::PageSize pageSize;
//    QPrinter::PaperSource paperSource;
//    QPrinter::PrinterMode printerMode;
//    QPrinter::PrintRange printRange;
//
//    int resolution;
//    int numCopies;
//    int fromPage;
//    int toPage;
//
//    QRect pageRect;
//    QRect paperRect;
//    QList<int> resolutions;
//
//#if defined (Q_WS_WIN)
//    int winPageSize;
//#endif
//};
//
//#endif

#endif
#endif // QT_NO_PRINTER
