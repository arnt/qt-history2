/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qlayout.h>
#include "qstyle.h"
#include <qevent.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qstyleoption.h>
#include <qscrollbar.h>
#include <qprogressbar.h>

#include <qplastiquestyle.h>
#include <qwindowsstyle.h>
#include <qcdestyle.h>
#include <qmotifstyle.h>
#include <qcommonstyle.h>
#include <qstylefactory.h>

#include <qimagereader.h>
#include <qimagewriter.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qmdiarea.h>


#if QT_VERSION >= 0x040200
#include <QCleanlooksStyle>
#endif

#ifdef Q_WS_MAC
#include <QMacStyle>
#endif

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#include <QWindowsVistaStyle>
#endif

#include <qwidget.h>

//TESTED_CLASS=
//TESTED_FILES=gui/styles/qstyle.h gui/styles/qstyle.cpp gui/styles/qplastiquestyle.cpp gui/styles/qwindowsstyle.cpp gui/styles/qwindowsxpstyle.cpp gui/styles/qwindowsvistastyle.cpp gui/styles/qmotifstyle.cpp

class tst_QStyle : public QObject
{
    Q_OBJECT
public:
    tst_QStyle();
    virtual ~tst_QStyle();
private:
    void testAllFunctions(QStyle *);
    void testScrollBarSubControls(QStyle *);
    void testPainting(QStyle *style, const QString &platform);
private slots:
    void drawItemPixmap();
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testMotifStyle();
    void testPlastiqueStyle();
    void testWindowsStyle();
    void testCDEStyle();
    void testWindowsXPStyle();
    void testWindowsVistaStyle();
    void testCleanlooksStyle();
    void testMacStyle();
    void testStyleFactory();
    void pixelMetric();
    void progressBarChangeStyle();
private:
	void lineUpLayoutTest(QStyle *);
    QWidget *testWidget;
};


tst_QStyle::tst_QStyle()
{
    testWidget = 0;
}

tst_QStyle::~tst_QStyle()
{
}

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget* QWidget=0, const char* name=0 );
protected:
    void paintEvent( QPaintEvent* );
};

void tst_QStyle::init()
{
    testWidget = new MyWidget( 0, "testObject");
}

void tst_QStyle::cleanup()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QStyle::initTestCase()
{
}

void tst_QStyle::cleanupTestCase()
{
}

void tst_QStyle::testStyleFactory()
{
    QStringList keys = QStyleFactory::keys();
    QVERIFY(keys.contains("Motif"));
    QVERIFY(keys.contains("Cleanlooks"));
    QVERIFY(keys.contains("Plastique"));
    QVERIFY(keys.contains("CDE"));
    QVERIFY(keys.contains("Windows"));
    QVERIFY(keys.contains("Motif"));
#ifdef Q_WS_WIN
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP && 
        QSysInfo::WindowsVersion < QSysInfo::WV_NT_based)
        QVERIFY(keys.contains("WindowsXP"));
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && 
        QSysInfo::WindowsVersion < QSysInfo::WV_NT_based)
        QVERIFY(keys.contains("WindowsVista"));
#endif

    foreach (QString styleName , keys) {
        QStyle *style = QStyleFactory::create(styleName);
        QVERIFY(style != 0);
        delete style;
    }
}

void tst_QStyle::drawItemPixmap()
{
    testWidget->resize(300, 300);
    testWidget->show();
    QPixmap p("task_25863.png", "PNG");
    QPixmap actualPix = QPixmap::grabWidget(testWidget);
    QVERIFY(pixmapsAreEqual(&actualPix,&p));
    testWidget->hide();
}

void tst_QStyle::testAllFunctions(QStyle *style)
{
    QStyleOption opt;
    opt.init(testWidget);

    testWidget->setStyle(style);
    
    //Tests styleHint with default arguments for potential crashes
    for ( int hint = 0 ; hint < int(QStyle::SH_ScrollBar_RollBetweenButtons); ++hint) {
        style->styleHint(QStyle::StyleHint(hint));
        style->styleHint(QStyle::StyleHint(hint), &opt, testWidget);
    }
    
    //Tests pixelMetric with default arguments for potential crashes
    for ( int pm = 0 ; pm < int(QStyle::PM_DockWidgetTitleMargin); ++pm) {
        style->pixelMetric(QStyle::PixelMetric(pm));
        style->pixelMetric(QStyle::PixelMetric(pm), &opt, testWidget);        
    }

    //Tests drawControl with default arguments for potential crashes
    for ( int control = 0 ; control < int(QStyle::CE_ToolBar); ++control) {
        QPixmap surface(QSize(200, 200));
        QPainter painter(&surface);
        style->drawControl(QStyle::ControlElement(control), &opt, &painter, 0);
    }

    //Tests drawComplexControl with default arguments for potential crashes
    {
        QPixmap surface(QSize(200, 200));
        QPainter painter(&surface);
        QStyleOptionComboBox copt1;
        copt1.init(testWidget);
        
        QStyleOptionGroupBox copt2;
        copt2.init(testWidget);
        QStyleOptionSizeGrip copt3;
        copt3.init(testWidget);
        QStyleOptionSlider copt4;
        copt4.init(testWidget);
        copt4.minimum = 0;
        copt4.maximum = 100;
        copt4.tickInterval = 25;
        copt4.sliderValue = 50;
        QStyleOptionSpinBox copt5;
        copt5.init(testWidget);
        QStyleOptionTitleBar copt6;
        copt6.init(testWidget);
        QStyleOptionToolButton copt7;
        copt7.init(testWidget);
        QStyleOptionQ3ListView copt8;
        copt8.init(testWidget);
        copt8.items << QStyleOptionQ3ListViewItem();

        style->drawComplexControl(QStyle::CC_SpinBox, &copt5, &painter, 0);
        style->drawComplexControl(QStyle::CC_ComboBox, &copt1, &painter, 0);
        style->drawComplexControl(QStyle::CC_ScrollBar, &copt4, &painter, 0);
        style->drawComplexControl(QStyle::CC_Slider, &copt4, &painter, 0);
        style->drawComplexControl(QStyle::CC_ToolButton, &copt7, &painter, 0);
        style->drawComplexControl(QStyle::CC_TitleBar, &copt6, &painter, 0);
        style->drawComplexControl(QStyle::CC_GroupBox, &copt2, &painter, 0);
        style->drawComplexControl(QStyle::CC_Dial, &copt4, &painter, 0);
        style->drawComplexControl(QStyle::CC_Q3ListView, &copt8, &painter, 0);
    }

    //Check standard pixmaps/icons
    for ( int i = 0 ; i < int(QStyle::SP_ToolBarVerticalExtensionButton); ++i) {
        QPixmap pixmap = style->standardPixmap(QStyle::StandardPixmap(i));
        if (pixmap.isNull()) {
            qWarning("missing StandardPixmap: %d", i);
        }
        QIcon icn = style->standardIcon(QStyle::StandardPixmap(i));
        if (icn.isNull()) {
            qWarning("missing StandardIcon: %d", i);
        }
    }

    style->itemPixmapRect(QRect(0, 0, 100, 100), Qt::AlignHCenter, QPixmap(200, 200));
    style->itemTextRect(QFontMetrics(qApp->font()), QRect(0, 0, 100, 100), Qt::AlignHCenter, true, QString("Test"));
    
    testScrollBarSubControls(style);
}

void tst_QStyle::testScrollBarSubControls(QStyle *)
{
    QScrollBar scrollBar;
    scrollBar.show();
    const QStyleOptionSlider opt = qt_qscrollbarStyleOption(&scrollBar);
    foreach (int subControl, QList<int>() << 1 << 2 << 4 << 8) {
        QRect sr = testWidget->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                    QStyle::SubControl(subControl), &scrollBar);
        
        QVERIFY(sr.isNull() == false);
    }
}

void tst_QStyle::testPlastiqueStyle()
{
    QPlastiqueStyle pstyle;
    testAllFunctions(&pstyle);
    lineUpLayoutTest(&pstyle);
}

void tst_QStyle::testCleanlooksStyle()
{
#if QT_VERSION >= 0x040200
    QCleanlooksStyle cstyle;
    testAllFunctions(&cstyle);
#endif
    lineUpLayoutTest(&cstyle);
}

void tst_QStyle::testWindowsStyle()
{
    QWindowsStyle wstyle;
    testAllFunctions(&wstyle);
    lineUpLayoutTest(&wstyle);
}

void tst_QStyle::testWindowsXPStyle()
{
#ifdef Q_WS_WIN
    QWindowsXPStyle xpstyle;
    testAllFunctions(&xpstyle);
    lineUpLayoutTest(&xpstyle);
#endif
}

void writeImage(const QString &fileName, QImage image)
{
    QImageWriter imageWriter(fileName);
    imageWriter.setFormat("png");
    qDebug() << "result " << imageWriter.write(image);
}

QImage readImage(const QString &fileName)
{
    QImageReader reader(fileName);
    return reader.read();
}


void tst_QStyle::testWindowsVistaStyle()
{
#ifdef Q_WS_WIN
    QWindowsVistaStyle vistastyle;
    testAllFunctions(&vistastyle);

    if (QSysInfo::WindowsVersion == QSysInfo::WV_VISTA)
        testPainting(&vistastyle, "vista");
#endif
}

void comparePixmap(const QString &filename, const QPixmap &pixmap)
{
    QImage oldFile = readImage(filename);
    QPixmap oldPixmap = QPixmap::fromImage(oldFile);
    if (!oldFile.isNull())
        QVERIFY(pixmapsAreEqual(&pixmap, &oldPixmap));
    else
        writeImage(filename, pixmap.toImage());
}

void tst_QStyle::testPainting(QStyle *style, const QString &platform)
{
    //Test Menu
    QString fileName = "images/" + platform + "/menu.png"; 
    QMenu menu;
    menu.setStyle(style);
    menu.show();
    menu.addAction(new QAction("Test 1", &menu));
    menu.addAction(new QAction("Test 2", &menu));
    QPixmap pixmap = QPixmap::grabWidget(&menu);
    comparePixmap(fileName, pixmap);

    //Push button    
    fileName = "images/" + platform + "/button.png";
    QPushButton button("OK");
    button.setStyle(style);
    button.show();
    pixmap = QPixmap::grabWidget(&button);
    button.hide();
    comparePixmap(fileName, pixmap);

    //Push button    
    fileName = "images/" + platform + "/radiobutton.png";
    QRadioButton radiobutton("Check");
    radiobutton.setStyle(style);
    radiobutton.show();
    pixmap = QPixmap::grabWidget(&radiobutton);
    radiobutton.hide();
    comparePixmap(fileName, pixmap);

    //Combo box
    fileName = "images/" + platform + "/combobox.png";
    QComboBox combobox;
    combobox.setStyle(style);
    combobox.addItem("Test 1");
    combobox.addItem("Test 2");
    combobox.show();
    pixmap = QPixmap::grabWidget(&combobox);
    combobox.hide();
    comparePixmap(fileName, pixmap);

    //Spin box
    fileName = "images/" + platform + "/spinbox.png";
    QDoubleSpinBox spinbox;
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    spinbox.setStyle(style);
    spinbox.show();
    pixmap = QPixmap::grabWidget(&spinbox);
    spinbox.hide();
    comparePixmap(fileName, pixmap);
    QLocale::setDefault(QLocale::system());
    
    //Slider
    fileName = "images/" + platform + "/slider.png";
    QSlider slider;
    slider.setStyle(style);
    slider.show();
    pixmap = QPixmap::grabWidget(&slider);
    slider.hide();
    comparePixmap(fileName, pixmap);

    //Line edit
    fileName = "images/" + platform + "/lineedit.png";
    QLineEdit lineedit("Test text");
    lineedit.setStyle(style);
    lineedit.show();
    pixmap = QPixmap::grabWidget(&lineedit);
    lineedit.hide();
    comparePixmap(fileName, pixmap);

    //MDI
    fileName = "images/" + platform + "/mdi.png";
    QMdiArea mdiArea;
    mdiArea.addSubWindow(new QWidget(&mdiArea));
    mdiArea.resize(200, 200);
    mdiArea.setStyle(style);
    mdiArea.show();
    pixmap = QPixmap::grabWidget(&mdiArea);
    mdiArea.hide();
    comparePixmap(fileName, pixmap);
}

void tst_QStyle::testMacStyle()
{
#ifdef Q_WS_MAC
    QMacStyle mstyle;
    testAllFunctions(&mstyle);
    testPainting(&mstyle, "mac");
#endif
}

void tst_QStyle::testMotifStyle()
{
    QMotifStyle mstyle;
    testAllFunctions(&mstyle);
}

void tst_QStyle::testCDEStyle()
{
    QCDEStyle cstyle;
    testAllFunctions(&cstyle);
}

// Helper class...

MyWidget::MyWidget( QWidget* parent, const char* name )
    : QWidget( parent )
{
    setObjectName(name);
}

void MyWidget::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    QPixmap big(400,400);
    big.fill(Qt::green);
    style()->drawItemPixmap(&p, rect(), Qt::AlignCenter, big);
}


class Qt42Style : public QWindowsStyle
{
    Q_OBJECT
public:
    Qt42Style() : QWindowsStyle()
    {
        margin_toplevel = 10;
        margin = 5;
        spacing = 0;
    }

    virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
                            const QWidget * widget = 0 ) const;

    int margin_toplevel;
    int margin;
    int spacing;

};

int Qt42Style::pixelMetric(PixelMetric metric, const QStyleOption * option /*= 0*/,
                                   const QWidget * widget /*= 0*/ ) const
{
    switch (metric) {
        case QStyle::PM_DefaultTopLevelMargin:
            return margin_toplevel;
        break;
        case QStyle::PM_DefaultChildMargin:
            return margin;
        break;
        case QStyle::PM_DefaultLayoutSpacing:
            return spacing;
        break;
        default:
            break;
    }
    return QWindowsStyle::pixelMetric(metric, option, widget);
}


void tst_QStyle::pixelMetric()
{
    Qt42Style *style = new Qt42Style();
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultTopLevelMargin), 10);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultChildMargin), 5);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultLayoutSpacing), 0);

    style->margin_toplevel = 0;
    style->margin = 0;
    style->spacing = 0;
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultTopLevelMargin), 0);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultChildMargin), 0);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultLayoutSpacing), 0);

    style->margin_toplevel = -1;
    style->margin = -1;
    style->spacing = -1;
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultTopLevelMargin), -1);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultChildMargin), -1);
    QCOMPARE(style->pixelMetric(QStyle::PM_DefaultLayoutSpacing), -1);

    delete style;
}

void tst_QStyle::progressBarChangeStyle()
{
    //test a crashing situation (task 143530) 
    //where changing the styles and deleting a progressbar would crash
    
    QWindowsStyle style1;
    QPlastiqueStyle style2;

    QProgressBar *progress=new QProgressBar;
    progress->setStyle(&style1);

    progress->show();

    progress->setStyle(&style2);

    QTest::qWait(100);
    delete progress;

    QTest::qWait(100);

    //before the correction, there would be a crash here
}

void tst_QStyle::lineUpLayoutTest(QStyle *style)
{
	QWidget widget;
	QHBoxLayout layout;
	QFont font;
	font.setPointSize(9); //Plastique is lined up for odd numbers...
	widget.setFont(font);
	QSpinBox spinbox(&widget);
	QLineEdit lineedit(&widget);
	QComboBox combo(&widget);
	combo.setEditable(true);
	layout.addWidget(&spinbox);
	layout.addWidget(&lineedit);
	layout.addWidget(&combo);
	widget.setLayout(&layout);
        widget.setStyle(style);
        // propagate the style.
        foreach (QWidget *w, qFindChildren<QWidget *>(&widget))
            w->setStyle(style);
	widget.show();
        QTest::qWait( 500 );

	QVERIFY(qAbs(spinbox.height() - lineedit.height()) <= 1);
	QVERIFY(qAbs(spinbox.height() - combo.height()) <= 1);
}

QTEST_MAIN(tst_QStyle)
#include "tst_qstyle.moc"
