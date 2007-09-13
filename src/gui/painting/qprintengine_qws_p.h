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

#ifndef QPRINTENGINE_QWS_P_H
#define QPRINTENGINE_QWS_P_H

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

#include "QtGui/qprinter.h"

#ifndef QT_NO_PRINTER

#include "QtGui/qprintengine.h"
#include "QtCore/qbytearray.h"
#include "private/qpaintengine_p.h"

QT_BEGIN_NAMESPACE

class QtopiaPrintEnginePrivate;
class QRasterPaintEngine;
class QPrinterPrivate;
class QImage;

class QtopiaPrintEngine : public QPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QtopiaPrintEngine)
public:
    QtopiaPrintEngine(QPrinter::PrinterMode mode);

    // override QWSPaintEngine
    bool begin(QPaintDevice *dev);
    bool end();
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTextItem(const QPointF &p, const QTextItem &ti);
    QPaintEngine::Type type() const { return QPaintEngine::X11; }

    QPaintEngine *paintEngine() const;

    void updateState(const QPaintEngineState &state);

    QRect paperRect() const;
    QRect pageRect() const;

    bool newPage();
    bool abort();

    QPrinter::PrinterState printerState() const;

    int metric(QPaintDevice::PaintDeviceMetric metricType) const;

    QVariant property(PrintEnginePropertyKey key) const;
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);

private:
    friend class QPrintDialog;
    friend class QPageSetupDialog;

    void clearPage();
    void flushPage();
};

class QtopiaPrintBuffer
{
public:
    QtopiaPrintBuffer( bool bigEndian=FALSE ) { _bigEndian = bigEndian; }
    ~QtopiaPrintBuffer() {}

    const QByteArray& data() const { return _data; }

    int size() const { return _data.size(); }

    void clear() { _data.clear(); }

    void append( char value ) { _data.append( value ); }
    void append( short value );
    void append( int value );
    void append( const QByteArray& array ) { _data.append( array ); }

    void patch( int posn, int value );

    void pad();

private:
    QByteArray _data;
    bool _bigEndian;
};

#define	QT_QWS_PRINTER_DEFAULT_DPI	   200

class QtopiaPrintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QtopiaPrintEngine)
public:
    QtopiaPrintEnginePrivate(QPrinter::PrinterMode m) :
	mode(m),
	printerState(QPrinter::Idle),
	orientation(QPrinter::Portrait),
	pageSize(QPrinter::A4),
	pageOrder(QPrinter::FirstPageFirst),
	colorMode(QPrinter::GrayScale),
	paperSource(QPrinter::OnlyOne),
        resolution(QT_QWS_PRINTER_DEFAULT_DPI),
        paintEngine(0),
	numCopies(1),
        outputToFile(false),
        fullPage(false),
        collateCopies(false),
	pageNumber(0),
	pageImage(0),
	partialByte(0),
	partialBits(0)
    {
    }
    ~QtopiaPrintEnginePrivate();

    void initialize();

    QPrinter::PrinterMode mode;

    QString printerName;
    QString outputFileName;
    QString printProgram;
    QString docName;
    QString creator;

    QPrinter::PrinterState printerState;

    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    QPrinter::ColorMode colorMode;
    QPrinter::PaperSource paperSource;

    int resolution;
    QPaintEngine *paintEngine;
    int numCopies;

    bool outputToFile;
    bool fullPage;
    bool collateCopies;

    int pageNumber;

    QImage *pageImage;

    QtopiaPrintBuffer buffer;

    // Definitions that are only relevant to G3FAX output.
    int ifdPatch;
    int partialByte;
    int partialBits;
    void writeG3FaxHeader();
    void writeG3FaxPage();
    int writeG3IFDEntry( int tag, int type, int count, int value );
    void writeG3Code( int code, int bits );
    void writeG3WhiteRun( int len );
    void writeG3BlackRun( int len );
    void writeG3EOL();
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_QWS_P_H
