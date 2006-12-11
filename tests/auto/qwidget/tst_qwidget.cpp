/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <q3hbox.h>
#include <q3pointarray.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdebug.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qwidget.h>
#include <qwindowsstyle.h>
#include <qinputcontext.h>
#include <private/qstylesheetstyle_p.h>
#include <QDesktopWidget>

// I *MUST* have QtTest afterwards or this test won't work with newer headers
#if defined(Q_WS_MAC)
# include <private/qt_mac_p.h>
#undef verify
#endif

#include <QtTest/QtTest>

#if defined(Q_WS_WIN)
#  include <qt_windows.h>
#elif defined(Q_WS_X11)
#  include <private/qt_x11_p.h>
#  include <qx11info_x11.h>
#elif defined(Q_WS_QWS)
# include <qwindowsystem_qws.h>
#endif

#if defined(Bool)
#undef Bool
#endif

// Will try to wait for the condition while allowing event processing
// for a maximum of 2 seconds.
#define WAIT_FOR_CONDITION(expr, expected) \
    do { \
        const int step = 100; \
        for (int i = 0; i < 2000 && expr != expected; i+=step) { \
            QTest::qWait(step); \
        } \
    } while(0)

//TESTED_CLASS=
//TESTED_FILES=gui/kernel/qwidget.h gui/kernel/qwidget.cpp

class tst_QWidget : public QObject
{
    Q_OBJECT

public:
    tst_QWidget();
    virtual ~tst_QWidget();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void fontPropagation();
    void palettePropagation();
    void enabledPropagation();
    void acceptDropsPropagation();
    void isEnabledTo();
    void visible();
    void visible_setWindowOpacity();
    void isVisibleTo();
    void isHidden();
    void fonts();
    void mapToGlobal();
    void checkFocus();
    void focusChainOnHide();
    void focusChainOnReparent();
    void setTabOrder();
    void activation();
    void reparent();
    void windowState();
    void showMaximized();
    void showFullScreen();
    void showMinimized();
    void icon();
    void hideWhenFocusWidgetIsChild();
    void normalGeometry();
    void setGeometry();
    void windowOpacity();
    void raise();
    void lower();
    void stackUnder();
    void testContentsPropagation();
    void saveRestoreGeometry();
    void restoreVersion1Geometry();

    void windowTitle();
    void windowModified();
    void windowIconText();

    void widgetAt();
#ifdef Q_WS_MAC
    void retainHIView();
#endif
    void resizeEvent();
    void task110173();

    void testDeletionInEventHandlers();
    void style();

    void childDeletesItsSibling();

    void setMinimumSize();
    void setMaximumSize();
    void setFixedSize();

    void ensureCreated();
    void persistentWinId();
    void qobject_castInDestroyedSlot();

    void showHideEvent_data();
    void showHideEvent();

    // tests QWidget::setGeometry() on windows only
    void setWindowGeometry_data();
    void setWindowGeometry();

    // tests QWidget::move() on windows only
    void windowMove_data();
    void windowMove();

#ifdef Q_WS_WIN
    void getDC();
#endif

    void setLocale();

private:
    QWidget *testWidget;
};

class MyInputContext : public QInputContext
{
public:
    MyInputContext() : QInputContext() {}
    QString identifierName() { return QString("NoName"); }
    QString language() { return QString("NoLanguage"); }
    void reset() {}
    bool isComposing() const { return false; }
};

// Testing get/set functions
void tst_QWidget::getSetCheck()
{
    QWidget obj1;
    QWidget child1(&obj1);
    // QStyle * QWidget::style()
    // void QWidget::setStyle(QStyle *)
    QWindowsStyle *var1 = new QWindowsStyle;
    obj1.setStyle(var1);
    QCOMPARE(var1, obj1.style());
    obj1.setStyle((QStyle *)0);
    QVERIFY(var1 != obj1.style());
    QVERIFY(0 != obj1.style()); // style can never be 0 for a widget

    // int QWidget::minimumWidth()
    // void QWidget::setMinimumWidth(int)
    obj1.setMinimumWidth(0);
    QCOMPARE(obj1.minimumWidth(), 0);
    obj1.setMinimumWidth(INT_MIN);
    QCOMPARE(obj1.minimumWidth(), 0); // A widgets width can never be less than 0
    obj1.setMinimumWidth(INT_MAX);
#ifndef Q_WS_QWS  //QWS doesn't allow toplevels to be bigger than the screen
    QCOMPARE(obj1.minimumWidth(), QWIDGETSIZE_MAX); // The largest minimum size should only be as big as the maximium
#endif

    child1.setMinimumWidth(0);
    QCOMPARE(child1.minimumWidth(), 0);
    child1.setMinimumWidth(INT_MIN);
    QCOMPARE(child1.minimumWidth(), 0); // A widgets width can never be less than 0
    child1.setMinimumWidth(INT_MAX);
    QCOMPARE(child1.minimumWidth(), QWIDGETSIZE_MAX); // The largest minimum size should only be as big as the maximium


    // int QWidget::minimumHeight()
    // void QWidget::setMinimumHeight(int)
    obj1.setMinimumHeight(0);
    QCOMPARE(obj1.minimumHeight(), 0);
    obj1.setMinimumHeight(INT_MIN);
    QCOMPARE(obj1.minimumHeight(), 0); // A widgets height can never be less than 0
    obj1.setMinimumHeight(INT_MAX);
#ifndef Q_WS_QWS    //QWS doesn't allow toplevels to be bigger than the screen
    QCOMPARE(obj1.minimumHeight(), QWIDGETSIZE_MAX); // The largest minimum size should only be as big as the maximium
#endif

    child1.setMinimumHeight(0);
    QCOMPARE(child1.minimumHeight(), 0);
    child1.setMinimumHeight(INT_MIN);
    QCOMPARE(child1.minimumHeight(), 0); // A widgets height can never be less than 0
    child1.setMinimumHeight(INT_MAX);
    QCOMPARE(child1.minimumHeight(), QWIDGETSIZE_MAX); // The largest minimum size should only be as big as the maximium


// int QWidget::maximumWidth()
    // void QWidget::setMaximumWidth(int)
    obj1.setMaximumWidth(0);
    QCOMPARE(obj1.maximumWidth(), 0);
    obj1.setMaximumWidth(INT_MIN);
    QCOMPARE(obj1.maximumWidth(), 0); // A widgets width can never be less than 0
    obj1.setMaximumWidth(INT_MAX);
    QCOMPARE(obj1.maximumWidth(), QWIDGETSIZE_MAX); // QWIDGETSIZE_MAX is the abs max, not INT_MAX

    // int QWidget::maximumHeight()
    // void QWidget::setMaximumHeight(int)
    obj1.setMaximumHeight(0);
    QCOMPARE(obj1.maximumHeight(), 0);
    obj1.setMaximumHeight(INT_MIN);
    QCOMPARE(obj1.maximumHeight(), 0); // A widgets height can never be less than 0
    obj1.setMaximumHeight(INT_MAX);
    QCOMPARE(obj1.maximumHeight(), QWIDGETSIZE_MAX); // QWIDGETSIZE_MAX is the abs max, not INT_MAX

    // back to normal
    obj1.setMinimumWidth(0);
    obj1.setMinimumHeight(0);
    obj1.setMaximumWidth(QWIDGETSIZE_MAX);
    obj1.setMaximumHeight(QWIDGETSIZE_MAX);

    // const QPalette & QWidget::palette()
    // void QWidget::setPalette(const QPalette &)
    QPalette var6;
    obj1.setPalette(var6);
    QCOMPARE(var6, obj1.palette());
    obj1.setPalette(QPalette());
    QCOMPARE(QPalette(), obj1.palette());

    // const QFont & QWidget::font()
    // void QWidget::setFont(const QFont &)
    QFont var7;
    obj1.setFont(var7);
    QCOMPARE(var7, obj1.font());
    obj1.setFont(QFont());
    QCOMPARE(QFont(), obj1.font());

    // qreal QWidget::windowOpacity()
    // void QWidget::setWindowOpacity(qreal)
    obj1.setWindowOpacity(0.0);
    QCOMPARE(0.0, obj1.windowOpacity());
    obj1.setWindowOpacity(1.1);
    QCOMPARE(1.0, obj1.windowOpacity()); // 1.0 is the fullest opacity possible

    // QWidget * QWidget::focusProxy()
    // void QWidget::setFocusProxy(QWidget *)
    QWidget *var9 = new QWidget();
    obj1.setFocusProxy(var9);
    QCOMPARE(var9, obj1.focusProxy());
    obj1.setFocusProxy((QWidget *)0);
    QCOMPARE((QWidget *)0, obj1.focusProxy());
    delete var9;

    // const QRect & QWidget::geometry()
    // void QWidget::setGeometry(const QRect &)
    qApp->processEvents();
    QRect var10(10, 10, 100, 100);
    obj1.setGeometry(var10);
    qApp->processEvents();
    qDebug() << obj1.geometry();
    QCOMPARE(var10, obj1.geometry());
    obj1.setGeometry(QRect(0,0,0,0));
    qDebug() << obj1.geometry();
    QCOMPARE(QRect(0,0,0,0), obj1.geometry());

    // QLayout * QWidget::layout()
    // void QWidget::setLayout(QLayout *)
    QBoxLayout *var11 = new QBoxLayout(QBoxLayout::LeftToRight);
    obj1.setLayout(var11);
    QCOMPARE(var11, obj1.layout());
    obj1.setLayout((QLayout *)0);
    QCOMPARE(var11, obj1.layout()); // You cannot set a 0-pointer layout, that keeps the current
    delete var11; // This will remove the layout from the widget
    QCOMPARE((QLayout *)0, obj1.layout());

    // bool QWidget::acceptDrops()
    // void QWidget::setAcceptDrops(bool)
    obj1.setAcceptDrops(false);
    QCOMPARE(false, obj1.acceptDrops());
    obj1.setAcceptDrops(true);
    QCOMPARE(true, obj1.acceptDrops());

    // QInputContext * QWidget::inputContext()
    // void QWidget::setInputContext(QInputContext *)
    MyInputContext *var13 = new MyInputContext;
    obj1.setInputContext(var13);
    QCOMPARE((QInputContext *)0, obj1.inputContext()); // The widget by default doesn't have the WA_InputMethodEnabled attribute
    obj1.setAttribute(Qt::WA_InputMethodEnabled);
    obj1.setInputContext(var13);
    QCOMPARE(var13, obj1.inputContext());
    obj1.setInputContext((QInputContext *)0);
    QCOMPARE(qApp->inputContext(), obj1.inputContext());
    QVERIFY(qApp->inputContext() != var13);
    //delete var13; // No delete, since QWidget takes ownership

    // bool QWidget::autoFillBackground()
    // void QWidget::setAutoFillBackground(bool)
    obj1.setAutoFillBackground(false);
    QCOMPARE(false, obj1.autoFillBackground());
    obj1.setAutoFillBackground(true);
    QCOMPARE(true, obj1.autoFillBackground());

    delete var1;
}

#ifdef Q_WS_X11
extern void qt_x11_wait_for_window_manager( QWidget* w );
#endif

tst_QWidget::tst_QWidget()
{
    testWidget = 0;
}

tst_QWidget::~tst_QWidget()
{
}

class BezierViewer : public QWidget {
public:
    BezierViewer( QWidget* parent=0, const char* name=0 );
    void paintEvent( QPaintEvent* );
    void setPoints( const Q3PointArray& a );
private:
    Q3PointArray points;
};

void tst_QWidget::initTestCase()
{
  // Create the test class
    testWidget = new BezierViewer( 0, "testObject");
    testWidget->resize(200,200);
#ifdef QT3_SUPPORT
    qApp->setMainWidget(testWidget);
#endif
    testWidget->show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(testWidget);
#endif
}

void tst_QWidget::cleanupTestCase()
{
    delete testWidget;
    testWidget = 0;
}

void tst_QWidget::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
    testWidget->unsetFont();
    testWidget->unsetPalette();
}

void tst_QWidget::cleanup()
{
}

// Helper class...

BezierViewer::BezierViewer( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
    setBackgroundColor( Qt::white );
}

void BezierViewer::setPoints( const Q3PointArray& a )
{
    points = a;
}

void BezierViewer::paintEvent( QPaintEvent* )
{
    if ( points.size() != 4 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "Q3PointArray::bezier: The array must have 4 control points" );
#endif
	return;
    }

    /* Calculate Bezier curve */
    QPolygon bezier = points.cubicBezier();

    QPainter painter( this );

    /* Calculate scale to fit in window */
    QRect br = bezier.boundingRect() | points.boundingRect();
    QRect pr = rect();
    int scl = QMAX( QMIN(pr.width()/br.width(), pr.height()/br.height()), 1 );
    int border = scl-1;

    /* Scale Bezier curve vertices */
    for ( QPolygon::Iterator it = bezier.begin(); it != bezier.end(); ++it ) {
	it->setX( (it->x()-br.x()) * scl + border );
	it->setY( (it->y()-br.y()) * scl + border );
    }

    /* Draw grid */
    painter.setPen( Qt::lightGray );
	int i;
	for ( i = border; i <= pr.width(); i += scl ) {
		painter.drawLine( i, 0, i, pr.height() );
    }
    for ( int j = border; j <= pr.height(); j += scl ) {
	painter.drawLine( 0, j, pr.width(), j );
    }

    /* Write number of vertices */
    painter.setPen( Qt::red );
    painter.setFont( QFont("Helvetica", 14, QFont::DemiBold, TRUE ) );
    QString caption;
    caption.setNum( bezier.size() );
    caption += QString::fromLatin1( " vertices" );
    painter.drawText( 10, pr.height()-10, caption );

    /* Draw Bezier curve */
    painter.setPen( Qt::black );
    painter.drawPolyline( bezier );

    /* Scale and draw control points */
    painter.setPen( Qt::darkGreen );
    for ( QPolygon::Iterator p1 = points.begin(); p1 != points.end(); ++p1 ) {
	int x = (p1->x()-br.x()) * scl + border;
	int y = (p1->y()-br.y()) * scl + border;
	painter.drawLine( x-4, y-4, x+4, y+4 );
	painter.drawLine( x+4, y-4, x-4, y+4 );
    }

    /* Draw vertices */
    painter.setPen( Qt::red );
    painter.setBrush( Qt::red );
    for ( QPolygon::Iterator p2 = bezier.begin(); p2 != bezier.end(); ++p2 )
	painter.drawEllipse( p2->x()-1, p2->y()-1, 3, 3 );
}

void tst_QWidget::fontPropagation()
{
    QFont font = testWidget->font();
    QWidget* childWidget = new QWidget( testWidget );
    childWidget->show();
    QCOMPARE( font, childWidget->font() );

    font.setBold( TRUE );
    testWidget->setFont( font );
    QCOMPARE( font, testWidget->font() );
    QCOMPARE( font, childWidget->font() );

    QFont newFont = font;
    newFont.setItalic( TRUE );
    childWidget->setFont( newFont );
    QWidget* grandChildWidget = new QWidget( childWidget );
    QCOMPARE( font, testWidget->font() );
    QCOMPARE( newFont, grandChildWidget->font() );

    font.setUnderline( TRUE );
    testWidget->setFont( font );

    // the child and grand child should now have merged bold and
    // underline
    newFont.setUnderline( TRUE );

    QCOMPARE( newFont, childWidget->font() );
    QCOMPARE( newFont, grandChildWidget->font() );

    // make sure font propagation continues working after reparenting
    font = testWidget->font();
    font.setPointSize(font.pointSize() + 2);
    testWidget->setFont(font);

    QWidget *one   = new QWidget(testWidget);
    QWidget *two   = new QWidget(one);
    QWidget *three = new QWidget(two);
    QWidget *four  = new QWidget(two);

    four->reparent(three, QPoint(0, 0));

    font.setPointSize(font.pointSize() + 2);
    testWidget->setFont(font);

    QCOMPARE(testWidget->font(), one->font());
    QCOMPARE(one->font(), two->font());
    QCOMPARE(two->font(), three->font());
    QCOMPARE(three->font(), four->font());

    QVERIFY(testWidget->ownFont());
    QVERIFY(! one->ownFont());
    QVERIFY(! two->ownFont());
    QVERIFY(! three->ownFont());
    QVERIFY(! four->ownFont());

    font.setPointSize(font.pointSize() + 2);
    one->setFont(font);

    QCOMPARE(one->font(), two->font());
    QCOMPARE(two->font(), three->font());
    QCOMPARE(three->font(), four->font());

    QVERIFY(one->ownFont());
    QVERIFY(! two->ownFont());
    QVERIFY(! three->ownFont());
    QVERIFY(! four->ownFont());

    font.setPointSize(font.pointSize() + 2);
    two->setFont(font);

    QCOMPARE(two->font(), three->font());
    QCOMPARE(three->font(), four->font());

    QVERIFY(two->ownFont());
    QVERIFY(! three->ownFont());
    QVERIFY(! four->ownFont());

    font.setPointSize(font.pointSize() + 2);
    three->setFont(font);

    QCOMPARE(three->font(), four->font());

    QVERIFY(three->ownFont());
    QVERIFY(! four->ownFont());

    font.setPointSize(font.pointSize() + 2);
    four->setFont(font);

    QVERIFY(four->ownFont());
}

void tst_QWidget::palettePropagation()
{
    QPalette palette = testWidget->palette();
    QWidget* childWidget = new QWidget( testWidget );
    childWidget->show();
    QCOMPARE( palette, childWidget->palette() );

    palette.setColor( QColorGroup::Base, Qt::red );
    testWidget->setPalette( palette );
    QCOMPARE( palette, testWidget->palette() );
    QCOMPARE( palette, childWidget->palette() );

    QPalette newPalette = palette;
    newPalette.setColor( QColorGroup::Highlight, Qt::green );
    childWidget->setPalette( newPalette );
    QWidget* grandChildWidget = new QWidget( childWidget );
    QCOMPARE( palette, testWidget->palette() );
    QCOMPARE( newPalette, grandChildWidget->palette() );

    palette.setColor( QColorGroup::Text, Qt::blue );
    testWidget->setPalette( palette );

    // the child and grand child should now have merged green
    // highlight and blue text
    newPalette.setColor( QColorGroup::Text, Qt::blue);

    QCOMPARE( newPalette, childWidget->palette() );
    QCOMPARE( newPalette, grandChildWidget->palette() );
}

void tst_QWidget::enabledPropagation()
{
    QWidget* childWidget = new QWidget( testWidget );
    childWidget->show();
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );

    testWidget->setEnabled( FALSE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );

    testWidget->setDisabled( FALSE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );

    QWidget* grandChildWidget = new QWidget( childWidget );
    QVERIFY( grandChildWidget->isEnabled() );

    testWidget->setDisabled( TRUE );
    QVERIFY( !testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( FALSE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );

    grandChildWidget->setEnabled( TRUE );
    testWidget->setEnabled( FALSE );
    childWidget->setDisabled( TRUE );
    testWidget->setEnabled( TRUE );
    QVERIFY( testWidget->isEnabled() );
    QVERIFY( !childWidget->isEnabled() );
    QVERIFY( !grandChildWidget->isEnabled() );
}

void tst_QWidget::acceptDropsPropagation()
{
    QWidget *childWidget = new QWidget(testWidget);
    childWidget->show();
    QVERIFY(!testWidget->acceptDrops());
    QVERIFY(!childWidget->acceptDrops());

    testWidget->setAcceptDrops(true);
    QVERIFY(testWidget->acceptDrops());
    QVERIFY(!childWidget->acceptDrops());
    QVERIFY(childWidget->testAttribute(Qt::WA_DropSiteRegistered));

    testWidget->setAcceptDrops(false);
    QVERIFY(!testWidget->acceptDrops());
    QVERIFY(!childWidget->acceptDrops());
    QVERIFY(!childWidget->testAttribute(Qt::WA_DropSiteRegistered));

    QWidget *grandChildWidget = new QWidget(childWidget);
    QVERIFY(!grandChildWidget->acceptDrops());
    QVERIFY(!grandChildWidget->testAttribute(Qt::WA_DropSiteRegistered));

    testWidget->setAcceptDrops(true);
    QVERIFY(testWidget->acceptDrops());
    QVERIFY(!childWidget->acceptDrops());
    QVERIFY(childWidget->testAttribute(Qt::WA_DropSiteRegistered));
    QVERIFY(!grandChildWidget->acceptDrops());
    QVERIFY(grandChildWidget->testAttribute(Qt::WA_DropSiteRegistered));

    grandChildWidget->setAcceptDrops(true);
    testWidget->setAcceptDrops(false);
    QVERIFY(!testWidget->acceptDrops());
    QVERIFY(!childWidget->acceptDrops());
    QVERIFY(grandChildWidget->acceptDrops());
    QVERIFY(grandChildWidget->testAttribute(Qt::WA_DropSiteRegistered));


    grandChildWidget->setAcceptDrops(false);
    QVERIFY(!grandChildWidget->testAttribute(Qt::WA_DropSiteRegistered));
    testWidget->setAcceptDrops(true);
    childWidget->setAcceptDrops(true);
    testWidget->setAcceptDrops(false);
    QVERIFY(!testWidget->acceptDrops());
    QVERIFY(childWidget->acceptDrops());
    QVERIFY(!grandChildWidget->acceptDrops());
    QVERIFY(grandChildWidget->testAttribute(Qt::WA_DropSiteRegistered));
}

void tst_QWidget::isEnabledTo()
{
    QWidget* childWidget = new QWidget( testWidget );
    QWidget* grandChildWidget = new QWidget( childWidget );

    QVERIFY( childWidget->isEnabledTo( testWidget ) );
    QVERIFY( grandChildWidget->isEnabledTo( testWidget ) );

    childWidget->setEnabled( FALSE );
    QVERIFY( !childWidget->isEnabledTo( testWidget ) );
    QVERIFY( grandChildWidget->isEnabledTo( childWidget ) );
    QVERIFY( !grandChildWidget->isEnabledTo( testWidget ) );
}

void tst_QWidget::visible()
{
    // Ensure that the testWidget is hidden for this test at the
    // start

    testWidget->hide();
    QVERIFY( !testWidget->isVisible() );
    QWidget* childWidget = new QWidget( testWidget );
    QVERIFY( !childWidget->isVisible() );

    testWidget->show();
    QVERIFY( testWidget->isVisible() );
    QVERIFY( childWidget->isVisible() );

    QWidget* grandChildWidget = new QWidget( childWidget );
    QVERIFY( !grandChildWidget->isVisible() );
    grandChildWidget->show();
    QVERIFY( grandChildWidget->isVisible() );

    grandChildWidget->hide();
    testWidget->hide();
    testWidget->show();
    QVERIFY( !grandChildWidget->isVisible() );
    QVERIFY( testWidget->isVisible() );
    QVERIFY( childWidget->isVisible() );

    grandChildWidget->show();
    childWidget->hide();
    testWidget->hide();
    testWidget->show();
    QVERIFY( testWidget->isVisible() );
    QVERIFY( !childWidget->isVisible() );
    QVERIFY( !grandChildWidget->isVisible() );

    grandChildWidget->show();
    QVERIFY( !grandChildWidget->isVisible() );
}

void tst_QWidget::setLocale()
{
    QWidget w;
    QCOMPARE(w.locale(), QLocale());

    w.setLocale(QLocale::Italian);
    QCOMPARE(w.locale(), QLocale(QLocale::Italian));

    QWidget child1(&w);
    QCOMPARE(child1.locale(), QLocale(QLocale::Italian));

    w.unsetLocale();
    QCOMPARE(w.locale(), QLocale());
    QCOMPARE(child1.locale(), QLocale());

    w.setLocale(QLocale::French);
    QCOMPARE(w.locale(), QLocale(QLocale::French));
    QCOMPARE(child1.locale(), QLocale(QLocale::French));

    child1.setLocale(QLocale::Italian);
    QCOMPARE(w.locale(), QLocale(QLocale::French));
    QCOMPARE(child1.locale(), QLocale(QLocale::Italian));

    child1.unsetLocale();
    QCOMPARE(w.locale(), QLocale(QLocale::French));
    QCOMPARE(child1.locale(), QLocale(QLocale::French));

    QWidget child2;
    QCOMPARE(child2.locale(), QLocale());
    child2.setParent(&w);
    QCOMPARE(child2.locale(), QLocale(QLocale::French));
}

void tst_QWidget::visible_setWindowOpacity()
{
    testWidget->hide();
    QVERIFY( !testWidget->isVisible() );
    testWidget->setWindowOpacity(0.5);
#ifdef Q_OS_WIN
    QVERIFY(::IsWindowVisible(testWidget->winId()) ==  FALSE);
#endif
    testWidget->setWindowOpacity(1.0);
}

void tst_QWidget::isVisibleTo()
{
    // Ensure that the testWidget is hidden for this test at the
    // start

    testWidget->hide();
    QWidget* childWidget = new QWidget( testWidget );
    QVERIFY( childWidget->isVisibleTo( testWidget ) );
    childWidget->hide();
    QVERIFY( !childWidget->isVisibleTo( testWidget ) );

    QWidget* grandChildWidget = new QWidget( childWidget );
    QVERIFY( !grandChildWidget->isVisibleTo( testWidget ) );
    QVERIFY( grandChildWidget->isVisibleTo( childWidget ) );

    testWidget->show();
    childWidget->show();

    QVERIFY( childWidget->isVisibleTo( testWidget ) );
    grandChildWidget->hide();
    QVERIFY( !grandChildWidget->isVisibleTo( childWidget ) );
    QVERIFY( !grandChildWidget->isVisibleTo( testWidget ) );

}

void tst_QWidget::isHidden()
{
    // Ensure that the testWidget is hidden for this test at the
    // start

    testWidget->hide();
    QVERIFY( testWidget->isHidden() );
    QWidget* childWidget = new QWidget( testWidget );
    QVERIFY( !childWidget->isHidden() );

    testWidget->show();
    QVERIFY( !testWidget->isHidden() );
    QVERIFY( !childWidget->isHidden() );

    QWidget* grandChildWidget = new QWidget( childWidget );
    QVERIFY( grandChildWidget->isHidden() );
    grandChildWidget->show();
    QVERIFY( !grandChildWidget->isHidden() );

    grandChildWidget->hide();
    testWidget->hide();
    testWidget->show();
    QVERIFY( grandChildWidget->isHidden() );
    QVERIFY( !testWidget->isHidden() );
    QVERIFY( !childWidget->isHidden() );

    grandChildWidget->show();
    childWidget->hide();
    testWidget->hide();
    testWidget->show();
    QVERIFY( !testWidget->isHidden() );
    QVERIFY( childWidget->isHidden() );
    QVERIFY( !grandChildWidget->isHidden() );

    grandChildWidget->show();
    QVERIFY( !grandChildWidget->isHidden() );
}

void tst_QWidget::fonts()
{
    // Tests setFont(), ownFont() and unsetFont()
    QWidget* cleanTestWidget = new QWidget( testWidget );
    QFont originalFont = cleanTestWidget->font();

    QVERIFY( !cleanTestWidget->ownFont() );
    cleanTestWidget->unsetFont();
    QVERIFY( !cleanTestWidget->ownFont() );

    QFont newFont( "times", 18 );
    cleanTestWidget->setFont( newFont );
    newFont = newFont.resolve( testWidget->font() );

    QVERIFY( cleanTestWidget->ownFont() );
    QVERIFY( cleanTestWidget->font() == newFont );

    cleanTestWidget->unsetFont();
    QVERIFY( !cleanTestWidget->ownFont() );
    QVERIFY( cleanTestWidget->font() == originalFont );
}

void tst_QWidget::mapToGlobal()
{
    QPoint vis = testWidget->mapToGlobal(QPoint(0,0));
    testWidget->hide();
    QCOMPARE(testWidget->mapToGlobal(QPoint(0,0)), vis);
    testWidget->show();

    // test in a layout and witha move
    Q3HBox * qhb = new Q3HBox(testWidget);
    QWidget * qw = new QWidget(qhb);
    qw->move(6,12);
    QPoint wVis = qw->mapToGlobal(QPoint(0,0));
    qw->hide();
    QCOMPARE(qw->mapToGlobal(QPoint(0,0)), wVis);
    delete qhb;

}

void tst_QWidget::focusChainOnReparent()
{
    QWidget window;
    QWidget *child1 = new QWidget(&window);
    QWidget *child2 = new QWidget(&window);
    QWidget *child3 = new QWidget(&window);
    QWidget *child21 = new QWidget(child2);
    QWidget *child22 = new QWidget(child2);
    QWidget *child4 = new QWidget(&window);

    QWidget *expectedOriginalChain[8] = {&window, child1,  child2,  child3,  child21, child22, child4, &window};
    QWidget *w = &window;
    for (int i = 0; i <8; ++i) {
        QCOMPARE(w, expectedOriginalChain[i]);
        w = w->nextInFocusChain();
    }

    QWidget window2;
    child2->setParent(&window2);

    QWidget *expectedNewChain[5] = {&window2, child2,  child21, child22, &window2};
    w = &window2;
    for (int i = 0; i <5; ++i) {
        QCOMPARE(w, expectedNewChain[i]);
        w = w->nextInFocusChain();
    }

    QWidget *expectedOldChain[5] = {&window, child1,  child3, child4, &window};
    w = &window;
    for (int i = 0; i <5; ++i) {
        QCOMPARE(w, expectedOldChain[i]);
        w = w->nextInFocusChain();
    }
}


void tst_QWidget::focusChainOnHide()
{
    testWidget->hide(); // We do not want to get disturbed by other widgets
    // focus should move to the next widget in the focus chain when we hide it.
    QWidget *parent = new QWidget();
    parent->setObjectName(QLatin1String("Parent"));
    parent->setFocusPolicy(Qt::StrongFocus);
    QWidget *child = new QWidget(parent);
    child->setObjectName(QLatin1String("child"));
    child->setFocusPolicy(Qt::StrongFocus);
    QWidget::setTabOrder(child, parent);

    parent->show();
    qApp->setActiveWindow(parent->window());
    child->activateWindow();
    child->setFocus();
    qApp->processEvents();

    WAIT_FOR_CONDITION(child->hasFocus(), true);
    QCOMPARE(child->hasFocus(), true);
    child->hide();
    qApp->processEvents();

    WAIT_FOR_CONDITION(parent->hasFocus(), true);
    QCOMPARE(parent->hasFocus(), true);
    QCOMPARE(parent, qApp->focusWidget());

    delete parent;
    testWidget->show(); //don't disturb later tests
}

void tst_QWidget::checkFocus()
{
    // This is a very specific test for a specific bug, the bug was
    // that when setEnabled(FALSE) then setEnabled(TRUE) was called on
    // the parent of a child widget which had focus while hidden, then
    // when the widget was shown, the focus would be in the wrong place.
    // And it doesn't seem to work on linux at all.

#ifdef Q_WS_X11
    QSKIP("This test doesn't work on X11", SkipAll);
#endif

    Q3HBox widget;
    QLineEdit *focusWidget = new QLineEdit( &widget );
    new QLineEdit( &widget );
    new QPushButton( &widget );
    focusWidget->setFocus();
    widget.setEnabled( FALSE );
    widget.setEnabled( TRUE );
    widget.show();
    QTest::qWait( 1000 );
    QVERIFY( qApp->focusWidget() == focusWidget );
}

class Container : public QWidget
{
public:
    Container()
    {
	(new QVBoxLayout(this))->setAutoAdd(true);
    }

    void tab()
    {
	focusNextPrevChild(TRUE);
    }

    void backTab()
    {
	focusNextPrevChild(FALSE);
    }
};

class Composite : public QFrame
{
public:
    Composite(QWidget* parent = 0, const char* name = 0)
        : QFrame(parent, name)
    {
        QHBoxLayout* hbox = new QHBoxLayout(this, 2, 0);
        hbox->setAutoAdd(true);

        lineEdit = new QLineEdit(this);

        button = new QPushButton(this);
        button->setFocusPolicy( Qt::NoFocus );

        setFocusProxy( lineEdit );
        setFocusPolicy( Qt::StrongFocus );

	setTabOrder(lineEdit, button);
    }

private:
    QLineEdit* lineEdit;
    QPushButton* button;
};

#define NUM_WIDGETS 4

void tst_QWidget::setTabOrder()
{
    qApp->processEvents(QEventLoop::AllEvents);

    Container container;

    Composite* comp[NUM_WIDGETS];

    QLineEdit *firstEdit = new QLineEdit(&container);

    int i = 0;
    for(i = 0; i < NUM_WIDGETS; i++) {
        comp[i] = new Composite(&container);
    }

    QLineEdit *lastEdit = new QLineEdit(&container);

    container.setTabOrder(lastEdit, comp[NUM_WIDGETS-1]);
    for(i = NUM_WIDGETS-1; i > 0; i--) {
        container.setTabOrder(comp[i], comp[i-1]);
    }
    container.setTabOrder(comp[0], firstEdit);

    int current = NUM_WIDGETS-1;
    lastEdit->setFocus();

    container.show();
    container.setActiveWindow();
    qApp->processEvents(QEventLoop::AllEvents);

#ifdef Q_WS_X11
    if (!lastEdit->hasFocus())
        QSKIP("Your window manager is too broken for this test.", SkipAll);
#endif
    QVERIFY(lastEdit->hasFocus());
    container.tab();
    do {
	QVERIFY(comp[current]->focusProxy()->hasFocus());
	container.tab();
	current--;
    } while (current >= 0);

    QVERIFY(firstEdit->hasFocus());
}

void tst_QWidget::activation()
{
#if !defined(Q_WS_WIN)
    QSKIP("This test is Windows-only.", SkipAll);
#endif

    QWidget widget1;
    widget1.setCaption("Widget1");

    QWidget widget2;
    widget2.setCaption("Widget2");

    widget1.show();
    widget2.show();

    QTest::qWait(100);
    QVERIFY(qApp->activeWindow() == &widget2);
    widget2.showMinimized();
    QTest::qWait(100);

    QVERIFY(qApp->activeWindow() == &widget1);
    widget2.showMaximized();
    QTest::qWait(100);
    QVERIFY(qApp->activeWindow() == &widget2);
    widget2.showMinimized();
    QTest::qWait(100);
    QVERIFY(qApp->activeWindow() == &widget1);
    widget2.showNormal();
    QTest::qWait(100);
    QVERIFY(qApp->activeWindow() == &widget2);
    widget2.hide();
    QTest::qWait(100);
    QVERIFY(qApp->activeWindow() == &widget1);
}

void tst_QWidget::windowState()
{
#ifdef Q_WS_X11
    QSKIP("Many window managers do not support window state properly, which causes this "
         "test to fail.", SkipAll);
#else
    const QPoint pos(500, 500);
    const QSize size(200, 200);

    QWidget widget1;
    widget1.move(pos);
    widget1.resize(size);
    QTest::qWait(100);
    widget1.setCaption("Widget1");

#define VERIFY_STATE(s) QCOMPARE(int(widget1.windowState() & stateMask), int(s))

    const int stateMask = Qt::WindowMaximized|Qt::WindowMinimized|Qt::WindowFullScreen;

    widget1.setWindowState(Qt::WindowMaximized);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowMaximized);

    widget1.show();
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowMaximized);

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMaximized);
    QTest::qWait(100);
    QVERIFY(!(widget1.windowState() & Qt::WindowMaximized));
    QCOMPARE(widget1.pos(), pos);

    widget1.setWindowState(Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowMinimized);

    widget1.setWindowState(widget1.windowState() | Qt::WindowMaximized);
    QTest::qWait(100);
    VERIFY_STATE((Qt::WindowMinimized|Qt::WindowMaximized));

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowMaximized);

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMaximized);
    QTest::qWait(100);
    QVERIFY(!(widget1.windowState() & (Qt::WindowMinimized|Qt::WindowMaximized)));
    QCOMPARE(widget1.pos(), pos);

    widget1.setWindowState(Qt::WindowFullScreen);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowFullScreen);

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE((Qt::WindowFullScreen|Qt::WindowMinimized));

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowFullScreen);

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMaximized);
    QTest::qWait(100);
    VERIFY_STATE((Qt::WindowFullScreen|Qt::WindowMaximized));

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE((Qt::WindowFullScreen|Qt::WindowMaximized|Qt::WindowMinimized));

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMinimized);
    QTest::qWait(100);
    VERIFY_STATE((Qt::WindowFullScreen|Qt::WindowMaximized));

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowFullScreen);
    QTest::qWait(100);
    VERIFY_STATE(Qt::WindowMaximized);

    widget1.setWindowState(widget1.windowState() ^ Qt::WindowMaximized);
    QTest::qWait(100);
    QVERIFY(!(widget1.windowState() & stateMask));

    QCOMPARE(widget1.pos(), pos);
    QCOMPARE(widget1.size(), size);
#endif
}

void tst_QWidget::showMaximized()
{
    QWidget plain;
    Q3HBox layouted;
    QLineEdit le(&layouted);
    QLineEdit le2(&layouted);
    QLineEdit le3(&layouted);

    plain.showMaximized();
    QVERIFY(plain.windowState() & Qt::WindowMaximized);

    plain.showNormal();
    QVERIFY(!(plain.windowState() & Qt::WindowMaximized));

    layouted.showMaximized();
    QVERIFY(layouted.windowState() & Qt::WindowMaximized);

    layouted.showNormal();
    QVERIFY(!(layouted.windowState() & Qt::WindowMaximized));

#if !defined(Q_WS_QWS)
//embedded may choose a different size to fit on the screen.
    QCOMPARE(layouted.size(), layouted.sizeHint());
#endif
    layouted.showMaximized();
    QVERIFY(layouted.isMaximized());
    QVERIFY(layouted.isVisible());

    layouted.hide();
    QVERIFY(layouted.isMaximized());
    QVERIFY(!layouted.isVisible());

    layouted.showMaximized();
    QVERIFY(layouted.isMaximized());
    QVERIFY(layouted.isVisible());

    layouted.showMinimized();
    QVERIFY(layouted.isMinimized());
    QVERIFY(layouted.isMaximized());

    layouted.showMaximized();
    QVERIFY(!layouted.isMinimized());
    QVERIFY(layouted.isMaximized());
    QVERIFY(layouted.isVisible());

    layouted.showMinimized();
    QVERIFY(layouted.isMinimized());
    QVERIFY(layouted.isMaximized());

    layouted.showMaximized();
    QVERIFY(!layouted.isMinimized());
    QVERIFY(layouted.isMaximized());
    QVERIFY(layouted.isVisible());

    {
        QWidget frame;
        QWidget widget(&frame);
        widget.showMaximized();
        QVERIFY(widget.isMaximized());
    }

#if !defined(Q_WS_QWS)
//embedded respects max/min sizes by design -- maybe wrong design, but that's the way it is now.
    {
        Q3HBox box;
        QWidget widget(&box);
        widget.setMinimumSize(500, 500);
        box.showMaximized();
        QVERIFY(box.isMaximized());
    }

    {
        Q3HBox box;
        QWidget widget(&box);
        widget.setMaximumSize(500, 500);

        box.showMaximized();
        QVERIFY(box.isMaximized());
    }
#endif
}

void tst_QWidget::showFullScreen()
{
    QWidget plain;
    Q3HBox layouted;
    QLineEdit le(&layouted);
    QLineEdit le2(&layouted);
    QLineEdit le3(&layouted);

    plain.showFullScreen();
    QVERIFY(plain.windowState() & Qt::WindowFullScreen);

    plain.showNormal();
    QVERIFY(!(plain.windowState() & Qt::WindowFullScreen));

    layouted.showFullScreen();
    QVERIFY(layouted.windowState() & Qt::WindowFullScreen);

    layouted.showNormal();
    QVERIFY(!(layouted.windowState() & Qt::WindowFullScreen));

#if !defined(Q_WS_QWS)
//embedded may choose a different size to fit on the screen.
    QCOMPARE(layouted.size(), layouted.sizeHint());
#endif

    layouted.showFullScreen();
    QVERIFY(layouted.isFullScreen());
    QVERIFY(layouted.isVisible());

    layouted.hide();
    QVERIFY(layouted.isFullScreen());
    QVERIFY(!layouted.isVisible());

    layouted.showFullScreen();
    QVERIFY(layouted.isFullScreen());
    QVERIFY(layouted.isVisible());

    layouted.showMinimized();
    QVERIFY(layouted.isMinimized());
    QVERIFY(layouted.isFullScreen());

    layouted.showFullScreen();
    QVERIFY(!layouted.isMinimized());
    QVERIFY(layouted.isFullScreen());
    QVERIFY(layouted.isVisible());

    layouted.showMinimized();
    QVERIFY(layouted.isMinimized());
    QVERIFY(layouted.isFullScreen());

    layouted.showFullScreen();
    QVERIFY(!layouted.isMinimized());
    QVERIFY(layouted.isFullScreen());
    QVERIFY(layouted.isVisible());

    {
        QWidget frame;
        QWidget widget(&frame);
        widget.showFullScreen();
        QVERIFY(widget.isFullScreen());
    }

#if !defined(Q_WS_QWS)
//embedded respects max/min sizes by design -- maybe wrong design, but that's the way it is now.
    {
        Q3HBox box;
        QWidget widget(&box);
        widget.setMinimumSize(500, 500);
        box.showFullScreen();
        QVERIFY(box.isFullScreen());
    }

    {
        Q3HBox box;
        QWidget widget(&box);
        widget.setMaximumSize(500, 500);

        box.showFullScreen();
        QVERIFY(box.isFullScreen());
    }
#endif
}

class ResizeWidget : public QWidget {
public:
    ResizeWidget(QWidget *p = 0) : QWidget(p)
    {
        m_resizeEventCount = 0;
    }
protected:
    void resizeEvent(QResizeEvent *e){
        QCOMPARE(size(), e->size());
        ++m_resizeEventCount;
    }

public:
    int m_resizeEventCount;
};

void tst_QWidget::resizeEvent()
{

    QWidget wParent;
    ResizeWidget wChild(&wParent);
    wParent.show();
    QCOMPARE (wChild.m_resizeEventCount, 1); // initial resize event before paint
    wParent.hide();
    wChild.resize(QSize(640,480));
    QCOMPARE (wChild.m_resizeEventCount, 1);
    wParent.show();
    QCOMPARE (wChild.m_resizeEventCount, 2);

    ResizeWidget wTopLevel;
    wTopLevel.show();
    QCOMPARE (wTopLevel.m_resizeEventCount, 1); // initial resize event before paint for toplevels
    wTopLevel.hide();
    wTopLevel.resize(QSize(640,480));
    QCOMPARE (wTopLevel.m_resizeEventCount, 1);
    wTopLevel.show();
    QCOMPARE (wTopLevel.m_resizeEventCount, 2);


    ResizeWidget w;
    qApp->processEvents();
    w.m_resizeEventCount = 0;
    w.showFullScreen();
    qApp->processEvents();
    WAIT_FOR_CONDITION(w.m_resizeEventCount, 1);
#ifdef Q_OS_WIN
    QEXPECT_FAIL("", "Windows incorrectly sends two resize events when switching to and from fullscreen.", Continue);
#endif
    QCOMPARE (w.m_resizeEventCount, 1);

    w.m_resizeEventCount = 0;
    w.showNormal();
    qApp->processEvents();
    WAIT_FOR_CONDITION(w.m_resizeEventCount, 1);
#ifdef Q_OS_WIN
    QEXPECT_FAIL("", "Windows incorrectly sends two resize events when switching to and from fullscreen.", Continue);
#endif
    QCOMPARE (w.m_resizeEventCount, 1);
}

void tst_QWidget::showMinimized()
{
    QWidget plain;
    plain.move(100, 100);
    Q3HBox layouted;
    QLineEdit le(&layouted);
    QLineEdit le2(&layouted);
    QLineEdit le3(&layouted);

    QPoint pos = plain.pos();

    plain.showMinimized();
    QVERIFY(plain.isMinimized());
    QVERIFY(plain.isVisible());
    QCOMPARE(plain.pos(), pos);

    plain.showNormal();
    QVERIFY(!plain.isMinimized());
    QVERIFY(plain.isVisible());
    QCOMPARE(plain.pos(), pos);

    plain.showMinimized();
    QVERIFY(plain.isMinimized());
    QCOMPARE(plain.pos(), pos);

    plain.hide();
    QVERIFY(plain.isMinimized());
    QVERIFY(!plain.isVisible());

    plain.showMinimized();
    QVERIFY(plain.isMinimized());
    QVERIFY(plain.isVisible());

    {
        QWidget frame;
        QWidget widget(&frame);
        widget.showMinimized();
        QVERIFY(widget.isMinimized());
    }

}

void tst_QWidget::reparent()
{
    QWidget parent;
    parent.setCaption("Toplevel");
    parent.setGeometry(300, 300, 200, 150);

    QWidget child(0, "child");
    child.setGeometry(10, 10, 180, 130);
    child.setPaletteBackgroundColor(Qt::white);

    QWidget childTLW(&child, "childTLW", Qt::WType_TopLevel);
    childTLW.setGeometry(100, 100, 50, 50);
    childTLW.setPaletteBackgroundColor(Qt::yellow);

    parent.show();
    childTLW.show();

#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&parent);
    qt_x11_wait_for_window_manager(&childTLW);
#endif

    parent.move(300, 300);

    QPoint childPos = parent.mapToGlobal(child.pos());
    QPoint tlwPos = childTLW.pos();

    child.reparent(0, parent.mapToGlobal(child.pos()), TRUE);

#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&child);
    // On X11, the window manager will apply NorthWestGravity rules to 'child', which
    // means the top-left corner of the window frame will be placed at 'childPos',
    // causing this test to fail
#else
    QCOMPARE(child.geometry().topLeft(), childPos);
#endif
    QCOMPARE(childTLW.pos(), tlwPos);

#ifdef Q_WS_WIN
    QWidget childTLWChild(&childTLW, "childTLWChild");

    QWidget grandChild(&child, "grandChild");
    grandChild.setGeometry(10, 10, 160, 110);
    grandChild.setPaletteBackgroundColor(Qt::red);

    QWidget grandChildTLW(&grandChild, "grandChildTLW", Qt::WType_TopLevel);
    grandChildTLW.setGeometry(200, 200, 50, 50);
    grandChildTLW.setPaletteBackgroundColor(Qt::yellow);

    QWidget grandChildTLWChild(&grandChildTLW, "grandChildTLWChild");

    QVERIFY(IsWindow(childTLW.winId()));
    QVERIFY(IsWindow(childTLWChild.winId()));
    QVERIFY(IsWindow(grandChildTLW.winId()));
    QVERIFY(IsWindow(grandChildTLWChild.winId()));

    parent.show();

    QVERIFY(IsWindow(childTLW.winId()));
    QVERIFY(IsWindow(childTLWChild.winId()));
    QVERIFY(IsWindow(grandChildTLW.winId()));
    QVERIFY(IsWindow(grandChildTLWChild.winId()));

    child.reparent(&parent, QPoint(10,10), TRUE);

    // this appears to stabelize results
    qApp->processEvents();

    QVERIFY(IsWindow(childTLW.winId()));
    QVERIFY(IsWindow(childTLWChild.winId()));

    QVERIFY(IsWindow(grandChildTLW.winId()));
    QVERIFY(IsWindow(grandChildTLWChild.winId()));
#else
    QSKIP("This test makes only sense on Windows", SkipAll);
#endif
}

void tst_QWidget::icon()
{
#if defined(Q_WS_QWS)
    QSKIP("Qt/Embedded does it differently", SkipAll);
#else
    QPixmap p(20,20);
    p.fill(Qt::red);
    testWidget->setIcon(p);

    QVERIFY(testWidget->icon() && !testWidget->icon()->isNull());
    testWidget->show();
    QVERIFY(testWidget->icon() && !testWidget->icon()->isNull());
    testWidget->showFullScreen();
    QVERIFY(testWidget->icon() && !testWidget->icon()->isNull());
    testWidget->showNormal();
    QVERIFY(testWidget->icon() && !testWidget->icon()->isNull());
#endif
}

void tst_QWidget::hideWhenFocusWidgetIsChild()
{
    testWidget->setActiveWindow();
    QWidget *parentWidget = new QWidget(testWidget, "parentWidget");
    parentWidget->setGeometry(0, 0, 100, 100);
    QLineEdit *edit = new QLineEdit(parentWidget, "edit1");
    QLineEdit *edit3 = new QLineEdit(parentWidget, "edit3");
    edit3->move(0,50);
    parentWidget->show();
    QLineEdit *edit2 = new QLineEdit(testWidget, "edit2");
    edit2->show();
    edit2->move(110, 100);
    edit->setFocus();
    qApp->processEvents();
    QString actualFocusWidget, expectedFocusWidget;
#ifdef Q_WS_X11
    if (!qApp->focusWidget())
        QSKIP("Your window manager is too broken for this test", SkipAll);
#endif
    QVERIFY(qApp->focusWidget());
    actualFocusWidget.sprintf("%p %s %s", qApp->focusWidget(), qApp->focusWidget()->name(), qApp->focusWidget()->className());
    expectedFocusWidget.sprintf("%p %s %s", edit, edit->name(), edit->className());
    QCOMPARE(actualFocusWidget, expectedFocusWidget);

    parentWidget->hide();
    qApp->processEvents();
    actualFocusWidget.sprintf("%p %s %s", qApp->focusWidget(), qApp->focusWidget()->name(), qApp->focusWidget()->className());
    expectedFocusWidget.sprintf("%p %s %s", edit2, edit2->name(), edit2->className());
    QCOMPARE(actualFocusWidget, expectedFocusWidget);
}

void tst_QWidget::normalGeometry()
{
    QWidget parent;
    parent.setWindowTitle("NormalGeometry parent");
    QWidget *child = new QWidget(&parent);

    QCOMPARE(parent.normalGeometry(), parent.geometry());
    QCOMPARE(child->normalGeometry(), QRect());

    parent.setGeometry(100, 100, 200, 200);
    parent.show();
    QTestEventLoop::instance().enterLoop(1);

    QRect geom = parent.geometry();
    // ### the window manager places the top-left corner at
    // ### 100,100... making geom something like 102,124 (offset by
    // ### the frame/frame)... this indicates a rather large different
    // ### between how X11 and Windows works
    // QCOMPARE(geom, QRect(100, 100, 200, 200));
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowMaximized);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(parent.windowState() & Qt::WindowMaximized);
    QVERIFY(parent.geometry() != geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowMaximized);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowMaximized));
    QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.showMaximized();
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(parent.windowState() & Qt::WindowMaximized);
    QVERIFY(parent.geometry() != geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.showNormal();
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowMaximized));
    QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowMinimized);
    QTestEventLoop::instance().enterLoop(1);
    parent.setWindowState(parent.windowState() ^ Qt::WindowMaximized);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(parent.windowState() & (Qt::WindowMinimized|Qt::WindowMaximized));
    // ### when minimized and maximized at the same time, the geometry
    // ### does *NOT* have to be the normal geometry, it could be the
    // ### maximized geometry.
    // QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowMinimized);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowMinimized));
    QVERIFY(parent.windowState() & Qt::WindowMaximized);
    QVERIFY(parent.geometry() != geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowMaximized);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowMaximized));
    QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowFullScreen);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(parent.windowState() & Qt::WindowFullScreen);
    QVERIFY(parent.geometry() != geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.setWindowState(parent.windowState() ^ Qt::WindowFullScreen);
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowFullScreen));
    QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.showFullScreen();
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(parent.windowState() & Qt::WindowFullScreen);
    QVERIFY(parent.geometry() != geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.showNormal();
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(!(parent.windowState() & Qt::WindowFullScreen));
    QCOMPARE(parent.geometry(), geom);
    QCOMPARE(parent.normalGeometry(), geom);

    parent.showNormal();
    parent.setWindowState(Qt:: WindowFullScreen | Qt::WindowMaximized);
    parent.setWindowState(Qt::WindowMinimized | Qt:: WindowFullScreen | Qt::WindowMaximized);
    parent.setWindowState(Qt:: WindowFullScreen | Qt::WindowMaximized);
    QTestEventLoop::instance().enterLoop(1);
    QCOMPARE(parent.normalGeometry(), geom);
}


void tst_QWidget::setGeometry()
{
    QWidget tlw;
    QWidget child(&tlw);

    QRect tr(100,100,200,200);
    QRect cr(50,50,50,50);
    tlw.setGeometry(tr);
    child.setGeometry(cr);
    tlw.show();
    QTest::qWait(50);
    QCOMPARE(tlw.geometry().size(), tr.size());
    QCOMPARE(child.geometry(), cr);

    tlw.setParent(0, Qt::Window|Qt::FramelessWindowHint);
    tr = QRect(0,0,100,100);
    tlw.setGeometry(tr);
    QCOMPARE(tlw.geometry(), tr);
    tlw.show();
    QTest::qWait(50);
    if (tlw.frameGeometry() != tlw.geometry())
        QSKIP("Your window manager is too broken for this test", SkipAll);
    QCOMPARE(tlw.geometry(), tr);

}

void tst_QWidget::windowOpacity()
{
#if defined(Q_WS_X11) && QT_VERSION < 0x040200
    QSKIP("QWidget::windowOpacity is broken in Qt < 4.2 on X11", SkipAll);
#else
    QWidget widget;
    QWidget child(&widget);

    // Initial value should be 1.0
    QCOMPARE(widget.windowOpacity(), 1.0);
    // children should always return 1.0
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Prior to 4.2, all children widgets returned 0.0", Continue);
#endif
    QCOMPARE(child.windowOpacity(), 1.0);

    widget.setWindowOpacity(0.0);
    QCOMPARE(widget.windowOpacity(), 0.0);
    child.setWindowOpacity(0.0);
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Prior to 4.2, all children widgets returned 0.0", Continue);
#endif
    QCOMPARE(child.windowOpacity(), 1.0);

    widget.setWindowOpacity(1.0);
    QCOMPARE(widget.windowOpacity(), 1.0);
    child.setWindowOpacity(1.0);
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Prior to 4.2, all children widgets returned 0.0", Continue);
#endif
    QCOMPARE(child.windowOpacity(), 1.0);

    widget.setWindowOpacity(2.0);
    QCOMPARE(widget.windowOpacity(), 1.0);
    child.setWindowOpacity(2.0);
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Prior to 4.2, all children widgets returned 0.0", Continue);
#endif
    QCOMPARE(child.windowOpacity(), 1.0);

    widget.setWindowOpacity(-1.0);
    QCOMPARE(widget.windowOpacity(), 0.0);
    child.setWindowOpacity(-1.0);
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Prior to 4.2, all children widgets returned 0.0", Continue);
#endif
    QCOMPARE(child.windowOpacity(), 1.0);
#endif
}

void tst_QWidget::raise()
{
    QWidget *parent = new QWidget(0);
    QWidget *child1 = new QWidget(parent);
    QWidget *child2 = new QWidget(parent);
    QWidget *child3 = new QWidget(parent);
    QWidget *child4 = new QWidget(parent);

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);

    child2->raise();

    QList<QObject *> list2;
    list2 << child1 << child3 << child4 << child2;
    QVERIFY(parent->children() == list2);

    delete parent;
}

void tst_QWidget::lower()
{
    QWidget *parent = new QWidget(0);
    QWidget *child1 = new QWidget(parent);
    QWidget *child2 = new QWidget(parent);
    QWidget *child3 = new QWidget(parent);
    QWidget *child4 = new QWidget(parent);

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);

    child3->lower();

    QList<QObject *> list2;
    list2 << child3 << child1 << child2 << child4;
    QVERIFY(parent->children() == list2);

    delete parent;
}

void tst_QWidget::stackUnder()
{
    QWidget *parent = new QWidget(0);
    QWidget *child1 = new QWidget(parent);
    QWidget *child2 = new QWidget(parent);
    QWidget *child3 = new QWidget(parent);
    QWidget *child4 = new QWidget(parent);

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);

    child4->stackUnder(child2);
    QList<QObject *> list2;
    list2 << child1 << child4 << child2 << child3;
    QVERIFY(parent->children() == list2);

    child1->stackUnder(child3);
    QList<QObject *> list3;
    list3 << child4 << child2 << child1 << child3;
    QVERIFY(parent->children() == list3);

    delete parent;
}

void drawPolygon(QPaintDevice *dev, int w, int h)
{
    QPainter p(dev);
    p.fillRect(0, 0, w, h, Qt::white);

    QPolygon a;
    a << QPoint(0, 0) << QPoint(w/2, h/2) << QPoint(w, 0)
      << QPoint(w/2, h) << QPoint(0, 0);

    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::DiagCrossPattern);
    p.drawPolygon(a);
}

class ContentsPropagationWidget : public QWidget
{
    Q_OBJECT
public:
    ContentsPropagationWidget(QWidget *parent = 0) : QWidget(parent)
    {
        QWidget *child = this;
        for (int i=0; i<32; ++i) {
            child = new QWidget(child);
            child->setGeometry(i, i, 400 - i*2, 400 - i*2);
        }
    }

    void setContentsPropagation(bool enable) {
        foreach (QObject *child, children())
            qobject_cast<QWidget *>(child)->setAutoFillBackground(!enable);
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        int w = width(), h = height();
        drawPolygon(this, w, h);
    }

    QSize sizeHint() const { return QSize(500, 500); }
};

void tst_QWidget::testContentsPropagation()
{
#ifdef Q_WS_MAC
    QSKIP("Pixmap is not antialiased whereas widget is.", SkipAll);
#endif
    ContentsPropagationWidget widget;
    widget.setFixedSize(500, 500);
    widget.setContentsPropagation(false);
    QPixmap widgetSnapshot = QPixmap::grabWidget(&widget);

    QPixmap correct(500, 500);
    drawPolygon(&correct, 500, 500);
    //correct.save("correct.png", "PNG");

    //widgetSnapshot.save("snap1.png", "PNG");
    QVERIFY(widgetSnapshot.convertToImage() != correct.convertToImage());

    widget.setContentsPropagation(true);
    widgetSnapshot = QPixmap::grabWidget(&widget);
    //widgetSnapshot.save("snap2.png", "PNG");

    QCOMPARE(widgetSnapshot, correct);
}

/*
    Test that saving and restoring window geometry with
    saveGeometry() and restoreGeometry() works.
*/
void tst_QWidget::saveRestoreGeometry()
{
    const QPoint position(100, 100);
    const QSize size(200, 200);

    QByteArray savedGeometry;

    {
        QWidget widget;
        widget.move(position);
        widget.resize(size);
        widget.show();
//        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.pos().x(), position.x());
        QCOMPARE(widget.size(), size);
        savedGeometry = widget.saveGeometry();
    }

#if 0
    // Code for saving a new geometry.dat file.
    QFile f("geometry.dat");
    f.open(QIODevice::WriteOnly);
    f.write(savedGeometry);
    f.close();
#endif

    {
        QWidget widget;

        const QByteArray empty;
        const QByteArray one("a");
        const QByteArray two("ab");
        const QByteArray three("abc");
        const QByteArray four("abca");
        const QByteArray garbage("abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc");

        QVERIFY(widget.restoreGeometry(empty) == false);
        QVERIFY(widget.restoreGeometry(one) == false);
        QVERIFY(widget.restoreGeometry(two) == false);
        QVERIFY(widget.restoreGeometry(three) == false);
        QVERIFY(widget.restoreGeometry(four) == false);
        QVERIFY(widget.restoreGeometry(garbage) == false);

        QVERIFY(widget.restoreGeometry(savedGeometry));
//        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.pos().x(), position.x());
        QCOMPARE(widget.size(), size);
        widget.show();
//        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.pos().x(), position.x());
        QCOMPARE(widget.size(), size);
    }
}

/*
    Test that the current version of restoreGeometry() can restore geometry
    saved width saveGeometry() version 1.0.
*/
void tst_QWidget::restoreVersion1Geometry()
{
    const QPoint position(100, 100);
    const QSize size(200, 200);

    QFile f(":geometry.dat");
    QVERIFY(f.exists());
    f.open(QIODevice::ReadOnly);
    const QByteArray savedGeometry = f.readAll();;
    QCOMPARE(savedGeometry.count(), 46);

    QWidget widget;
    QVERIFY(widget.restoreGeometry(savedGeometry));
//    QCOMPARE(widget.pos(), position);
    QCOMPARE(widget.pos().x(), position.x());
    QCOMPARE(widget.size(), size);
    widget.show();
//    QCOMPARE(widget.pos(), position);
    QCOMPARE(widget.pos().x(), position.x());
    QCOMPARE(widget.size(), size);
}

void tst_QWidget::widgetAt()
{
    QWidget *w1 = new QWidget(0, Qt::X11BypassWindowManagerHint);
    w1->setGeometry(0,0,150,150);
    w1->setObjectName("w1");

    QWidget *w2 = new QWidget(0, Qt::X11BypassWindowManagerHint);
    w2->setGeometry(50,50,100,100);
    w2->setObjectName("w2");
    w1->show();
    qApp->processEvents();
    QWidget *wr = QApplication::widgetAt(100, 100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w1"));

    w2->show();
    qApp->processEvents();
    qApp->processEvents();
    qApp->processEvents();
    wr = QApplication::widgetAt(100, 100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w2"));

    w2->lower();
    qApp->processEvents();
    wr = QApplication::widgetAt(100, 100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w1"));

    w2->raise();
    qApp->processEvents();
    wr = QApplication::widgetAt(100, 100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w2"));


    QWidget *w3 = new QWidget(w2);
    w3->setGeometry(10,10,50,50);
    w3->setObjectName("w3");
    w3->show();
    qApp->processEvents();
    wr = QApplication::widgetAt(100,100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w3"));

    w3->setAttribute(Qt::WA_TransparentForMouseEvents);
    qApp->processEvents();
    wr = QApplication::widgetAt(100, 100);
    QVERIFY(wr);
    QCOMPARE(wr->objectName(), QString("w2"));

    QRegion rgn = QRect(QPoint(0,0), w2->size());
    QPoint point = w2->mapFromGlobal(QPoint(100,100));
    rgn -= QRect(point, QSize(1,1));
    w2->setMask(rgn);
    qApp->processEvents();
    QCOMPARE(QApplication::widgetAt(100,100)->objectName(), w1->objectName());
    QCOMPARE(QApplication::widgetAt(101,101)->objectName(), w2->objectName());

    QBitmap bitmap(w2->size());
    QPainter p(&bitmap);
    p.fillRect(bitmap.rect(), Qt::color1);
    p.setPen(Qt::color0);
    p.drawPoint(w2->mapFromGlobal(QPoint(100,100)));
    p.end();
    w2->setMask(bitmap);
    qApp->processEvents();
    QTest::qWait(100);
    QVERIFY(QApplication::widgetAt(100,100) == w1);
    QVERIFY(QApplication::widgetAt(101,101) == w2);

    delete w2;
    delete w1;
}

#if defined(Q_WS_X11)
bool getProperty(Display *display, Window target, Atom type, Atom property,
                 unsigned char** data, unsigned long* count)
{
    Atom atom_return;
    int size;
    unsigned long nitems, bytes_left;

    int ret = XGetWindowProperty(display, target, property,
                                 0l, 1l, false,
                                 type, &atom_return, &size,
                                 &nitems, &bytes_left, data);
    if (ret != Success || nitems < 1)
        return false;

    if (bytes_left != 0) {
        XFree(*data);
        unsigned long remain = ((size / 8) * nitems) + bytes_left;
        ret = XGetWindowProperty(display, target,
                                 property, 0l, remain, false,
                                 type, &atom_return, &size,
                                 &nitems, &bytes_left, data);
        if (ret != Success)
            return false;
    }

    *count = nitems;
    return true;
}

QString textPropertyToString(Display *display, XTextProperty& text_prop)
{
    QString ret;
    if (text_prop.value && text_prop.nitems > 0) {
        if (text_prop.encoding == XA_STRING) {
            ret = reinterpret_cast<char *>(text_prop.value);
        } else {
            text_prop.nitems = strlen(reinterpret_cast<char *>(text_prop.value));
            char **list;
            int num;
            if (XmbTextPropertyToTextList(display, &text_prop, &list, &num) == Success
                && num > 0 && *list) {
                ret = QString::fromLocal8Bit(*list);
                XFreeStringList(list);
            }
        }
    }
    return ret;
}

#endif

static QString visibleWindowTitle(QWidget *window, Qt::WindowState state = Qt::WindowNoState)
{
    QString vTitle;

#ifdef Q_WS_WIN
    unsigned short title[256];
    GetWindowTextW(window->winId(), (WCHAR *)title, sizeof(title));
    vTitle = QString::fromUtf16((ushort *)title);
#elif defined(Q_WS_X11)
    /*
      We can't check what the window manager displays, but we can
      check what we tell the window manager to display.  This will
      have to do.
    */
    Atom UTF8_STRING = XInternAtom(window->x11Info().display(), "UTF8_STRING", false);
    Atom _NET_WM_NAME = XInternAtom(window->x11Info().display(), "_NET_WM_NAME", false);
    Atom _NET_WM_ICON_NAME = XInternAtom(window->x11Info().display(), "_NET_WM_ICON_NAME", false);
    uchar *data = 0;
    ulong length = 0;
    if (state == Qt::WindowMinimized) {
        if (getProperty(window->x11Info().display(), window->winId(),
                    UTF8_STRING, _NET_WM_ICON_NAME, &data, &length)) {
            vTitle = QString::fromUtf8((char *) data, length);
            XFree(data);
        } else {
            XTextProperty text_prop;
            if (XGetWMIconName(window->x11Info().display(), window->winId(), &text_prop)) {
                vTitle = textPropertyToString(window->x11Info().display(), text_prop);
                XFree((char *) text_prop.value);
            }
        }
    } else {
        if (getProperty(window->x11Info().display(), window->winId(),
                    UTF8_STRING, _NET_WM_NAME, &data, &length)) {
            vTitle = QString::fromUtf8((char *) data, length);
            XFree(data);
        } else {
            XTextProperty text_prop;
            if (XGetWMName(window->x11Info().display(), window->winId(), &text_prop)) {
                vTitle = textPropertyToString(window->x11Info().display(), text_prop);
                XFree((char *) text_prop.value);
            }
        }
    }
#elif defined(Q_WS_MAC)
    QCFString macTitle;
    if (state == Qt::WindowMinimized)
        CopyWindowAlternateTitle(qt_mac_window_for(window), &macTitle);
    else
        CopyWindowTitleAsCFString(qt_mac_window_for(window), &macTitle);
    vTitle = macTitle;
#elif defined(Q_WS_QWS)
    if (qwsServer) {
    const QWSWindow *win = 0;
    const QList<QWSWindow*> windows = qwsServer->clientWindows();
    for (int i = 0; i < windows.count(); ++i) {
        const QWSWindow* w = windows.at(i);
        if (w->winId() == window->winId()) {
            win = w;
            break;
        }
    }
    if (win)
        vTitle = win->caption();
    }
#endif

    return vTitle;
}

void tst_QWidget::windowTitle()
{
    QWidget widget(0);
    widget.setWindowTitle("Application Name");
    widget.winId(); // Make sure the window is created...
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowTitle("Application Name *");
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name *"));

    widget.setWindowTitle("Application Name[*]");
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowTitle("Application Name[*][*]");
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*]"));

    widget.setWindowTitle("Application Name[*][*][*]");
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*]"));

    widget.setWindowTitle("Application Name[*][*][*][*]");
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*][*]"));
}

void tst_QWidget::windowIconText()
{
    QWidget widget(0);

    widget.setWindowTitle("Application Name");
    widget.setWindowIconText("Application Minimized");
    widget.showNormal();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));
    widget.showMinimized();
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does not implement showMinimized()", Continue);
#endif
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget, Qt::WindowMinimized),
            QString("Application Minimized"));

    widget.setWindowTitle("Application Name[*]");
    widget.setWindowIconText("Application Minimized[*]");
    widget.showNormal();
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));
    widget.showMinimized();
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does not implement showMinimized()", Continue);
#endif
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget, Qt::WindowMinimized),
            QString("Application Minimized"));

    widget.setWindowModified(true);
    widget.showNormal();
    QApplication::instance()->processEvents();
    if (widget.style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, &widget))
        QCOMPARE(visibleWindowTitle(&widget), QString("Application Name*"));
    else
        QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));
    widget.showMinimized();
#ifdef Q_WS_QWS
    QEXPECT_FAIL(0, "Qt/Embedded does not implement showMinimized()", Continue);
#endif
    QApplication::instance()->processEvents();
#ifdef Q_WS_MAC
    QCOMPARE(visibleWindowTitle(&widget, Qt::WindowMinimized),
            QString("Application Minimized"));
    QVERIFY(IsWindowModified(qt_mac_window_for(&widget)));
#else
    QCOMPARE(visibleWindowTitle(&widget, Qt::WindowMinimized),
            QString("Application Minimized*"));
#endif
}

void tst_QWidget::windowModified()
{
    QWidget widget(0);
    widget.show();

    QTest::ignoreMessage(QtWarningMsg, "QWidget::setWindowModified: The window title does not contain a '[*]' placeholder");
    widget.setWindowTitle("Application Name");
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

#ifdef Q_WS_MAC
    widget.setWindowModified(true);
    QVERIFY(IsWindowModified(qt_mac_window_for(&widget)));
#else
    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowTitle("Application Name[*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    if (widget.style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, &widget))
        QCOMPARE(visibleWindowTitle(&widget), QString("Application Name*"));
    else
        QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowTitle("Application[*] Name[*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application* Name*"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name"));

    widget.setWindowTitle("Application Name[*][*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*]"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*]"));

    widget.setWindowTitle("Application[*][*] Name[*][*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application[*] Name[*]"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application[*] Name[*]"));

    widget.setWindowTitle("Application[*] Name[*][*][*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application* Name[*]*"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application Name[*]"));

    widget.setWindowTitle("Application[*][*][*] Name[*][*][*]");

    widget.setWindowModified(true);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application[*]* Name[*]*"));

    widget.setWindowModified(false);
    QApplication::instance()->processEvents();
    QCOMPARE(visibleWindowTitle(&widget), QString("Application[*] Name[*]"));
#endif
}

void tst_QWidget::task110173()
{
    QWidget w;

    QPushButton *pb1 = new QPushButton("click", &w);
    pb1->setFocusPolicy(Qt::ClickFocus);
    pb1->move(100, 100);

    QPushButton *pb2 = new QPushButton("push", &w);
    pb2->setFocusPolicy(Qt::ClickFocus);
    pb2->move(300, 300);

    QTest::keyClick( &w, Qt::Key_Tab );
    w.show();
    QTest::qWait(1000);
}

class Widget : public QWidget
{
public:
    Widget() : deleteThis(false) { setFocusPolicy(Qt::StrongFocus); }
    void actionEvent(QActionEvent *) { if (deleteThis) delete this; }
    void changeEvent(QEvent *) { if (deleteThis) delete this; }
    void closeEvent(QCloseEvent *) { if (deleteThis) delete this; }
    void hideEvent(QHideEvent *) { if (deleteThis) delete this; }
    void focusOutEvent(QFocusEvent *) { if (deleteThis) delete this; }
    void keyPressEvent(QKeyEvent *) { if (deleteThis) delete this; }
    void keyReleaseEvent(QKeyEvent *) { if (deleteThis) delete this; }
    void mouseDoubleClickEvent(QMouseEvent *) { if (deleteThis) delete this; }
    void mousePressEvent(QMouseEvent *) { if (deleteThis) delete this; }
    void mouseReleaseEvent(QMouseEvent *) { if (deleteThis) delete this; }
    void mouseMoveEvent(QMouseEvent *) { if (deleteThis) delete this; }

    bool deleteThis;
};

void tst_QWidget::testDeletionInEventHandlers()
{
    // closeEvent
    QPointer<Widget> w = new Widget;
    w->deleteThis = true;
    w->close();
    QVERIFY(w == 0);
    delete w;

    // focusOut (crashes)
    //w = new Widget;
    //w->show();
    //w->setFocus();
    //QVERIFY(qApp->focusWidget() == w);
    //w->deleteThis = true;
    //w->clearFocus();
    //QVERIFY(w == 0);

    // key press
    w = new Widget;
    w->show();
    w->deleteThis = true;
    QTest::keyPress(w, Qt::Key_A);
    QVERIFY(w == 0);
    delete w;

    // key release
    w = new Widget;
    w->show();
    w->deleteThis = true;
    QTest::keyRelease(w, Qt::Key_A);
    QVERIFY(w == 0);
    delete w;

    // mouse press
    w = new Widget;
    w->show();
    w->deleteThis = true;
    QTest::mousePress(w, Qt::LeftButton);
    QVERIFY(w == 0);
    delete w;

    // mouse release
    w = new Widget;
    w->show();
    w->deleteThis = true;
    QTest::mouseRelease(w, Qt::LeftButton);
    QVERIFY(w == 0);
    delete w;

    // mouse double click
    w = new Widget;
    w->show();
    w->deleteThis = true;
    QTest::mouseDClick(w, Qt::LeftButton);
    QVERIFY(w == 0);
    delete w;

    // hide event (crashes)
    //w = new Widget;
    //w->show();
    //w->deleteThis = true;
    //w->hide();
    //QVERIFY(w == 0);

    // action event
    w = new Widget;
    w->deleteThis = true;
    w->addAction(new QAction(w));
    QVERIFY(w == 0);
    delete w;

    // change event
    w = new Widget;
    w->show();
    w->deleteThis = true;
    w->setMouseTracking(true);
    QVERIFY(w == 0);
    delete w;

    w = new Widget;
    w->setMouseTracking(true);
    w->show();
    w->deleteThis = true;
    QTest::mouseMove(w, QPoint(0,0));
    QTest::qWait(2000); // mouseMove is async
    QVERIFY(w == 0);
    delete w;
}

void tst_QWidget::style()
{
    QWidget *window1 = new QWidget;
    window1->setObjectName("window1");
    QWidget *widget1 = new QWidget(window1);
    widget1->setObjectName("widget1");
    QWidget *widget2 = new QWidget;
    widget2->setObjectName("widget2");
    QWidget *window2 = new QWidget;
    window2->setObjectName("window2");
    window1->ensurePolished();
    window2->ensurePolished();
    widget1->ensurePolished();
    widget2->ensurePolished();

    QWindowsStyle style1, style2;
    QPointer<QStyle> pstyle1 = &style1;
    QPointer<QStyle> pstyle2 = &style2;

    QStyle *appStyle = qApp->style();

    // Sanity: By default, a window inherits the application style
    QCOMPARE(appStyle, window1->style());

    // Setting a custom style on a widget
    window1->setStyle(&style1);
    QCOMPARE(&style1, window1->style());

    // Setting another style must not delete the older style
    window1->setStyle(&style2);
    QCOMPARE(&style2, window1->style());
    QVERIFY(!pstyle1.isNull()); // case we have not already crashed

    // Setting null style must make it follow the qApp style
    window1->setStyle(0);
    QCOMPARE(window1->style(), appStyle);
    QVERIFY(!pstyle2.isNull()); // case we have not already crashed
    QVERIFY(!pstyle2.isNull()); // case we have not already crashed

    // Sanity: Set the stylesheet
    window1->setStyleSheet(":x { }");

    QPointer<QStyleSheetStyle> proxy = (QStyleSheetStyle *)window1->style();
    QVERIFY(!proxy.isNull());
    QCOMPARE(proxy->className(), "QStyleSheetStyle"); // must be our proxy
    QVERIFY(proxy->base == 0); // and follows the application

    // Set the stylesheet
    window1->setStyle(&style1);
    QVERIFY(proxy.isNull()); // we create a new one each time
    proxy = (QStyleSheetStyle *)window1->style();
    QCOMPARE(proxy->className(), "QStyleSheetStyle"); // it is a proxy
    QCOMPARE(proxy->baseStyle(), &style1); // must have been replaced with the new one

    // Update the stylesheet and check nothing changes
    window1->setStyleSheet(":y { }");
    QCOMPARE(window1->style()->className(), "QStyleSheetStyle"); // it is a proxy
    QCOMPARE(proxy->baseStyle(), &style1); // the same guy

    // Remove the stylesheet
    proxy = (QStyleSheetStyle *)window1->style();
    window1->setStyleSheet("");
    QVERIFY(proxy.isNull()); // should have disappeared
    QCOMPARE(window1->style(), &style1); // its restored

    // Style Sheet existing children propagation
    window1->setStyleSheet(":z { }");
    proxy = (QStyleSheetStyle *)window1->style();
    QCOMPARE(proxy->className(), "QStyleSheetStyle");
    QCOMPARE(window1->style(), widget1->style()); // proxy must have propagated
    QCOMPARE(widget2->style(), appStyle); // widget2 is following the app style

    // Style Sheet automatic propagation to new children
    widget2->setParent(window1); // reparent in!
    QCOMPARE(window1->style(), widget2->style()); // proxy must have propagated

    // Style Sheet automatic removal from children who abandoned their parents
    window2->setStyle(&style2);
    widget2->setParent(window2); // reparent
    QCOMPARE(widget2->style(), appStyle); // widget2 is following the app style

    // Style Sheet propagation on a child widget with a custom style
    widget2->setStyle(&style1);
    window2->setStyleSheet(":x { }");
    proxy = (QStyleSheetStyle *)widget2->style();
    QCOMPARE(proxy->className(), "QStyleSheetStyle");
    QCOMPARE(proxy->baseStyle(), &style1);

    // Style Sheet propagation on a child widget with a custom style already set
    window2->setStyleSheet("");
    QCOMPARE(window2->style(), &style2);
    QCOMPARE(widget2->style(), &style1);
    widget2->setStyle(0);
    window2->setStyleSheet(":x { }");
    widget2->setStyle(&style1);
    proxy = (QStyleSheetStyle *)widget2->style();
    QCOMPARE(proxy->className(), "QStyleSheetStyle");

    // QApplication, QWidget both having a style sheet

    // clean everything out
    window1->setStyle(0);
    window1->setStyleSheet("");
    window2->setStyle(0);
    window2->setStyleSheet("");
    qApp->setStyle(0);

    qApp->setStyleSheet("may_insanity_prevail { }"); // app has styleshet
    QCOMPARE(window1->style(), qApp->style());
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle");
    QCOMPARE(widget1->style()->metaObject()->className(), "QStyleSheetStyle"); // check the child
    window1->setStyleSheet("may_more_insanity_prevail { }"); // window has stylesheet
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle"); // a new one
    QCOMPARE(widget1->style(), window1->style()); // child follows...
    proxy = (QStyleSheetStyle *) window1->style();
    QCOMPARE(static_cast<QStyle *>(proxy), qApp->style()); // and it follows the application!
    QWindowsStyle *newStyle = new QWindowsStyle;
    qApp->setStyle(newStyle); // set a custom style on app
    proxy = (QStyleSheetStyle *) window1->style();
    QCOMPARE(proxy->baseStyle(), newStyle); // magic ;) the widget still follows the application
    QCOMPARE(static_cast<QStyle *>(proxy), widget1->style()); // child still follows...

    window1->setStyleSheet(""); // remove stylesheet
    QCOMPARE(window1->style(), qApp->style()); // is this cool or what
    QCOMPARE(widget1->style(), qApp->style()); // annoying child follows...
    QWindowsStyle wndStyle;
    window1->setStyle(&wndStyle);
    QCOMPARE(window1->style()->metaObject()->className(), "QStyleSheetStyle"); // auto wraps it
    QCOMPARE(widget1->style(), window1->style()); // and auto propagates to child
    qApp->setStyleSheet(""); // remove the app stylesheet
    QCOMPARE(window1->style(), &wndStyle); // auto dewrap
    QCOMPARE(widget1->style(), qApp->style()); // and child state is restored
    window1->setStyle(0); // let sanity prevail

    delete window1;
    delete widget2;
    delete window2;
}

#ifdef Q_WS_MAC

bool testAndRelease(const HIViewRef view)
{
//    qDebug() << CFGetRetainCount(view);
    if (CFGetRetainCount(view) != 2)
        return false;
    CFRelease(view);
    CFRelease(view);
    return true;
}

typedef QPair<QWidget *, HIViewRef> WidgetViewPair;

WidgetViewPair createAndRetain(QWidget * const parent = 0)
{
    QWidget * const widget = new QWidget(parent);
    const HIViewRef view = (HIViewRef)widget->winId();
    // Retain twice so we can safely call CFGetRetaintCount even if the retain count
    // is off by one because of a double release.
    CFRetain(view);
    CFRetain(view);
    return qMakePair(widget, view);
}

/*
    Test that retaining and releasing the HIView returned by QWidget::winId()
    works even if the widget itself is deleted.
*/
void tst_QWidget::retainHIView()
{
    // Single window
    {
        const WidgetViewPair window  = createAndRetain();
        delete window.first;
        QVERIFY(testAndRelease(window.second));
    }

    // Child widget
    {
        const WidgetViewPair parent = createAndRetain();
        const WidgetViewPair child = createAndRetain(parent.first);

        delete parent.first;
        QVERIFY(testAndRelease(parent.second));
        QVERIFY(testAndRelease(child.second));
    }

    // Multiple children
    {
        const WidgetViewPair parent = createAndRetain();
        const WidgetViewPair child1 = createAndRetain(parent.first);
        const WidgetViewPair child2 = createAndRetain(parent.first);

        delete parent.first;
        QVERIFY(testAndRelease(parent.second));
        QVERIFY(testAndRelease(child1.second));
        QVERIFY(testAndRelease(child2.second));
    }

    // Grandchild widget
    {
        const WidgetViewPair parent = createAndRetain();
        const WidgetViewPair child = createAndRetain(parent.first);
        const WidgetViewPair grandchild = createAndRetain(child.first);

        delete parent.first;
        QVERIFY(testAndRelease(parent.second));
        QVERIFY(testAndRelease(child.second));
        QVERIFY(testAndRelease(grandchild.second));
    }

    // Reparent child widget
    {
        const WidgetViewPair parent1 = createAndRetain();
        const WidgetViewPair parent2 = createAndRetain();
        const WidgetViewPair child = createAndRetain(parent1.first);

        child.first->setParent(parent2.first);

        delete parent1.first;
        QVERIFY(testAndRelease(parent1.second));
        delete parent2.first;
        QVERIFY(testAndRelease(parent2.second));
        QVERIFY(testAndRelease(child.second));
    }

    // Reparent window
    {
        const WidgetViewPair window1 = createAndRetain();
        const WidgetViewPair window2 = createAndRetain();
        const WidgetViewPair child1 = createAndRetain(window1.first);
        const WidgetViewPair child2 = createAndRetain(window2.first);

        window2.first->setParent(window1.first);

        delete window2.first;
        QVERIFY(testAndRelease(window2.second));
        QVERIFY(testAndRelease(child2.second));
        delete window1.first;
        QVERIFY(testAndRelease(window1.second));
        QVERIFY(testAndRelease(child1.second));
    }

    // Delete child widget
    {
        const WidgetViewPair parent = createAndRetain();
        const WidgetViewPair child = createAndRetain(parent.first);

        delete child.first;
        QVERIFY(testAndRelease(child.second));
        delete parent.first;
        QVERIFY(testAndRelease(parent.second));
    }
}

#endif

class SiblingDeleter : public QWidget
{
public:
    inline SiblingDeleter(QWidget *sibling, QWidget *parent)
        : QWidget(parent), sibling(sibling) {}
    inline virtual ~SiblingDeleter() { delete sibling; }

private:
    QPointer<QWidget> sibling;
};


void tst_QWidget::childDeletesItsSibling()
{
    QWidget *commonParent = new QWidget(0);
    QPointer<QWidget> child = new QWidget(0);
    QPointer<QWidget> siblingDeleter = new SiblingDeleter(child, commonParent);
    child->setParent(commonParent);
    delete commonParent; // don't crash
    QVERIFY(!child);
    QVERIFY(!siblingDeleter);

}

void tst_QWidget::setMinimumSize()
{
    QWidget w;
    w.setGeometry(10, 10, 100, 100);

    QSize defaultSize = w.size();

    w.setMinimumSize(defaultSize + QSize(100, 100));
    QCOMPARE(w.size(), defaultSize + QSize(100, 100));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setMinimumSize(defaultSize + QSize(50, 50));
    QCOMPARE(w.size(), defaultSize + QSize(100, 100));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setMinimumSize(defaultSize + QSize(200, 200));
    QCOMPARE(w.size(), defaultSize + QSize(200, 200));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));
}

void tst_QWidget::setMaximumSize()
{
    QWidget w;
    w.setGeometry(10, 10, 100, 100);

    QSize defaultSize = w.size();

    w.setMinimumSize(defaultSize + QSize(100, 100));
    QCOMPARE(w.size(), defaultSize + QSize(100, 100));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setMaximumSize(defaultSize + QSize(200, 200));
    QCOMPARE(w.size(), defaultSize + QSize(100, 100));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setMaximumSize(defaultSize + QSize(50, 50));
    QCOMPARE(w.size(), defaultSize + QSize(50, 50));
    QVERIFY(!w.testAttribute(Qt::WA_Resized));
}

void tst_QWidget::setFixedSize()
{
    QWidget w;
    w.setGeometry(10, 10, 100, 100);

    QSize defaultSize = w.size();

    w.setFixedSize(defaultSize + QSize(100, 100));
    QCOMPARE(w.size(), defaultSize + QSize(100, 100));
    QEXPECT_FAIL("", "Bug in <= 4.2", Continue);
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setFixedSize(defaultSize + QSize(200, 200));

    QCOMPARE(w.minimumSize(), defaultSize + QSize(200,200));
    QCOMPARE(w.maximumSize(), defaultSize + QSize(200,200));
    QCOMPARE(w.size(), defaultSize + QSize(200, 200));
    QEXPECT_FAIL("", "Bug in <= 4.2", Continue);
    QVERIFY(!w.testAttribute(Qt::WA_Resized));

    w.setFixedSize(defaultSize + QSize(50, 50));
    QCOMPARE(w.size(), defaultSize + QSize(50, 50));
    QEXPECT_FAIL("", "Bug in <= 4.2", Continue);
    QVERIFY(!w.testAttribute(Qt::WA_Resized));
}

void tst_QWidget::ensureCreated()
{
    {
        QWidget widget;
        WId widgetWinId = widget.winId();
        Q_UNUSED(widgetWinId);
        QVERIFY(widget.testAttribute(Qt::WA_WState_Created));
    }

    {
        QWidget window;

        QDialog dialog(&window);
        dialog.setWindowModality(Qt::NonModal);

        WId dialogWinId = dialog.winId();
        Q_UNUSED(dialogWinId);
        QVERIFY(dialog.testAttribute(Qt::WA_WState_Created));
        QVERIFY(window.testAttribute(Qt::WA_WState_Created));
    }

    {
        QWidget window;

        QDialog dialog(&window);
        dialog.setWindowModality(Qt::WindowModal);

        WId dialogWinId = dialog.winId();
        Q_UNUSED(dialogWinId);
        QVERIFY(dialog.testAttribute(Qt::WA_WState_Created));
        QVERIFY(window.testAttribute(Qt::WA_WState_Created));
    }

    {
        QWidget window;

        QDialog dialog(&window);
        dialog.setWindowModality(Qt::ApplicationModal);

        WId dialogWinId = dialog.winId();
        Q_UNUSED(dialogWinId);
        QVERIFY(dialog.testAttribute(Qt::WA_WState_Created));
        QVERIFY(window.testAttribute(Qt::WA_WState_Created));
    }
}

void tst_QWidget::persistentWinId()
{
    QWidget *parent = new QWidget;
    QWidget *w1 = new QWidget;
    QWidget *w2 = new QWidget;
    QWidget *w3 = new QWidget;
    w1->setParent(parent);
    w2->setParent(w1);
    w3->setParent(w2);

    WId winId1 = w1->winId();
    WId winId2 = w2->winId();
    WId winId3 = w3->winId();

    // reparenting should change the winId of the widget being reparented, but not of its children
    w1->setParent(0);
    QVERIFY(w1->winId() != winId1);
    winId1 = w1->winId();
    QCOMPARE(w2->winId(), winId2);
    QCOMPARE(w3->winId(), winId3);

    w1->setParent(parent);
    QVERIFY(w1->winId() != winId1);
    winId1 = w1->winId();
    QCOMPARE(w2->winId(), winId2);
    QCOMPARE(w3->winId(), winId3);

    w2->setParent(0);
    QVERIFY(w2->winId() != winId2);
    winId2 = w2->winId();
    QCOMPARE(w3->winId(), winId3);

    w2->setParent(parent);
    QVERIFY(w2->winId() != winId2);
    winId2 = w2->winId();
    QCOMPARE(w3->winId(), winId3);

    w2->setParent(w1);
    QVERIFY(w2->winId() != winId2);
    winId2 = w2->winId();
    QCOMPARE(w3->winId(), winId3);

    w3->setParent(0);
    QVERIFY(w3->winId() != winId3);
    winId3 = w3->winId();

    w3->setParent(w1);
    QVERIFY(w3->winId() != winId3);
    winId3 = w3->winId();

    w3->setParent(w2);
    QVERIFY(w3->winId() != winId3);
    winId3 = w3->winId();

    delete parent;
}

class ShowHideEventWidget : public QWidget
{
public:
    int numberOfShowEvents, numberOfHideEvents;
    
    ShowHideEventWidget(QWidget *parent = 0)
        : QWidget(parent), numberOfShowEvents(0), numberOfHideEvents(0)
    { }
    
    void create()
    { QWidget::create(); }
    
    void showEvent(QShowEvent *)
    { ++numberOfShowEvents; }
    
    void hideEvent(QHideEvent *)
    { ++numberOfHideEvents; }
};

void tst_QWidget::showHideEvent_data()
{
    QTest::addColumn<bool>("show");
    QTest::addColumn<bool>("hide");
    QTest::addColumn<bool>("create");
    QTest::addColumn<int>("expectedShowEvents");
    QTest::addColumn<int>("expectedHideEvents");
    
    QTest::newRow("window: only show")
            << true
            << false
            << false
            << 1
            << 0;
    QTest::newRow("window: show/hide")
            << true
            << true
            << false
            << 1
            << 1;
    QTest::newRow("window: show/hide/create")
            << true
            << true
            << true
            << 1
            << 1;
    QTest::newRow("window: hide/create")
            << false
            << true
            << true
            << 0
            << 0;
    QTest::newRow("window: only hide")
            << false
            << true
            << false
            << 0
            << 0;
    QTest::newRow("window: nothing")
            << false
            << false
            << false
            << 0
            << 0;
}

void tst_QWidget::showHideEvent()
{
    QFETCH(bool, show);
    QFETCH(bool, hide);
    QFETCH(bool, create);
    QFETCH(int, expectedShowEvents);
    QFETCH(int, expectedHideEvents);
    
    ShowHideEventWidget widget;
    if (show)
        widget.show();
    if (hide)
        widget.hide();
    if (create && !widget.testAttribute(Qt::WA_WState_Created))
        widget.create();
    
    QCOMPARE(widget.numberOfShowEvents, expectedShowEvents);
    QCOMPARE(widget.numberOfHideEvents, expectedHideEvents);
}

bool wasWidget;
class DestroyedSlotChecker : public QObject
{
Q_OBJECT
    public slots:
    void destroyedSlot(QObject *object)
    {
        wasWidget = (qobject_cast<QWidget *>(object) != 0 || object->isWidgetType());
    }
};

/*
    Test that qobject_cast<QWidget*> returns 0 in a slot
    connected to QObject::destroyed.
*/
void tst_QWidget::qobject_castInDestroyedSlot()
{
    DestroyedSlotChecker checker;
    QWidget *widget = new QWidget();

    QObject::connect(widget, SIGNAL(destroyed(QObject *)), &checker, SLOT(destroyedSlot(QObject *)));
    delete widget;

    QVERIFY(wasWidget == false);
}

void tst_QWidget::setWindowGeometry_data()
{
    QTest::addColumn<QRect>("rect");
    QTest::addColumn<int>("windowFlags");

    QList<QRect> rects;
    rects << QRect(100, 100, 200, 200)
          << QApplication::desktop()->availableGeometry().adjusted(50, 50, -50, -50)
          << QRect(100, 100, 0, 200)
          << QRect(100, 100, 200, 0)
          << QRect(100, 100, 0, 0);

    QList<int> windowFlags;
    windowFlags << 0
                << Qt::FramelessWindowHint
                << Qt::X11BypassWindowManagerHint;

    foreach (QRect rect, rects) {
        foreach (int windowFlag, windowFlags) {
            QTest::newRow(QString("%1,%2 %3x%4, flags %5")
                          .arg(rect.x())
                          .arg(rect.y())
                          .arg(rect.width())
                          .arg(rect.height())
                          .arg(windowFlag, 0, 16))
                << rect
                << windowFlag;
        }
    }
}

void tst_QWidget::setWindowGeometry()
{
    QFETCH(QRect, rect);
    QFETCH(int, windowFlags);

    // we expect that all toplevels will have at least 1x1 size
    QRect expectedRect = rect;
    expectedRect.setSize(rect.size().expandedTo(QSize(1, 1)));

    {
        // test setGeometry() without actually showing the window
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.setGeometry(rect);
        QApplication::processEvents();
        QCOMPARE(widget.geometry(), expectedRect);
    }

    {
        // setGeometry() first, then show()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.setGeometry(rect);
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QApplication::processEvents();
        QCOMPARE(widget.geometry(), expectedRect);
    }

    {
        // show() first, then setGeometry()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        widget.setGeometry(rect);
        QApplication::processEvents();
        QCOMPARE(widget.geometry(), expectedRect);
    }
}

void tst_QWidget::windowMove_data()
{
    setWindowGeometry_data();
}

void tst_QWidget::windowMove()
{
    QFETCH(QRect, rect);
    QFETCH(int, windowFlags);

    {
        // test setGeometry() without actually showing the window
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.resize(rect.size());
        widget.move(rect.topLeft());
        QApplication::processEvents();
        QCOMPARE(widget.pos(), rect.topLeft());
    }

    {
        // setGeometry() first, then show()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.resize(rect.size());
        widget.move(rect.topLeft());
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QApplication::processEvents();
        QCOMPARE(widget.pos(), rect.topLeft());
    }

    {
        // show() first, then setGeometry()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));
        widget.resize(rect.size());
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        widget.move(rect.topLeft());
        QApplication::processEvents();
        QCOMPARE(widget.pos(), rect.topLeft());
    }
}

#ifdef Q_WS_WIN
void tst_QWidget::getDC()
{
    QWidget widget;
    widget.setGeometry(0, 0, 2, 4);

    HDC dc = widget.getDC();
    QVERIFY(dc != 0);

    widget.releaseDC(dc);
}
#endif
QTEST_MAIN(tst_QWidget)
#include "tst_qwidget.moc"
