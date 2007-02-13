/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>



#include <qprinter.h>
#include <qpagesetupdialog.h>
#include <qpainter.h>
#include <qprintdialog.h>
#include <q3paintdevicemetrics.h>
#include <qvariant.h>
#include <qpainter.h>
#include <qprintengine.h>

#include <math.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

Q_DECLARE_METATYPE(QRect)



//TESTED_CLASS=
//TESTED_FILES=gui/painting/qprinter.h gui/painting/qprinter.cpp

class tst_QPrinter : public QObject
{
    Q_OBJECT

public:
    tst_QPrinter();
    virtual ~tst_QPrinter();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
// Add your testfunctions and testdata create functions here
    void testPageSize();
    void testPageRectAndPaperRect();
    void testPageRectAndPaperRect_data();
    void testSetOptions();
    void testMargins_data();
    void testMargins();
    void testNonExistentPrinter();
    void testPageSetupDialog();
    void testMulitpleSets_data();
    void testMulitpleSets();
    void changingOutputFormat();
    void outputFormatFromSuffix();
private:
};

// Testing get/set functions
void tst_QPrinter::getSetCheck()
{
    QPrinter obj1;
    // OutputFormat QPrinter::outputFormat()
    // void QPrinter::setOutputFormat(OutputFormat)
    obj1.setOutputFormat(QPrinter::OutputFormat(QPrinter::PdfFormat));
    QCOMPARE(QPrinter::OutputFormat(QPrinter::PdfFormat), obj1.outputFormat());

    // bool QPrinter::collateCopies()
    // void QPrinter::setCollateCopies(bool)
    obj1.setCollateCopies(false);
    QCOMPARE(false, obj1.collateCopies());
    obj1.setCollateCopies(true);
#if !defined(Q_OS_UNIX) && QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Apparently only the Unix (ps) printer supports collate at the moment", Continue);
#endif
    QCOMPARE(true, obj1.collateCopies());

    // bool QPrinter::fontEmbeddingEnabled()
    // void QPrinter::setFontEmbeddingEnabled(bool)
    obj1.setFontEmbeddingEnabled(false);
    QCOMPARE(false, obj1.fontEmbeddingEnabled());
    obj1.setFontEmbeddingEnabled(true);
#if !defined(Q_OS_UNIX) && QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Apparently only the Unix (ps) printer supports font embedding at the moment", Continue);
#endif
    QCOMPARE(true, obj1.fontEmbeddingEnabled());

    // PageSize QPrinter::pageSize()
    // void QPrinter::setPageSize(PageSize)
    obj1.setPageSize(QPrinter::PageSize(QPrinter::A4));
    QCOMPARE(QPrinter::PageSize(QPrinter::A4), obj1.pageSize());
    obj1.setPageSize(QPrinter::PageSize(QPrinter::Letter));
    QCOMPARE(QPrinter::PageSize(QPrinter::Letter), obj1.pageSize());
    obj1.setPageSize(QPrinter::PageSize(QPrinter::Legal));
    QCOMPARE(QPrinter::PageSize(QPrinter::Legal), obj1.pageSize());

    // PrintRange QPrinter::printRange()
    // void QPrinter::setPrintRange(PrintRange)
    obj1.setPrintRange(QPrinter::PrintRange(QPrinter::AllPages));
    QCOMPARE(QPrinter::PrintRange(QPrinter::AllPages), obj1.printRange());
    obj1.setPrintRange(QPrinter::PrintRange(QPrinter::Selection));
    QCOMPARE(QPrinter::PrintRange(QPrinter::Selection), obj1.printRange());
    obj1.setPrintRange(QPrinter::PrintRange(QPrinter::PageRange));
    QCOMPARE(QPrinter::PrintRange(QPrinter::PageRange), obj1.printRange());
}

tst_QPrinter::tst_QPrinter()
{
}

tst_QPrinter::~tst_QPrinter()
{

}

// initTestCase will be executed once before the first testfunction is executed.
void tst_QPrinter::initTestCase()
{
// TODO: Add testcase generic initialization code here.
// suggestion:
//    testWidget = new QPrinter(0,"testWidget");
//    testWidget->setFixedSize(200, 200);
//    qApp->setMainWidget(testWidget);
//    testWidget->show();
}

// cleanupTestCase will be executed once after the last testfunction is executed.
void tst_QPrinter::cleanupTestCase()
{
// TODO: Add testcase generic cleanup code here.
// suggestion:
//    testWidget->hide();
//    qApp->setMainWidget(0);
//    delete testWidget;
}

// init() will be executed immediately before each testfunction is run.
void tst_QPrinter::init()
{
// TODO: Add testfunction specific initialization code here.
}

// cleanup() will be executed immediately after each testfunction is run.
void tst_QPrinter::cleanup()
{
// TODO: Add testfunction specific cleanup code here.
}

#define MYCOMPARE(a, b) QCOMPARE(QVariant((int)a), QVariant((int)b))

void tst_QPrinter::testPageSetupDialog()
{
#if QT_VERSION < 0x040102
    QSKIP("QPrinter::OutputFormat doesn't exist in Qt 4.0.x and test crashes on < 4.1.2");
#else

    // Make sure this doesn't crash at least
    {
        QPrinter printer;
        QPageSetupDialog dialog(&printer);
    }

    // Check that we don't support page setup dialogs on Pdf printers
    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        QPageSetupDialog dialog(&printer);
        QCOMPARE(dialog.exec(), int(QDialog::Rejected));
    }

#endif
}



void tst_QPrinter::testPageSize()
{
#if defined (Q_WS_WIN)
    QPrinter prn;

    prn.setPageSize(QPrinter::Letter);
    MYCOMPARE(prn.pageSize(), QPrinter::Letter);
    MYCOMPARE(prn.winPageSize(), DMPAPER_LETTER);

    prn.setPageSize(QPrinter::A4);
    MYCOMPARE(prn.pageSize(), QPrinter::A4);
    MYCOMPARE(prn.winPageSize(), DMPAPER_A4);

    prn.setWinPageSize(DMPAPER_LETTER);
    MYCOMPARE(prn.winPageSize(), DMPAPER_LETTER);
    MYCOMPARE(prn.pageSize(), QPrinter::Letter);

    prn.setWinPageSize(DMPAPER_A4);
    MYCOMPARE(prn.winPageSize(), DMPAPER_A4);
    MYCOMPARE(prn.pageSize(), QPrinter::A4);
#else
    QSKIP("QPrinter::winPageSize() does not exist for nonwindows platforms", SkipAll);
#endif
}

void tst_QPrinter::testPageRectAndPaperRect_data()
{
    QTest::addColumn<int>("orientation");
    QTest::addColumn<bool>("withPainter");
    QTest::addColumn<int>("resolution");
    QTest::addColumn<bool>("doPaperRect");

    // paperrect
    QTest::newRow("paperRect0") << int(QPrinter::Portrait) << true << 300 << true;
    QTest::newRow("paperRect1") << int(QPrinter::Portrait) << false << 300 << true;
    QTest::newRow("paperRect2") << int(QPrinter::Landscape) << true << 300 << true;
    QTest::newRow("paperRect3") << int(QPrinter::Landscape) << false << 300 << true;
    QTest::newRow("paperRect4") << int(QPrinter::Portrait) << true << 600 << true;
    QTest::newRow("paperRect5") << int(QPrinter::Portrait) << false << 600 << true;
    QTest::newRow("paperRect6") << int(QPrinter::Landscape) << true << 600 << true;
    QTest::newRow("paperRect7") << int(QPrinter::Landscape) << false << 600 << true;
    QTest::newRow("paperRect8") << int(QPrinter::Portrait) << true << 1200 << true;
    QTest::newRow("paperRect9") << int(QPrinter::Portrait) << false << 1200 << true;
    QTest::newRow("paperRect10") << int(QPrinter::Landscape) << true << 1200 << true;
    QTest::newRow("paperRect11") << int(QPrinter::Landscape) << false << 1200 << true;

    // page rect
    QTest::newRow("pageRect0") << int(QPrinter::Portrait) << true << 300 << false;
    QTest::newRow("pageRect1") << int(QPrinter::Portrait) << false << 300 << false;
    QTest::newRow("pageRect2") << int(QPrinter::Landscape) << true << 300 << false;
    QTest::newRow("pageRect3") << int(QPrinter::Landscape) << false << 300 << false;
    QTest::newRow("pageRect4") << int(QPrinter::Portrait) << true << 600 << false;
    QTest::newRow("pageRect5") << int(QPrinter::Portrait) << false << 600 << false;
    QTest::newRow("pageRect6") << int(QPrinter::Landscape) << true << 600 << false;
    QTest::newRow("pageRect7") << int(QPrinter::Landscape) << false << 600 << false;
    QTest::newRow("pageRect8") << int(QPrinter::Portrait) << true << 1200 << false;
    QTest::newRow("pageRect9") << int(QPrinter::Portrait) << false << 1200 << false;
    QTest::newRow("pageRect10") << int(QPrinter::Landscape) << true << 1200 << false;
    QTest::newRow("pageRect11") << int(QPrinter::Landscape) << false << 1200 << false;
}

void tst_QPrinter::testPageRectAndPaperRect()
{
#if QT_VERSION <= 0x040100
    QEXPECT_FAIL("", "Fixed in Qt 4.1.1", Continue);
#endif

    QFETCH(bool,  withPainter);
    QFETCH(int,  orientation);
    QFETCH(int, resolution);
    QFETCH(bool, doPaperRect);

    QPainter *painter = 0;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOrientation(QPrinter::Orientation(orientation));
    printer.setOutputToFile(true);
    printer.setOutputFileName("silly");

    QRect pageRect = doPaperRect ? printer.paperRect() : printer.pageRect();
    float inchesX = float(pageRect.width()) / float(printer.resolution());
    float inchesY = float(pageRect.height()) / float(printer.resolution());
    printer.setResolution(resolution);
    if (withPainter)
        painter = new QPainter(&printer);

    QRect otherRect = doPaperRect ? printer.paperRect() : printer.pageRect();
    float otherInchesX = float(otherRect.width()) / float(printer.resolution());
    float otherInchesY = float(otherRect.height()) / float(printer.resolution());
    if (painter != 0)
        delete painter;

    QVERIFY(qAbs(otherInchesX - inchesX) < 0.01);
    QVERIFY(qAbs(otherInchesY - inchesY) < 0.01);

    QVERIFY(printer.orientation() == QPrinter::Portrait || pageRect.width() > pageRect.height());
    QVERIFY(printer.orientation() != QPrinter::Portrait || pageRect.width() < pageRect.height());
}

void tst_QPrinter::testSetOptions()
{
    QPrinter prn;
    QPrintDialog dlg(&prn);

    // Verify default values
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintToFile), TRUE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintSelection), FALSE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintPageRange), TRUE);

    dlg.setEnabledOptions(QAbstractPrintDialog::PrintPageRange);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintToFile), FALSE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintSelection), FALSE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintPageRange), TRUE);

    dlg.setEnabledOptions((QAbstractPrintDialog::PrintDialogOptions(QAbstractPrintDialog::PrintSelection
                                                                    | QAbstractPrintDialog::PrintPageRange)));
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintToFile), FALSE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintSelection), TRUE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintPageRange), TRUE);

    dlg.setEnabledOptions(QAbstractPrintDialog::PrintSelection);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintToFile), FALSE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintSelection), TRUE);
    MYCOMPARE(dlg.isOptionEnabled(QAbstractPrintDialog::PrintPageRange), FALSE);
}

void tst_QPrinter::testMargins_data()
{
    QTest::addColumn<int>("orientation");
    QTest::addColumn<bool>("fullpage");
    QTest::addColumn<int>("pagesize");
    QTest::addColumn<int>("width");
    QTest::addColumn<int>("height");
    QTest::addColumn<bool>("withPainter");

    QTest::newRow("data0") << int(QPrinter::Portrait) << true << int(QPrinter::A4) << 210 << 297 << false;
    QTest::newRow("data1") << int(QPrinter::Landscape) << true << int(QPrinter::A4) << 297 << 210 << false;
    QTest::newRow("data2") << int(QPrinter::Landscape) << false << int(QPrinter::A4) << 297 << 210 << false;
    QTest::newRow("data3") << int(QPrinter::Portrait) << false << int(QPrinter::A4) << 210 << 297 << false;
    QTest::newRow("data4") << int(QPrinter::Portrait) << true << int(QPrinter::A4) << 210 << 297 << true;
    QTest::newRow("data5") << int(QPrinter::Landscape) << true << int(QPrinter::A4) << 297 << 210 << true;
    QTest::newRow("data6") << int(QPrinter::Landscape) << false << int(QPrinter::A4) << 297 << 210 << true;
    QTest::newRow("data7") << int(QPrinter::Portrait) << false << int(QPrinter::A4) << 210 << 297 << true;
}

void tst_QPrinter::testMargins()
{
    QFETCH(bool,  withPainter);
    QFETCH(int,  orientation);
    QFETCH(int,  pagesize);
    QFETCH(int,  width);
    QFETCH(int,  height);
    QFETCH(bool, fullpage);
    QPrinter printer;
    QPainter *painter = 0;
    printer.setOutputToFile(true);
    printer.setOutputFileName("silly");
    printer.setOrientation((QPrinter::Orientation)orientation);
    printer.setFullPage(fullpage);
    printer.setPageSize((QPrinter::PageSize)pagesize);
    if (withPainter)
	painter = new QPainter(&printer);
    Q3PaintDeviceMetrics metrics(&printer);
    int pwidth = metrics.width();
    int pheight = metrics.height();

    if (orientation == QPrinter::Portrait) {
	QVERIFY(pheight >= pwidth);
    } else {
	QVERIFY(pwidth >= pheight);
    }

    if (fullpage) {
	QCOMPARE(metrics.widthMM(), width);
	QCOMPARE(metrics.heightMM(), height);
    }
    if (painter)
        delete painter;
}

void tst_QPrinter::testNonExistentPrinter()
{
#if defined(Q_WS_X11) || defined(Q_WS_QWS) || defined(Q_OS_MAC)
    QSKIP("QPrinter::testNonExistentPrinter() is not relevant for X11/Embedded/Mac", SkipAll);
#else
    QPrinter printer;
    QPainter painter;

    // Make sure it doesn't crash on setting or getting properties
    printer.setPrinterName("some non existing printer");
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setFullPage(true);
    printer.pageSize();
    printer.orientation();
    printer.fullPage();
    printer.printerName();

    // nor metrics
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmWidth), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmHeight), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmWidthMM), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmHeightMM), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmNumColors), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmDepth), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmDpiX), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmDpiY), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmPhysicalDpiX), 0);
    QCOMPARE(printer.printEngine()->metric(QPaintDevice::PdmPhysicalDpiY), 0);

    QVERIFY(!painter.begin(&printer));
#endif
}


void tst_QPrinter::testMulitpleSets_data()
{
    QTest::addColumn<int>("resolution");
    QTest::addColumn<int>("pageSize");
    QTest::addColumn<int>("widthMMAfter");
    QTest::addColumn<int>("heightMMAfter");


    QTest::newRow("lowRes") << int(QPrinter::ScreenResolution) << int(QPrinter::A4) << 210 << 297;
    QTest::newRow("lowResLetter") << int(QPrinter::ScreenResolution) << int(QPrinter::Letter) << 216 << 279;
    QTest::newRow("lowResA5") << int(QPrinter::ScreenResolution) << int(QPrinter::A5) << 148 << 210;
    QTest::newRow("midRes") << int(QPrinter::PrinterResolution) << int(QPrinter::A4) << 210 << 297;
    QTest::newRow("midResLetter") << int(QPrinter::PrinterResolution) << int(QPrinter::Letter) << 216 << 279;
    QTest::newRow("midResA5") << int(QPrinter::PrinterResolution) << int(QPrinter::A5) << 148 << 210;
    QTest::newRow("highRes") << int(QPrinter::HighResolution) << int(QPrinter::A4) << 210 << 297;
    QTest::newRow("highResLetter") << int(QPrinter::HighResolution) << int(QPrinter::Letter) << 216 << 279;
    QTest::newRow("highResA5") << int(QPrinter::HighResolution) << int(QPrinter::A5) << 148 << 210;
}

static void computePageValue(const QPrinter &printer, int &retWidth, int &retHeight)
{
    const double Inch2MM = 25.4;

    double width = double(printer.paperRect().width()) / printer.logicalDpiX() * Inch2MM;
    double height = double(printer.paperRect().height()) / printer.logicalDpiY() * Inch2MM;
    retWidth = qRound(width);
    retHeight = qRound(height);
}

void tst_QPrinter::testMulitpleSets()
{
    // A very simple test, but Mac needs to have its format "validated" if the format is changed
    // This takes care of that.
    QFETCH(int, resolution);
    QFETCH(int, pageSize);
    QFETCH(int, widthMMAfter);
    QFETCH(int, heightMMAfter);


    QPrinter::PrinterMode mode = QPrinter::PrinterMode(resolution);
    QPrinter::PageSize printerPageSize = QPrinter::PageSize(pageSize);
    QPrinter printer(mode);
    printer.setFullPage(true);
#if QT_VERSION < 0x040103
    QEXPECT_FAIL("", "Fixed in Qt 4.1.3", SkipAll);
#endif

    int paperWidth, paperHeight;
    const int Tolerance = 2;

    computePageValue(printer, paperWidth, paperHeight);
    printer.setPageSize(printerPageSize);

    if (printer.pageSize() != printerPageSize) {
        QSKIP("Current page size is not supported on this printer", SkipSingle);
        return;
    }

    QCOMPARE(printer.widthMM(), widthMMAfter);
    QCOMPARE(printer.heightMM(), heightMMAfter);

    computePageValue(printer, paperWidth, paperHeight);

    QVERIFY(qAbs(paperWidth - widthMMAfter) <= 2);
    QVERIFY(qAbs(paperHeight - heightMMAfter) <= 2);

    // Set it again and see if it still works.
    printer.setPageSize(printerPageSize);
    QCOMPARE(printer.widthMM(), widthMMAfter);
    QCOMPARE(printer.heightMM(), heightMMAfter);

    printer.setOrientation(QPrinter::Landscape);
    computePageValue(printer, paperWidth, paperHeight);
    QVERIFY(qAbs(paperWidth - heightMMAfter) <= 2);
    QVERIFY(qAbs(paperHeight - widthMMAfter) <= 2);
}

void tst_QPrinter::changingOutputFormat()
{
    QPrinter p;
    p.setOutputFormat(QPrinter::PostScriptFormat);
    p.setPageSize(QPrinter::A8);
    p.setOutputFormat(QPrinter::PdfFormat);
    QCOMPARE(p.pageSize(), QPrinter::A8);
}

void tst_QPrinter::outputFormatFromSuffix()
{
    QPrinter p;
    QVERIFY(p.outputFormat() == QPrinter::NativeFormat);
    p.setOutputFileName("test.ps");
    QVERIFY(p.outputFormat() == QPrinter::PostScriptFormat);
    p.setOutputFileName("test.pdf");
    QVERIFY(p.outputFormat() == QPrinter::PdfFormat);
    p.setOutputFileName(QString());
    QVERIFY(p.outputFormat() == QPrinter::NativeFormat);
}

QTEST_MAIN(tst_QPrinter)
#include "tst_qprinter.moc"
