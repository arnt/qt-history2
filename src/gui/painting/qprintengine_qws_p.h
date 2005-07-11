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

#ifndef QT_NO_PRINTER

#include <private/qpaintengine_qws_p.h>
#include <qbytearray.h>
#include "qprinter.h"
#include "qprintengine.h"

class QWSPrintEnginePrivate;
class QPrinterPrivate;
class QPixmap;

class QWSPrintEngine : public QWSPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QWSPrintEngine)
public:
    QWSPrintEngine(QPrinter::PrinterMode mode);

    // override QWSPaintEngine
    bool begin(QPaintDevice *dev);
    bool end();

    // Printer functions...
    void setPrinterName(const QString &);
    QString printerName() const;

    void setOutputToFile(bool);
    bool outputToFile() const;

    void setOutputFileName(const QString &);
    QString outputFileName()const;

    void setPrintProgram(const QString &);
    QString printProgram() const;

    void setDocName(const QString &);
    QString docName() const;

    void setCreator(const QString &);
    QString creator() const;

    void setOrientation(QPrinter::Orientation);
    QPrinter::Orientation orientation() const;

    void setPageSize(QPrinter::PageSize);
    QPrinter::PageSize pageSize() const;

    void setPageOrder(QPrinter::PageOrder);
    QPrinter::PageOrder pageOrder() const;

    void setResolution(int);
    int resolution() const;

    void setColorMode(QPrinter::ColorMode);
    QPrinter::ColorMode colorMode() const;

    void setFullPage(bool);
    bool fullPage() const;

    void setNumCopies(int);
    int numCopies() const;

    void setCollateCopies(bool);
    bool collateCopies() const;
    void setPaperSource(QPrinter::PaperSource);
    QPrinter::PaperSource paperSource() const;

    QList<int> supportedResolutions() const;

    QRect paperRect() const;
    QRect pageRect() const;

    bool newPage();
    bool abort();

    QPrinter::PrinterState printerState() const;

    int metric(QPaintDevice::PaintDeviceMetric metricType) const;

private:
    friend class QPrintDialog;
    friend class QPageSetupDialog;

    void clearPage();
    void flushPage();
};

class QWSPrintBuffer
{
public:
    QWSPrintBuffer( bool bigEndian=FALSE ) { _bigEndian = bigEndian; }
    ~QWSPrintBuffer() {}

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

class QWSPrintEnginePrivate : public QWSPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWSPrintEngine)
public:
    QWSPrintEnginePrivate(QPrinter::PrinterMode m) :
	mode(m),
	printerState(QPrinter::Idle),
	orientation(QPrinter::Portrait),
	pageSize(QPrinter::A4),
	pageOrder(QPrinter::FirstPageFirst),
	colorMode(QPrinter::GrayScale),
	paperSource(QPrinter::OnlyOne),
        resolution(QT_QWS_PRINTER_DEFAULT_DPI),
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
    ~QWSPrintEnginePrivate();

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
    int numCopies;

    uint outputToFile : 1;
    uint fullPage : 1;
    uint collateCopies : 1;

    int pageNumber;

    QPixmap *pageImage;

    QWSPrintBuffer buffer;

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
