/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_PRINTER

#include <private/qprintengine_qws_p.h>
#include <qbitmap.h>
#include <qfile.h>

#define MM(n) int((n * 720 + 127) / 254)
#define IN(n) int(n * 72)

struct PaperSize {
    int width, height;
};

static const PaperSize paperSizes[QPrinter::NPageSize] =
{
    {  MM(210), MM(297) },      // A4
    {  MM(176), MM(250) },      // B5
    {  IN(8.5), IN(11) },       // Letter
    {  IN(8.5), IN(14) },       // Legal
    {  IN(7.5), IN(10) },       // Executive
    {  MM(841), MM(1189) },     // A0
    {  MM(594), MM(841) },      // A1
    {  MM(420), MM(594) },      // A2
    {  MM(297), MM(420) },      // A3
    {  MM(148), MM(210) },      // A5
    {  MM(105), MM(148) },      // A6
    {  MM(74), MM(105)},        // A7
    {  MM(52), MM(74) },        // A8
    {  MM(37), MM(52) },        // A9
    {  MM(1000), MM(1414) },    // B0
    {  MM(707), MM(1000) },     // B1
    {  MM(31), MM(44) },        // B10
    {  MM(500), MM(707) },      // B2
    {  MM(353), MM(500) },      // B3
    {  MM(250), MM(353) },      // B4
    {  MM(125), MM(176) },      // B6
    {  MM(88), MM(125) },       // B7
    {  MM(62), MM(88) },        // B8
    {  MM(44), MM(62) },        // B9
    {  MM(162),    MM(229) },   // C5E
    {  IN(4.125),  IN(9.5) },   // Comm10E
    {  MM(110),    MM(220) },   // DLE
    {  IN(8.5),    IN(13) },    // Folio
    {  IN(17),     IN(11) },    // Ledger
    {  IN(11),     IN(17) }     // Tabloid
};

QWSPrintEngine::QWSPrintEngine(QPrinter::PrinterMode mode)
    : QWSPaintEngine(*(new QWSPrintEnginePrivate( mode )))
{
}

bool QWSPrintEngine::begin(QPaintDevice *)
{
    // Create a new off-screen monochrome image to handle the drawing process.
    QSize size = paperRect().size();
    if ( d_func()->pageImage )
	delete d_func()->pageImage;
    d_func()->pageImage = new QBitmap( size );
    if ( !(d_func()->pageImage) )
	return false;

    // Begin the paint process on the image.
    if ( !QWSPaintEngine::begin( d_func()->pageImage ) )
	return false;

    // Clear the first page to all-white.
    clearPage();

    // Clear the print buffer and output the image header.
    d_func()->buffer.clear();
    d_func()->writeG3FaxHeader();

    // The print engine is currently active.
    d_func()->printerState = QPrinter::Active;
    return true;
}

bool QWSPrintEngine::end()
{
    // Stop painting on the internal image.
    QWSPaintEngine::end();

    // Flush the last page.
    flushPage();

    // Output the fax data to a file (TODO: send to the print queuing daemon).
    QString filename;
    if ( outputToFile() )
	filename = outputFileName();
    else
	filename = "qwsfax.tiff";
    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
	qDebug( "Failed to open %s for printer output",
		filename.toLatin1().constData() );
    } else {
	file.write( d_func()->buffer.data() );
	file.close();
    }

    // Free up the memory for the image buffer.
    d_func()->buffer.clear();

    // Finalize the print job.
    d_func()->printerState = QPrinter::Idle;
    return true;
}

void QWSPrintEngine::setPrinterName(const QString &name)
{
    d_func()->printerName = name;
}

QString QWSPrintEngine::printerName() const
{
    return d_func()->printerName;
}

void QWSPrintEngine::setOutputToFile(bool toFile)
{
    d_func()->outputToFile = toFile;
}

bool QWSPrintEngine::outputToFile() const
{
    return d_func()->outputToFile;
}

void QWSPrintEngine::setOutputFileName(const QString &name)
{
    d_func()->outputFileName = name;
}

QString QWSPrintEngine::outputFileName() const
{
    return d_func()->outputFileName;
}

void QWSPrintEngine::setPrintProgram(const QString &name)
{
    d_func()->printProgram = name;
}

QString QWSPrintEngine::printProgram() const
{
    return d_func()->printProgram;
}

void QWSPrintEngine::setDocName(const QString &name)
{
    d_func()->docName = name;
}

QString QWSPrintEngine::docName() const
{
    return d_func()->docName;
}

void QWSPrintEngine::setCreator(const QString &creator)
{
    d_func()->creator = creator;
}

QString QWSPrintEngine::creator() const
{
    return d_func()->creator;
}

void QWSPrintEngine::setOrientation(QPrinter::Orientation orientation)
{
    d_func()->orientation = orientation;
}

QPrinter::Orientation QWSPrintEngine::orientation() const
{
    return d_func()->orientation;
}

void QWSPrintEngine::setPageSize(QPrinter::PageSize ps)
{
    d_func()->pageSize = ps;
}

QPrinter::PageSize QWSPrintEngine::pageSize() const
{
    return d_func()->pageSize;
}

void QWSPrintEngine::setPageOrder(QPrinter::PageOrder po)
{
    d_func()->pageOrder = po;
}

QPrinter::PageOrder QWSPrintEngine::pageOrder() const
{
    return d_func()->pageOrder;
}

void QWSPrintEngine::setResolution(int res)
{
    d_func()->resolution = res;
}

int QWSPrintEngine::resolution() const
{
    return d_func()->resolution;
}

void QWSPrintEngine::setColorMode(QPrinter::ColorMode cm)
{
    d_func()->colorMode = cm;
}

QPrinter::ColorMode QWSPrintEngine::colorMode() const
{
    return d_func()->colorMode;
}

void QWSPrintEngine::setFullPage(bool fp)
{
    d_func()->fullPage = fp;
}

bool QWSPrintEngine::fullPage() const
{
    return d_func()->fullPage;
}

void QWSPrintEngine::setNumCopies(int numCopies)
{
    d_func()->numCopies = numCopies;
}

int QWSPrintEngine::numCopies() const
{
    return d_func()->numCopies;
}

void QWSPrintEngine::setCollateCopies(bool collate)
{
    d_func()->collateCopies = collate;
}

bool QWSPrintEngine::collateCopies() const
{
    return d_func()->collateCopies;
}

void QWSPrintEngine::setPaperSource(QPrinter::PaperSource ps)
{
    d_func()->paperSource = ps;
}

QPrinter::PaperSource QWSPrintEngine::paperSource() const
{
    return d_func()->paperSource;
}

QList<int> QWSPrintEngine::supportedResolutions() const
{
    return QList<int>() << QT_QWS_PRINTER_DEFAULT_DPI;
}

QRect QWSPrintEngine::paperRect() const
{
    PaperSize s = paperSizes[d_func()->pageSize];
    int w = qRound(s.width*d_func()->resolution/72.);
    int h = qRound(s.height*d_func()->resolution/72.);
    if (d_func()->orientation == QPrinter::Portrait)
        return QRect(0, 0, w, h);
    else
        return QRect(0, 0, h, w);
}

QRect QWSPrintEngine::pageRect() const
{
    QRect r = paperRect();
    if (d_func()->fullPage)
        return r;
    // would be nice to get better margins than this.
    return QRect(d_func()->resolution/3, d_func()->resolution/3, r.width()-2*d_func()->resolution/3, r.height()-2*d_func()->resolution/3);
}

bool QWSPrintEngine::newPage()
{
    flushPage();
    clearPage();
    ++(d_func()->pageNumber);
    return true;
}

bool QWSPrintEngine::abort()
{
    return false;
}

QPrinter::PrinterState QWSPrintEngine::printerState() const
{
    return d_func()->printerState;
}

int QWSPrintEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    int val;
    QRect r = d_func()->fullPage ? paperRect() : pageRect();
    switch (metricType) {
    case QPaintDevice::PdmWidth:
        val = r.width();
        break;
    case QPaintDevice::PdmHeight:
        val = r.height();
        break;
    case QPaintDevice::PdmDpiX:
        val = d_func()->resolution;
        break;
    case QPaintDevice::PdmDpiY:
        val = d_func()->resolution;
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
        val = QT_QWS_PRINTER_DEFAULT_DPI;
        break;
    case QPaintDevice::PdmWidthMM:
        val = qRound(r.width()*25.4/d_func()->resolution);
        break;
    case QPaintDevice::PdmHeightMM:
        val = qRound(r.height()*25.4/d_func()->resolution);
        break;
    case QPaintDevice::PdmNumColors:
        val = 2;
        break;
    case QPaintDevice::PdmDepth:
        val = 1;
        break;
    default:
        qWarning("QWSPrintEngine::metric: Invalid metric command");
        return 0;
    }
    return val;
}

void QWSPrintEngine::clearPage()
{
    d_func()->pageImage->fill( Qt::color0 );
}

void QWSPrintEngine::flushPage()
{
    d_func()->writeG3FaxPage();
}

QWSPrintEnginePrivate::~QWSPrintEnginePrivate()
{
    if ( pageImage )
	delete pageImage;
}

void QWSPrintEnginePrivate::writeG3FaxHeader()
{
    // Write the TIFF file magic number (little-endian TIFF).
    buffer.append( (char)'I' );
    buffer.append( (char)'I' );
    buffer.append( (char)42 );
    buffer.append( (char)0 );

    // Leave a place-holder for the IFD offset of the first page.
    ifdPatch = buffer.size();
    buffer.append( (int)0 );
}

// Tag values, from RFC 2301.
#define	TIFF_IFD_NEW_SUB_FILE_TYPE	254
#define	TIFF_IFD_IMAGE_WIDTH		256
#define	TIFF_IFD_IMAGE_LENGTH		257
#define	TIFF_IFD_BITS_PER_SAMPLE	258
#define	TIFF_IFD_COMPRESSION		259
#define	TIFF_IFD_PHOTOMETRIC_INTERP	262
#define	TIFF_IFD_FILL_ORDER		266
#define	TIFF_IFD_STRIP_OFFSETS		273
#define	TIFF_IFD_ORIENTATION		274
#define	TIFF_IFD_SAMPLES_PER_PIXEL	277
#define	TIFF_IFD_ROWS_PER_STRIP		278
#define	TIFF_IFD_STRIP_BYTE_COUNTS	279
#define	TIFF_IFD_X_RESOLUTION	    	282
#define	TIFF_IFD_Y_RESOLUTION	    	283
#define	TIFF_IFD_PLANAR_CONFIG		284
#define	TIFF_IFD_T4_OPTIONS		292
#define	TIFF_IFD_RESOLUTION_UNIT	296
#define	TIFF_IFD_PAGE_NUMBER	    	297
#define	TIFF_IFD_CLEAN_FAX_DATA		327

// IFD type values.
#define	TIFF_TYPE_SHORT			3
#define	TIFF_TYPE_LONG			4
#define	TIFF_TYPE_RATIONAL		5

// Construct a SHORT pair from two values.
#define	TIFF_SHORT_PAIR(a,b)		(((a) & 0xFFFF) | ((b) << 16))

// Width of a FAX page in pixels, in the baseline specification from RFC 2301.
// This must be hard-wired, as per the RFC.  We truncate any pixels that
// are beyond this limit, or pad lines to reach this limit.
#define	TIFF_FAX_WIDTH			1728

void QWSPrintEnginePrivate::writeG3FaxPage()
{
    // Pad the image file to a word boundary, just in case.
    buffer.pad();

    // Back-patch the IFD link for the previous page.
    buffer.patch( ifdPatch, buffer.size() );

    // Output the contents of the IFD for this page (these must be
    // in ascending order of tag value).
    buffer.append( (short)19 );	    // Number of IFD entries.
    writeG3IFDEntry( TIFF_IFD_NEW_SUB_FILE_TYPE, TIFF_TYPE_LONG, 1, 2 );
    writeG3IFDEntry( TIFF_IFD_IMAGE_WIDTH, TIFF_TYPE_LONG, 1, TIFF_FAX_WIDTH );
    writeG3IFDEntry
	( TIFF_IFD_IMAGE_LENGTH, TIFF_TYPE_LONG, 1, pageImage->height() );
    writeG3IFDEntry( TIFF_IFD_BITS_PER_SAMPLE, TIFF_TYPE_SHORT, 1, 1 );
    writeG3IFDEntry( TIFF_IFD_COMPRESSION, TIFF_TYPE_SHORT, 1, 3 );
    writeG3IFDEntry( TIFF_IFD_PHOTOMETRIC_INTERP, TIFF_TYPE_SHORT, 1, 0 );
    writeG3IFDEntry( TIFF_IFD_FILL_ORDER, TIFF_TYPE_SHORT, 1, 1 );
    int stripOffsets =
	writeG3IFDEntry( TIFF_IFD_STRIP_OFFSETS, TIFF_TYPE_LONG, 1, 0 );
    writeG3IFDEntry( TIFF_IFD_ORIENTATION, TIFF_TYPE_SHORT, 1, 1 );
    writeG3IFDEntry( TIFF_IFD_SAMPLES_PER_PIXEL, TIFF_TYPE_SHORT, 1, 1 );
    writeG3IFDEntry
	( TIFF_IFD_ROWS_PER_STRIP, TIFF_TYPE_LONG, 1, pageImage->height() );
    int stripBytes = writeG3IFDEntry
	( TIFF_IFD_STRIP_BYTE_COUNTS, TIFF_TYPE_LONG, 1, 0 );
    int xres =
	writeG3IFDEntry( TIFF_IFD_X_RESOLUTION, TIFF_TYPE_RATIONAL, 1, 0 );
    int yres =
	writeG3IFDEntry( TIFF_IFD_Y_RESOLUTION, TIFF_TYPE_RATIONAL, 1, 0 );
    writeG3IFDEntry( TIFF_IFD_PLANAR_CONFIG, TIFF_TYPE_SHORT, 1, 1 );
    writeG3IFDEntry( TIFF_IFD_T4_OPTIONS, TIFF_TYPE_LONG, 1, 0 );
    writeG3IFDEntry( TIFF_IFD_RESOLUTION_UNIT, TIFF_TYPE_SHORT, 1, 2 );
    writeG3IFDEntry( TIFF_IFD_PAGE_NUMBER, TIFF_TYPE_SHORT, 2,
		     TIFF_SHORT_PAIR( pageNumber, 0 ) );
    writeG3IFDEntry( TIFF_IFD_CLEAN_FAX_DATA, TIFF_TYPE_SHORT, 1, 0 );

    // Leave a place-holder for the IFD offset of the next page.
    ifdPatch = buffer.size();
    buffer.append( (int)0 );

    // Output the X and Y resolutions, as rational values (usually 200/1).
    buffer.patch( xres, buffer.size() );
    buffer.append( (int)resolution );
    buffer.append( (int)1 );
    buffer.patch( yres, buffer.size() );
    buffer.append( (int)resolution );
    buffer.append( (int)1 );

    // We are now at the start of the image data - set the strip offset.
    int start = buffer.size();
    buffer.patch( stripOffsets, start );

    // Output the image data.
    int width = pageImage->width();
    for ( int y = 0; y < pageImage->height(); ++y ) {
	unsigned char *scan = pageImage->scanLine(y);
	int prev, pixel, len;
	writeG3EOL();
	prev = 0;
	len = 0;
	for ( int x = 0; x < width && x < TIFF_FAX_WIDTH; ++x ) {
	    pixel = ((scan[x >> 3] & (1 << (x & 7))) != 0);
	    if ( pixel != prev ) {
		if ( prev ) {
		    writeG3BlackRun( len );
		} else {
		    writeG3WhiteRun( len );
		}
		prev = pixel;
		len = 1;
	    } else {
		++len;
	    }
	}

	// Output the last run on the line, and pad to TIFF_FAX_WIDTH.
	if ( len != 0 ) {
	    if ( prev ) {
		writeG3BlackRun( len );
		if ( width < TIFF_FAX_WIDTH ) {
		    writeG3WhiteRun( TIFF_FAX_WIDTH - width );
		}
	    } else {
		if ( width < TIFF_FAX_WIDTH ) {
		    writeG3WhiteRun( len + ( TIFF_FAX_WIDTH - width ) );
		} else {
		    writeG3WhiteRun( len );
		}
	    }
	}
    }

    // Flush the last partial byte, which is padded with zero fill bits.
    if ( partialBits > 0 ) {
	buffer.append( (char)( partialByte << ( 8 - partialBits ) ) );
	partialByte = 0;
	partialBits = 0;
    }

    // Update the byte count for the image data strip.
    buffer.patch( stripBytes, buffer.size() - start );
}

int QWSPrintEnginePrivate::writeG3IFDEntry
	( int tag, int type, int count, int value )
{
    buffer.append( (short)tag );
    buffer.append( (short)type );
    buffer.append( count );
    buffer.append( value );
    return buffer.size() - 4;    // Offset of the value for back-patching.
}

void QWSPrintEnginePrivate::writeG3Code( int code, int bits )
{
    partialByte = ( ( partialByte << bits ) | code );
    partialBits += bits;
    while ( partialBits >= 8 ) {
	partialBits -= 8;
	buffer.append( (char)( partialByte >> partialBits ) );
    }
}

void QWSPrintEnginePrivate::writeG3WhiteRun( int len )
{
    static struct {
	unsigned short code;
	unsigned short bits;
    } whiteCodes[64 + 27] = {
	{0x0035, 8},		// 0
	{0x0007, 6},
	{0x0007, 4},
	{0x0008, 4},
	{0x000B, 4},
	{0x000C, 4},
	{0x000E, 4},
	{0x000F, 4},
	{0x0013, 5},		// 8
	{0x0014, 5},
	{0x0007, 5},
	{0x0008, 5},
	{0x0008, 6},
	{0x0003, 6},
	{0x0034, 6},
	{0x0035, 6},
	{0x002A, 6},		// 16
	{0x002B, 6},
	{0x0027, 7},
	{0x000C, 7},
	{0x0008, 7},
	{0x0017, 7},
	{0x0003, 7},
	{0x0004, 7},
	{0x0028, 7},		// 24
	{0x002B, 7},
	{0x0013, 7},
	{0x0024, 7},
	{0x0018, 7},
	{0x0002, 8},
	{0x0003, 8},
	{0x001A, 8},
	{0x001B, 8},		// 32
	{0x0012, 8},
	{0x0013, 8},
	{0x0014, 8},
	{0x0015, 8},
	{0x0016, 8},
	{0x0017, 8},
	{0x0028, 8},
	{0x0029, 8},		// 40
	{0x002A, 8},
	{0x002B, 8},
	{0x002C, 8},
	{0x002D, 8},
	{0x0004, 8},
	{0x0005, 8},
	{0x000A, 8},
	{0x000B, 8},		// 48
	{0x0052, 8},
	{0x0053, 8},
	{0x0054, 8},
	{0x0055, 8},
	{0x0024, 8},
	{0x0025, 8},
	{0x0058, 8},
	{0x0059, 8},		// 56
	{0x005A, 8},
	{0x005B, 8},
	{0x004A, 8},
	{0x004B, 8},
	{0x0032, 8},
	{0x0033, 8},
	{0x0034, 8},
	{0x001B, 5},		// Make up codes: 64
	{0x0012, 5},		// 128
	{0x0017, 6},		// 192
	{0x0037, 7},		// 256
	{0x0036, 8},		// 320
	{0x0037, 8},		// 384
	{0x0064, 8},		// 448
	{0x0065, 8},		// 512
	{0x0068, 8},		// 576
	{0x0067, 8},		// 640
	{0x00CC, 9},		// 704
	{0x00CD, 9},		// 768
	{0x00D2, 9},		// 832
	{0x00D3, 9},		// 896
	{0x00D4, 9},		// 960
	{0x00D5, 9},		// 1024
	{0x00D6, 9},		// 1088
	{0x00D7, 9},		// 1152
	{0x00D8, 9},		// 1216
	{0x00D9, 9},		// 1280
	{0x00DA, 9},		// 1344
	{0x00DB, 9},		// 1408
	{0x0098, 9},		// 1472
	{0x0099, 9},		// 1536
	{0x009A, 9},		// 1600
	{0x0018, 6},		// 1664
	{0x009B, 9},		// 1728
    };
    if ( len >= 64 ) {
	int index = 63 + (len >> 6);
	writeG3Code( whiteCodes[index].code, whiteCodes[index].bits );
	len &= 63;
    }
    writeG3Code( whiteCodes[len].code, whiteCodes[len].bits );
}

void QWSPrintEnginePrivate::writeG3BlackRun( int len )
{
    static struct {
	unsigned short code;
	unsigned short bits;
    } blackCodes[64 + 27] = {
	{0x0037, 10},		// 0
	{0x0002, 3},
	{0x0003, 2},
	{0x0002, 2},
	{0x0003, 3},
	{0x0003, 4},
	{0x0002, 4},
	{0x0003, 5},
	{0x0005, 6},		// 8
	{0x0004, 6},
	{0x0004, 7},
	{0x0005, 7},
	{0x0007, 7},
	{0x0004, 8},
	{0x0007, 8},
	{0x0018, 9},
	{0x0017, 10},		// 16
	{0x0018, 10},
	{0x0008, 10},
	{0x0067, 11},
	{0x0068, 11},
	{0x006C, 11},
	{0x0037, 11},
	{0x0028, 11},
	{0x0017, 11},		// 24
	{0x0018, 11},
	{0x00CA, 12},
	{0x00CB, 12},
	{0x00CC, 12},
	{0x00CD, 12},
	{0x0068, 12},
	{0x0069, 12},
	{0x006A, 12},		// 32
	{0x006B, 12},
	{0x00D2, 12},
	{0x00D3, 12},
	{0x00D4, 12},
	{0x00D5, 12},
	{0x00D6, 12},
	{0x00D7, 12},
	{0x006C, 12},		// 40
	{0x006D, 12},
	{0x00DA, 12},
	{0x00DB, 12},
	{0x0054, 12},
	{0x0055, 12},
	{0x0056, 12},
	{0x0057, 12},
	{0x0064, 12},		// 48
	{0x0065, 12},
	{0x0052, 12},
	{0x0053, 12},
	{0x0024, 12},
	{0x0037, 12},
	{0x0038, 12},
	{0x0027, 12},
	{0x0028, 12},		// 56
	{0x0058, 12},
	{0x0059, 12},
	{0x002B, 12},
	{0x002C, 12},
	{0x005A, 12},
	{0x0066, 12},
	{0x0067, 12},
	{0x000F, 10},		// Make up codes: 64
	{0x00C8, 12},		// 128
	{0x00C9, 12},		// 192
	{0x005B, 12},		// 256
	{0x0033, 12},		// 320
	{0x0034, 12},		// 384
	{0x0035, 12},		// 448
	{0x006C, 13},		// 512
	{0x006D, 13},		// 576
	{0x004A, 13},		// 640
	{0x004B, 13},		// 704
	{0x004C, 13},		// 768
	{0x004D, 13},		// 832
	{0x0072, 13},		// 896
	{0x0073, 13},		// 960
	{0x0074, 13},		// 1024
	{0x0075, 13},		// 1088
	{0x0076, 13},		// 1152
	{0x0077, 13},		// 1216
	{0x0052, 13},		// 1280
	{0x0053, 13},		// 1344
	{0x0054, 13},		// 1408
	{0x0055, 13},		// 1472
	{0x005A, 13},		// 1536
	{0x005B, 13},		// 1600
	{0x0064, 13},		// 1664
	{0x0065, 13},		// 1728
    };
    if ( len >= 64 ) {
	int index = 63 + (len >> 6);
	writeG3Code( blackCodes[index].code, blackCodes[index].bits );
	len &= 63;
    }
    writeG3Code( blackCodes[len].code, blackCodes[len].bits );
}

void QWSPrintEnginePrivate::writeG3EOL()
{
    writeG3Code( 0x0001, 12 );
}

void QWSPrintBuffer::append( short value )
{
    if ( _bigEndian ) {
	_data.append( (char)(value >> 8) );
	_data.append( (char)value );
    } else {
	_data.append( (char)value );
	_data.append( (char)(value >> 8) );
    }
}

void QWSPrintBuffer::append( int value )
{
    if ( _bigEndian ) {
	_data.append( (char)(value >> 24) );
	_data.append( (char)(value >> 16) );
	_data.append( (char)(value >> 8) );
	_data.append( (char)value );
    } else {
	_data.append( (char)value );
	_data.append( (char)(value >> 8) );
	_data.append( (char)(value >> 16) );
	_data.append( (char)(value >> 24) );
    }
}

void QWSPrintBuffer::patch( int posn, int value )
{
    if ( _bigEndian ) {
	_data[posn]     = (char)(value >> 24);
	_data[posn + 1] = (char)(value >> 16);
	_data[posn + 2] = (char)(value >> 8);
	_data[posn + 3] = (char)value;
    } else {
	_data[posn]     = (char)value;
	_data[posn + 1] = (char)(value >> 8);
	_data[posn + 2] = (char)(value >> 16);
	_data[posn + 3] = (char)(value >> 24);
    }
}

void QWSPrintBuffer::pad()
{
    while ( ( _data.size() % 4 ) != 0 )
	_data.append( (char)0 );
}

#endif // QT_NO_PRINTER
