/****************************************************************************
**
** Definition of QPrintEnginePrivate class
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QPRINTENGINE_WIN_P_H
#define QPRINTENGINE_WIN_P_H

#ifndef QT_NO_PRINTER

#include <private/qpaintengine_win_p.h>

class QPrinterPrivate;

class QWin32PrintEnginePrivate : public QWin32PaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PrintEngine);
public:
    QWin32PrintEnginePrivate() :
	hPrinter(0),
        globalDevMode(0),
	devMode(0),
	pInfo(0),
        mode(QPrinter::ScreenResolution),
	state(QPrinter::Idle),
        resolution(0),
        printToFile(false),
        fullPage(false),
        reinit(false)
    {
    }

    /* Reads the default printer name and its driver (printerProgram) into
       the engines private data. */
    void queryDefault();

    /* Initializes the printer data based on the current printer name. This
       function creates a DEVMODE struct, HDC and a printer handle. If these
       structures are already in use, they are freed using release
    */
    void initialize();

    /* Releases all the handles the printer currently holds, HDC, DEVMODE,
       etc and resets the corresponding members to 0. */
    void release();

    /* Adjusts the viewport / viewport mapping to match the resolution
       specified by either ScreenResolution or HighResolution */
    void setupPrinterMapping();

    /* Adjusts the origin of the printer to the top left corner of the page
       when printing in full screen mode. */
    void setupOriginMapping();

    /* Queries the resolutions for the current printer, and returns them
       in a list. */
    QList<int> queryResolutions() const;

    // Windows GDI printer references.
    HANDLE hPrinter;

    HGLOBAL globalDevMode;
    DEVMODE *devMode;
    PRINTER_INFO_2 *pInfo;

    QPrinter::PrinterMode mode;

    // Printer info
    QString name;
    QString program;
    QString port;

    // Document info
    QString docName;
    QString fileName;

    QPrinter::PrinterState state;
    int resolution;

    QRect pageRect;
    QRect paperRect;

    QFont pfont;

    uint printToFile : 1;
    uint fullPage : 1;
    uint reinit : 1;
};



#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
