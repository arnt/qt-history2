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

#ifndef QPRINTENGINE_WIN_P_H
#define QPRINTENGINE_WIN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_PRINTER

#include "qprinter.h"
#include "qprintengine.h"
#include "qpaintengine.h"
#include "qpaintengine_p.h"

class QWin32PrintEnginePrivate;
class QPrinterPrivate;

class QWin32PrintEngine : public QPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QWin32PrintEngine)
public:
    QWin32PrintEngine(QPrinter::PrinterMode mode);

    // override QWin32PaintEngine
    bool begin(QPaintDevice *dev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updateMatrix(const QMatrix &matrix);
    void updateClipPath(const QPainterPath &clip, Qt::ClipOperation op);

    void drawPath(const QPainterPath &path);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &p);
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;

    bool newPage();
    bool abort();
    int metric(QPaintDevice::PaintDeviceMetric) const;

    QPrinter::PrinterState printerState() const;

    QPaintEngine::Type type() const { return Windows; }

    HDC getDC() const;
    void releaseDC(HDC) const;

    HDC getPrinterDC() const { return getDC(); }
    void releasePrinterDC(HDC dc) const { releaseDC(dc); }

private:
    friend class QPrintDialog;
    friend class QPageSetupDialog;
};

class QWin32PrintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PrintEngine)
public:
    QWin32PrintEnginePrivate() :
	hPrinter(0),
        globalDevMode(0),
	devMode(0),
        hdc(0),
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

    /* Queries the resolutions for the current printer, and returns them
       in a list. */
    QList<QVariant> queryResolutions() const;

    /* Resets the DC with changes in devmode. If the printer is active
       this funciton only sets the reinit variable to true so it
       is handled in the next begin or newpage. */
    void doReinit();

    /* Used by print/page setup dialogs */
    HGLOBAL *createDevNames();

    void readDevmode(HGLOBAL globalDevmode);
    void readDevnames(HGLOBAL globalDevnames);

    inline DEVMODEW *devModeW() const { return (DEVMODEW*) devMode; }
    inline DEVMODEA *devModeA() const { return (DEVMODEA*) devMode; }

    inline PRINTER_INFO_2W *pInfoW() { return (PRINTER_INFO_2W*) pInfo; };
    inline PRINTER_INFO_2A *pInfoA() { return (PRINTER_INFO_2A*) pInfo; };

    inline bool resetDC() {
        QT_WA( {
            hdc = ResetDCW(hdc, devModeW());
        }, {
            hdc = ResetDCA(hdc, devModeA());
        } );
        return hdc != 0;
    }

    void strokePath(const QPainterPath &path, const QColor &color);
    void fillPath(const QPainterPath &path, const QColor &color);

    void composeGdiPath(const QPainterPath &path);
    void fillPath_dev(const QPainterPath &path, const QColor &color);

    void updateOrigin();


    // Windows GDI printer references.
    HANDLE hPrinter;

    HGLOBAL globalDevMode;
    void *devMode;
    void *pInfo;

    HDC hdc;

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

    QRect devPageRect;
    QRect devPaperRect;
    qreal stretch_x;
    qreal stretch_y;
    int origin_x;
    int origin_y;

    int dpi_x;
    int dpi_y;
    int dpi_display;

    QFont pfont;

    uint printToFile : 1;
    uint fullPage : 1;
    uint reinit : 1;

    uint complex_xform : 1;
    uint has_pen : 1;
    uint has_brush : 1;

    QColor brush_color;
    QPen pen;
    QColor pen_color;

    QMatrix painterMatrix;
    QMatrix matrix;
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
