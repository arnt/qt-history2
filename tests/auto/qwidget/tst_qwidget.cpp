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
#include <qdesktopwidget.h>
#include <private/qbackingstore_p.h>
#include <qcalendarwidget.h>

#ifdef Q_WS_QWS
# include <qscreen_qws.h>
#endif

// I *MUST* have QtTest afterwards or this test won't work with newer headers
#if defined(Q_WS_MAC)
# include <private/qt_mac_p.h>
#undef verify
#endif

#include <QtTest/QtTest>

#if defined(Q_WS_WIN)
#  include <qt_windows.h>
#define Q_CHECK_PAINTEVENTS \
    if (::SwitchDesktop(::GetThreadDesktop(::GetCurrentThreadId())) == 0) \
        QSKIP("desktop is not visible, this test would fail", SkipSingle);
#elif defined(Q_WS_X11)
#  include <private/qt_x11_p.h>
#  include <qx11info_x11.h>
#elif defined(Q_WS_QWS)
# include <qwindowsystem_qws.h>
#endif

#if !defined(Q_WS_WIN)
#define Q_CHECK_PAINTEVENTS
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
    void mapFromAndTo_data();
    void mapFromAndTo();
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

    void restoreVersion1Geometry_data();
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

    void childDeletesItsSibling();

    void setMinimumSize();
    void setMaximumSize();
    void setFixedSize();

    void ensureCreated();
    void persistentWinId();
    void qobject_castInDestroyedSlot();

    void showHideEvent_data();
    void showHideEvent();

    void update();
    void isOpaque();

    // tests QWidget::setGeometry() on windows only
    void setWindowGeometry_data();
    void setWindowGeometry();

    // tests QWidget::move() and resize() on windows only
    void windowMoveResize_data();
    void windowMoveResize();

#ifdef Q_WS_WIN
    void getDC();
    void setGeometry_win();
#endif

    void setLocale();
    void deleteStyle();
    void multipleToplevelFocusCheck();
    void setFocus();
    void setCursor();
    void testWindowIconChangeEventPropagation();
#ifdef Q_WS_X11
    void minAndMaxSizeWithX11BypassWindowManagerHint();
#endif

    void compatibilityChildInsertedEvents();
    void render();

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
#ifdef Q_WS_WIN
    obj1.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    HWND handle = obj1.winId();
    long flags = GetWindowLong(handle, GWL_STYLE);
    QVERIFY(flags & WS_POPUP);
#endif
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

void tst_QWidget::mapFromAndTo_data()
{
    QTest::addColumn<bool>("windowHidden");
    QTest::addColumn<bool>("subWindow1Hidden");
    QTest::addColumn<bool>("subWindow2Hidden");
    QTest::addColumn<bool>("subSubWindowHidden");
    QTest::addColumn<bool>("windowMinimized");
    QTest::addColumn<bool>("subWindow1Minimized");

    QTest::newRow("window 1 sub1 1 sub2 1 subsub 1") << false << false << false << false << false << false;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 1") << true << false << false << false << false << false;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 1") << false << true << false << false << false << false;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 1") << true << true << false << false << false << false;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 1") << false << false << true << false << false << false;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 1") << true << false << true << false << false << false;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 1") << false << true << true << false << false << false;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 1") << true << true << true << false << false << false;
    QTest::newRow("window 1 sub1 1 sub2 1 subsub 0") << false << false << false << true << false << false;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 0") << true << false << false << true << false << false;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 0") << false << true << false << true << false << false;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 0") << true << true << false << true << false << false;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 0") << false << false << true << true << false << false;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 0") << true << false << true << true << false << false;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 0") << false << true << true << true << false << false;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 0") << true << true << true << true << false << false;
    QTest::newRow("window 1 sub1 1 sub2 1 subsub 1 windowMinimized") << false << false << false << false << true << false;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 1 windowMinimized") << true << false << false << false << true << false;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 1 windowMinimized") << false << true << false << false << true << false;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 1 windowMinimized") << true << true << false << false << true << false;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 1 windowMinimized") << false << false << true << false << true << false;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 1 windowMinimized") << true << false << true << false << true << false;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 1 windowMinimized") << false << true << true << false << true << false;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 1 windowMinimized") << true << true << true << false << true << false;
    QTest::newRow("window 1 sub1 1 sub2 1 subsub 0 windowMinimized") << false << false << false << true << true << false;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 0 windowMinimized") << true << false << false << true << true << false;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 0 windowMinimized") << false << true << false << true << true << false;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 0 windowMinimized") << true << true << false << true << true << false;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 0 windowMinimized") << false << false << true << true << true << false;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 0 windowMinimized") << true << false << true << true << true << false;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 0 windowMinimized") << false << true << true << true << true << false;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 0 windowMinimized") << true << true << true << true << true << false;
    QTest::newRow("window 1 sub1 1 sub2 1 subsub 1 subWindow1Minimized") << false << false << false << false << false << true;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 1 subWindow1Minimized") << true << false << false << false << false << true;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 1 subWindow1Minimized") << false << true << false << false << false << true;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 1 subWindow1Minimized") << true << true << false << false << false << true;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 1 subWindow1Minimized") << false << false << true << false << false << true;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 1 subWindow1Minimized") << true << false << true << false << false << true;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 1 subWindow1Minimized") << false << true << true << false << false << true;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 1 subWindow1Minimized") << true << true << true << false << false << true;
    QTest::newRow("window 1 sub1 1 sub2 1 subsub 0 subWindow1Minimized") << false << false << false << true << false << true;
    QTest::newRow("window 0 sub1 1 sub2 1 subsub 0 subWindow1Minimized") << true << false << false << true << false << true;
    QTest::newRow("window 1 sub1 0 sub2 1 subsub 0 subWindow1Minimized") << false << true << false << true << false << true;
    QTest::newRow("window 0 sub1 0 sub2 1 subsub 0 subWindow1Minimized") << true << true << false << true << false << true;
    QTest::newRow("window 1 sub1 1 sub2 0 subsub 0 subWindow1Minimized") << false << false << true << true << false << true;
    QTest::newRow("window 0 sub1 1 sub2 0 subsub 0 subWindow1Minimized") << true << false << true << true << false << true;
    QTest::newRow("window 1 sub1 0 sub2 0 subsub 0 subWindow1Minimized") << false << true << true << true << false << true;
    QTest::newRow("window 0 sub1 0 sub2 0 subsub 0 subWindow1Minimized") << true << true << true << true << false << true;


}

void tst_QWidget::mapFromAndTo()
{
    QFETCH(bool, windowHidden);
    QFETCH(bool, subWindow1Hidden);
    QFETCH(bool, subWindow2Hidden);
    QFETCH(bool, subSubWindowHidden);
    QFETCH(bool, windowMinimized);
    QFETCH(bool, subWindow1Minimized);

    // create a toplevel and two overlapping siblings
    QWidget window;
    QWidget *subWindow1 = new QWidget(&window);
    QWidget *subWindow2 = new QWidget(&window);
    QWidget *subSubWindow = new QWidget(subWindow1);

    // set their geometries
    window.setGeometry(100, 100, 100, 100);
    subWindow1->setGeometry(50, 50, 100, 100);
    subWindow2->setGeometry(75, 75, 100, 100);
    subSubWindow->setGeometry(10, 10, 10, 10);

    // update visibility
    if (windowMinimized) {
        if (!windowHidden) {
            window.showMinimized();
            QVERIFY(window.isMinimized());
        }
    } else {
        window.setVisible(!windowHidden);
    }
    if (subWindow1Minimized) {
        subWindow1->hide();
        subWindow1->showMinimized();
        QVERIFY(subWindow1->isMinimized());
    } else {
        subWindow1->setVisible(!subWindow1Hidden);
    }
    subWindow2->setVisible(!subWindow2Hidden);
    subSubWindow->setVisible(!subSubWindowHidden);

    // window
    QCOMPARE(window.mapToGlobal(QPoint(0, 0)), QPoint(100, 100));
    QCOMPARE(window.mapToGlobal(QPoint(10, 0)), QPoint(110, 100));
    QCOMPARE(window.mapToGlobal(QPoint(0, 10)), QPoint(100, 110));
    QCOMPARE(window.mapToGlobal(QPoint(-10, 0)), QPoint(90, 100));
    QCOMPARE(window.mapToGlobal(QPoint(0, -10)), QPoint(100, 90));
    QCOMPARE(window.mapToGlobal(QPoint(100, 100)), QPoint(200, 200));
    QCOMPARE(window.mapToGlobal(QPoint(110, 100)), QPoint(210, 200));
    QCOMPARE(window.mapToGlobal(QPoint(100, 110)), QPoint(200, 210));
    QCOMPARE(window.mapFromGlobal(QPoint(100, 100)), QPoint(0, 0));
    QCOMPARE(window.mapFromGlobal(QPoint(110, 100)), QPoint(10, 0));
    QCOMPARE(window.mapFromGlobal(QPoint(100, 110)), QPoint(0, 10));
    QCOMPARE(window.mapFromGlobal(QPoint(90, 100)), QPoint(-10, 0));
    QCOMPARE(window.mapFromGlobal(QPoint(100, 90)), QPoint(0, -10));
    QCOMPARE(window.mapFromGlobal(QPoint(200, 200)), QPoint(100, 100));
    QCOMPARE(window.mapFromGlobal(QPoint(210, 200)), QPoint(110, 100));
    QCOMPARE(window.mapFromGlobal(QPoint(200, 210)), QPoint(100, 110));
    QCOMPARE(window.mapToParent(QPoint(0, 0)), QPoint(100, 100));
    QCOMPARE(window.mapToParent(QPoint(10, 0)), QPoint(110, 100));
    QCOMPARE(window.mapToParent(QPoint(0, 10)), QPoint(100, 110));
    QCOMPARE(window.mapToParent(QPoint(-10, 0)), QPoint(90, 100));
    QCOMPARE(window.mapToParent(QPoint(0, -10)), QPoint(100, 90));
    QCOMPARE(window.mapToParent(QPoint(100, 100)), QPoint(200, 200));
    QCOMPARE(window.mapToParent(QPoint(110, 100)), QPoint(210, 200));
    QCOMPARE(window.mapToParent(QPoint(100, 110)), QPoint(200, 210));
    QCOMPARE(window.mapFromParent(QPoint(100, 100)), QPoint(0, 0));
    QCOMPARE(window.mapFromParent(QPoint(110, 100)), QPoint(10, 0));
    QCOMPARE(window.mapFromParent(QPoint(100, 110)), QPoint(0, 10));
    QCOMPARE(window.mapFromParent(QPoint(90, 100)), QPoint(-10, 0));
    QCOMPARE(window.mapFromParent(QPoint(100, 90)), QPoint(0, -10));
    QCOMPARE(window.mapFromParent(QPoint(200, 200)), QPoint(100, 100));
    QCOMPARE(window.mapFromParent(QPoint(210, 200)), QPoint(110, 100));
    QCOMPARE(window.mapFromParent(QPoint(200, 210)), QPoint(100, 110));

    // first subwindow
    QCOMPARE(subWindow1->mapToGlobal(QPoint(0, 0)), QPoint(150, 150));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(10, 0)), QPoint(160, 150));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(0, 10)), QPoint(150, 160));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(-10, 0)), QPoint(140, 150));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(0, -10)), QPoint(150, 140));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(100, 100)), QPoint(250, 250));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(110, 100)), QPoint(260, 250));
    QCOMPARE(subWindow1->mapToGlobal(QPoint(100, 110)), QPoint(250, 260));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(150, 150)), QPoint(0, 0));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(160, 150)), QPoint(10, 0));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(150, 160)), QPoint(0, 10));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(140, 150)), QPoint(-10, 0));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(150, 140)), QPoint(0, -10));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(250, 250)), QPoint(100, 100));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(260, 250)), QPoint(110, 100));
    QCOMPARE(subWindow1->mapFromGlobal(QPoint(250, 260)), QPoint(100, 110));
    QCOMPARE(subWindow1->mapToParent(QPoint(0, 0)), QPoint(50, 50));
    QCOMPARE(subWindow1->mapToParent(QPoint(10, 0)), QPoint(60, 50));
    QCOMPARE(subWindow1->mapToParent(QPoint(0, 10)), QPoint(50, 60));
    QCOMPARE(subWindow1->mapToParent(QPoint(-10, 0)), QPoint(40, 50));
    QCOMPARE(subWindow1->mapToParent(QPoint(0, -10)), QPoint(50, 40));
    QCOMPARE(subWindow1->mapToParent(QPoint(100, 100)), QPoint(150, 150));
    QCOMPARE(subWindow1->mapToParent(QPoint(110, 100)), QPoint(160, 150));
    QCOMPARE(subWindow1->mapToParent(QPoint(100, 110)), QPoint(150, 160));
    QCOMPARE(subWindow1->mapFromParent(QPoint(50, 50)), QPoint(0, 0));
    QCOMPARE(subWindow1->mapFromParent(QPoint(60, 50)), QPoint(10, 0));
    QCOMPARE(subWindow1->mapFromParent(QPoint(50, 60)), QPoint(0, 10));
    QCOMPARE(subWindow1->mapFromParent(QPoint(40, 50)), QPoint(-10, 0));
    QCOMPARE(subWindow1->mapFromParent(QPoint(50, 40)), QPoint(0, -10));
    QCOMPARE(subWindow1->mapFromParent(QPoint(150, 150)), QPoint(100, 100));
    QCOMPARE(subWindow1->mapFromParent(QPoint(160, 150)), QPoint(110, 100));
    QCOMPARE(subWindow1->mapFromParent(QPoint(150, 160)), QPoint(100, 110));

    // subsubwindow
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(0, 0)), QPoint(160, 160));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(10, 0)), QPoint(170, 160));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(0, 10)), QPoint(160, 170));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(-10, 0)), QPoint(150, 160));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(0, -10)), QPoint(160, 150));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(100, 100)), QPoint(260, 260));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(110, 100)), QPoint(270, 260));
    QCOMPARE(subSubWindow->mapToGlobal(QPoint(100, 110)), QPoint(260, 270));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(160, 160)), QPoint(0, 0));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(170, 160)), QPoint(10, 0));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(160, 170)), QPoint(0, 10));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(150, 160)), QPoint(-10, 0));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(160, 150)), QPoint(0, -10));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(260, 260)), QPoint(100, 100));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(270, 260)), QPoint(110, 100));
    QCOMPARE(subSubWindow->mapFromGlobal(QPoint(260, 270)), QPoint(100, 110));
    QCOMPARE(subSubWindow->mapToParent(QPoint(0, 0)), QPoint(10, 10));
    QCOMPARE(subSubWindow->mapToParent(QPoint(10, 0)), QPoint(20, 10));
    QCOMPARE(subSubWindow->mapToParent(QPoint(0, 10)), QPoint(10, 20));
    QCOMPARE(subSubWindow->mapToParent(QPoint(-10, 0)), QPoint(0, 10));
    QCOMPARE(subSubWindow->mapToParent(QPoint(0, -10)), QPoint(10, 0));
    QCOMPARE(subSubWindow->mapToParent(QPoint(100, 100)), QPoint(110, 110));
    QCOMPARE(subSubWindow->mapToParent(QPoint(110, 100)), QPoint(120, 110));
    QCOMPARE(subSubWindow->mapToParent(QPoint(100, 110)), QPoint(110, 120));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(10, 10)), QPoint(0, 0));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(20, 10)), QPoint(10, 0));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(10, 20)), QPoint(0, 10));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(0, 10)), QPoint(-10, 0));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(10, 0)), QPoint(0, -10));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(110, 110)), QPoint(100, 100));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(120, 110)), QPoint(110, 100));
    QCOMPARE(subSubWindow->mapFromParent(QPoint(110, 120)), QPoint(100, 110));
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

    Q3HBox widget;
    QLineEdit *focusWidget = new QLineEdit( &widget );
    new QLineEdit( &widget );
    new QPushButton( &widget );
    focusWidget->setFocus();
    widget.setEnabled( FALSE );
    widget.setEnabled( TRUE );
    widget.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&widget);
#endif
    QTest::qWait( 100 );
    QVERIFY( widget.focusWidget() == focusWidget );
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
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&container);
#endif
    container.setActiveWindow();
    qApp->setActiveWindow(&container);
    qApp->processEvents(QEventLoop::AllEvents);

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
    Q_CHECK_PAINTEVENTS

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
    }

    {
        ResizeWidget wTopLevel;
        wTopLevel.show();
        QCOMPARE (wTopLevel.m_resizeEventCount, 1); // initial resize event before paint for toplevels
        wTopLevel.hide();
        wTopLevel.resize(QSize(640,480));
        QCOMPARE (wTopLevel.m_resizeEventCount, 1);
        wTopLevel.show();
        QCOMPARE (wTopLevel.m_resizeEventCount, 2);
    }
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

class UpdateWidget : public QWidget
{
public:
    UpdateWidget(QWidget *parent = 0) : QWidget(parent) {
        reset();
    }

    void paintEvent(QPaintEvent *e) {
        paintedRegion += e->region();
        ++numPaintEvents;
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::ZOrderChange)
            ++numZOrderChangeEvents;
        return QWidget::event(event);
    }

    void reset() {
        numPaintEvents = 0;
        numZOrderChangeEvents = 0;
        paintedRegion = QRegion();
    }

    int numPaintEvents;
    int numZOrderChangeEvents;
    QRegion paintedRegion;
};

void tst_QWidget::raise()
{
    QTest::qWait(1000);
    QWidget *parent = new QWidget(0);
    QList<UpdateWidget *> allChildren;

    UpdateWidget *child1 = new UpdateWidget(parent);
    child1->setAutoFillBackground(true);
    allChildren.append(child1);

    UpdateWidget *child2 = new UpdateWidget(parent);
    child2->setAutoFillBackground(true);
    allChildren.append(child2);

    UpdateWidget *child3 = new UpdateWidget(parent);
    child3->setAutoFillBackground(true);
    allChildren.append(child3);

    UpdateWidget *child4 = new UpdateWidget(parent);
    child4->setAutoFillBackground(true);
    allChildren.append(child4);

    parent->show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(parent);
#endif
    QApplication::processEvents();

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);
    QCOMPARE(allChildren.count(), list1.count());

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child4 ? 1 : 0;
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, 0);
        child->reset();
    }

    for (int i = 0; i < 5; ++i)
        child2->raise();
    qApp->processEvents();

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child2 ? 1 : 0;
        int expectedZOrderChangeEvents = child == child2 ? 1 : 0;
#ifdef Q_WS_MAC
        if (expectedPaintEvents == 1)
            QEXPECT_FAIL(0, "Carbon Compositor issues double repaints for Z-Order change", Continue);
#endif
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, expectedZOrderChangeEvents);
        child->reset();
    }

    QList<QObject *> list2;
    list2 << child1 << child3 << child4 << child2;
    QVERIFY(parent->children() == list2);

    delete parent;
}

void tst_QWidget::lower()
{
    QWidget *parent = new QWidget(0);
    QList<UpdateWidget *> allChildren;

    UpdateWidget *child1 = new UpdateWidget(parent);
    child1->setAutoFillBackground(true);
    allChildren.append(child1);

    UpdateWidget *child2 = new UpdateWidget(parent);
    child2->setAutoFillBackground(true);
    allChildren.append(child2);

    UpdateWidget *child3 = new UpdateWidget(parent);
    child3->setAutoFillBackground(true);
    allChildren.append(child3);

    UpdateWidget *child4 = new UpdateWidget(parent);
    child4->setAutoFillBackground(true);
    allChildren.append(child4);

    parent->show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(parent);
#endif
    QApplication::processEvents();

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);
    QCOMPARE(allChildren.count(), list1.count());

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child4 ? 1 : 0;
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, 0);
        child->reset();
    }

    for (int i = 0; i < 5; ++i)
        child3->lower();
    qApp->processEvents();

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child4 ? 1 : 0;
        int expectedZOrderChangeEvents = child == child3 ? 1 : 0;
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, expectedZOrderChangeEvents);
        child->reset();
    }

    QList<QObject *> list2;
    list2 << child3 << child1 << child2 << child4;
    QVERIFY(parent->children() == list2);

    delete parent;
}

void tst_QWidget::stackUnder()
{
    QTest::qWait(1000);
    QWidget *parent = new QWidget(0);
    QList<UpdateWidget *> allChildren;

    UpdateWidget *child1 = new UpdateWidget(parent);
    child1->setAutoFillBackground(true);
    allChildren.append(child1);

    UpdateWidget *child2 = new UpdateWidget(parent);
    child2->setAutoFillBackground(true);
    allChildren.append(child2);

    UpdateWidget *child3 = new UpdateWidget(parent);
    child3->setAutoFillBackground(true);
    allChildren.append(child3);

    UpdateWidget *child4 = new UpdateWidget(parent);
    child4->setAutoFillBackground(true);
    allChildren.append(child4);

    parent->show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(parent);
#endif
    QApplication::processEvents();

    QList<QObject *> list1;
    list1 << child1 << child2 << child3 << child4;
    QVERIFY(parent->children() == list1);

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child4 ? 1 : 0;
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, 0);
        child->reset();
    }

    for (int i = 0; i < 5; ++i)
        child4->stackUnder(child2);
    qApp->processEvents();

    QList<QObject *> list2;
    list2 << child1 << child4 << child2 << child3;
    QVERIFY(parent->children() == list2);

    foreach (UpdateWidget *child, allChildren) {
        int expectedPaintEvents = child == child3 ? 1 : 0;
        int expectedZOrderChangeEvents = child == child4 ? 1 : 0;
#ifdef Q_WS_MAC
        if (expectedPaintEvents == 1)
            QEXPECT_FAIL(0, "Carbon Compositor issues double repaints for Z-Order change", Continue);
#endif
        QCOMPARE(child->numPaintEvents, expectedPaintEvents);
        QCOMPARE(child->numZOrderChangeEvents, expectedZOrderChangeEvents);
        child->reset();
    }

    for (int i = 0; i < 5; ++i)
        child1->stackUnder(child3);
    qApp->processEvents();

    QList<QObject *> list3;
    list3 << child4 << child2 << child1 << child3;
    QVERIFY(parent->children() == list3);

    foreach (UpdateWidget *child, allChildren) {
        int expectedZOrderChangeEvents = child == child1 ? 1 : 0;
        if (child == child3) {
#ifndef Q_WS_MAC
            QEXPECT_FAIL(0, "Task 153869", Continue);
#endif
            QCOMPARE(child->numPaintEvents, 0);
        } else {
            QCOMPARE(child->numPaintEvents, 0);
        }
        QCOMPARE(child->numZOrderChangeEvents, expectedZOrderChangeEvents);
        child->reset();
    }

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
#ifdef Q_WS_QWS
    widget.resize(500,500);
#else
    widget.setFixedSize(500, 500);
#endif
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
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);

        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.size(), size);
        savedGeometry = widget.saveGeometry();
    }

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
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);

        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.size(), size);
        widget.show();
        QCOMPARE(widget.pos(), position);
        QCOMPARE(widget.size(), size);
    }

    {
        QWidget widget;
        widget.move(position);
        widget.resize(size);
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);

        QRect geom;

        //Restore from Full screen
        savedGeometry = widget.saveGeometry();
        geom = widget.geometry();
        widget.setWindowState(widget.windowState() | Qt::WindowFullScreen);
        QTest::qWait(100);
        QVERIFY((widget.windowState() & Qt::WindowFullScreen));
        QVERIFY(widget.restoreGeometry(savedGeometry));
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), geom);
        QVERIFY(!(widget.windowState() & Qt::WindowFullScreen));

        //Restore to full screen
        widget.setWindowState(widget.windowState() | Qt::WindowFullScreen);
        QTest::qWait(100);
        QVERIFY((widget.windowState() & Qt::WindowFullScreen));
        savedGeometry = widget.saveGeometry();
        geom = widget.geometry();
        widget.setWindowState(widget.windowState() ^ Qt::WindowFullScreen);
        QTest::qWait(100);
        QVERIFY(widget.restoreGeometry(savedGeometry));
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), geom);
        QVERIFY((widget.windowState() & Qt::WindowFullScreen));
        widget.setWindowState(widget.windowState() ^ Qt::WindowFullScreen);
        QTest::qWait(100);

        //Restore from Maximised
        widget.move(position);
        widget.resize(size);
        QTest::qWait(100);
        savedGeometry = widget.saveGeometry();
        geom = widget.geometry();
        widget.setWindowState(widget.windowState() | Qt::WindowMaximized);
        QTest::qWait(100);
        QVERIFY((widget.windowState() & Qt::WindowMaximized));
        QVERIFY(widget.geometry() != geom);
        QVERIFY(widget.restoreGeometry(savedGeometry));
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), geom);

        QVERIFY(!(widget.windowState() & Qt::WindowMaximized));

        //Restore to maximised
        widget.setWindowState(widget.windowState() | Qt::WindowMaximized);
        QTest::qWait(100);
        QVERIFY((widget.windowState() & Qt::WindowMaximized));
        geom = widget.geometry();
        savedGeometry = widget.saveGeometry();
        widget.setWindowState(widget.windowState() ^ Qt::WindowMaximized);
        QTest::qWait(100);
        QVERIFY(widget.restoreGeometry(savedGeometry));
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), geom);
        QVERIFY((widget.windowState() & Qt::WindowMaximized));
    }
}

void tst_QWidget::restoreVersion1Geometry_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<uint>("expectedWindowState");
    QTest::addColumn<QPoint>("expectedPosition");
    QTest::addColumn<QSize>("expectedSize");
    QTest::addColumn<QRect>("expectedNormalGeometry");
    const QPoint position(100, 100);
    const QSize size(200, 200);
    const QRect normalGeometry(102, 124, 200, 200);

    QTest::newRow("geometry.dat") << ":geometry.dat" << uint(Qt::WindowNoState) << position << size << normalGeometry;
    QTest::newRow("geometry-maximized.dat") << ":geometry-maximized.dat" << uint(Qt::WindowMaximized) << position << size << normalGeometry;
    QTest::newRow("geometry-fullscreen.dat") << ":geometry-fullscreen.dat" << uint(Qt::WindowFullScreen) << position << size << normalGeometry;
}

/*
    Test that the current version of restoreGeometry() can restore geometry
    saved width saveGeometry() version 1.0.
*/
void tst_QWidget::restoreVersion1Geometry()
{
    QFETCH(QString, fileName);
    QFETCH(uint, expectedWindowState);
    QFETCH(QPoint, expectedPosition);
    QFETCH(QSize, expectedSize);
    QFETCH(QRect, expectedNormalGeometry);

    // WindowActive is uninteresting for this test
    const uint WindowStateMask = Qt::WindowFullScreen | Qt::WindowMaximized | Qt::WindowMinimized;

    QFile f(fileName);
    QVERIFY(f.exists());
    f.open(QIODevice::ReadOnly);
    const QByteArray savedGeometry = f.readAll();
    QCOMPARE(savedGeometry.count(), 46);
    f.close();

    QWidget widget;

    QVERIFY(widget.restoreGeometry(savedGeometry));

    QCOMPARE(uint(widget.windowState() & WindowStateMask), expectedWindowState);
    if (expectedWindowState == Qt::WindowNoState) {
        QCOMPARE(widget.pos(), expectedPosition);
        QCOMPARE(widget.size(), expectedSize);
    }
    widget.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&widget);
#endif
    QTest::qWait(100);

    if (expectedWindowState == Qt::WindowNoState) {
        QCOMPARE(widget.pos(), expectedPosition);
        QCOMPARE(widget.size(), expectedSize);
    }

    widget.showNormal();
    QTest::qWait(1000);

    if (expectedWindowState != Qt::WindowNoState) {
        // restoring from maximized or fullscreen, we can only restore to the normal geometry
        QCOMPARE(widget.geometry(), expectedNormalGeometry);
    } else {
        QCOMPARE(widget.pos(), expectedPosition);
        QCOMPARE(widget.size(), expectedSize);
    }

#if 0
    // Code for saving a new geometry*.dat files
    {
        QWidget widgetToSave;
        widgetToSave.move(expectedPosition);
        widgetToSave.resize(expectedSize);
        widgetToSave.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(500); // stabilize
        widgetToSave.setWindowState(Qt::WindowStates(expectedWindowState));
        QTest::qWait(500); // stabilize

        QByteArray geometryToSave = widgetToSave.saveGeometry();

        // Code for saving a new geometry.dat file.
        f.setFileName(fileName.mid(1));
        QVERIFY(f.open(QIODevice::WriteOnly)); // did you forget to 'p4 edit *.dat'? :)
        f.write(geometryToSave);
        f.close();
    }
#endif

}

void tst_QWidget::widgetAt()
{
    Q_CHECK_PAINTEVENTS

    QWidget *w1 = new QWidget(0, Qt::X11BypassWindowManagerHint);
    w1->setGeometry(0,0,150,150);
    w1->setObjectName("w1");

    QWidget *w2 = new QWidget(0, Qt::X11BypassWindowManagerHint  | Qt::FramelessWindowHint);
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
    QMouseEvent me(QEvent::MouseMove, QPoint(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &me);
    QVERIFY(w == 0);
    delete w;
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

#ifdef Q_WS_QWS
# define SET_SAFE_SIZE(w) \
    do { \
        QSize safeSize(qt_screen->width() - 250, qt_screen->height() - 250);      \
         if (!safeSize.isValid()) \
             QSKIP("Screen size too small", SkipAll); \
         if (defaultSize.width() > safeSize.width() || defaultSize.height() > safeSize.height()) { \
             defaultSize = safeSize; \
             w.resize(defaultSize); \
             w.setAttribute(Qt::WA_Resized, false); \
         } \
    } while (false)
#else
# define SET_SAFE_SIZE(w)
#endif


void tst_QWidget::setMinimumSize()
{
    QWidget w;
    QSize defaultSize = w.size();
    SET_SAFE_SIZE(w);

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
    QSize defaultSize = w.size();
    SET_SAFE_SIZE(w);

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
    QSize defaultSize = w.size();
    SET_SAFE_SIZE(w);

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

void tst_QWidget::update()
{
    QTest::qWait(1000);  // Wait for the initStuff to do it's stuff.
    Q_CHECK_PAINTEVENTS

    UpdateWidget w;
    w.setGeometry(50, 50, 100, 100);
    w.show();
    QApplication::processEvents();
    QApplication::processEvents();

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    QCOMPARE(w.numPaintEvents, 2); //the window gets update once more when it is activated
#else
    QCOMPARE(w.numPaintEvents, 1);
#endif
    QCOMPARE(w.visibleRegion(), QRegion(w.rect()));
    QCOMPARE(w.paintedRegion, w.visibleRegion());
    w.reset();

    UpdateWidget child(&w);
    child.setGeometry(10, 10, 80, 80);
    child.show();

    QPoint childOffset = child.mapToParent(QPoint());

    // widgets are transparent by default, so both should get repaints
    {
        QApplication::processEvents();
        QApplication::processEvents();
        QCOMPARE(child.numPaintEvents, 1);
        QCOMPARE(child.visibleRegion(), QRegion(child.rect()));
        QCOMPARE(child.paintedRegion, child.visibleRegion());
        QCOMPARE(w.numPaintEvents, 1);
        QCOMPARE(w.visibleRegion(), QRegion(w.rect()));
        QCOMPARE(w.paintedRegion, child.visibleRegion().translated(childOffset));

        w.reset();
        child.reset();

        w.update();
        QApplication::processEvents();
        QApplication::processEvents();
        QCOMPARE(child.numPaintEvents, 1);
        QCOMPARE(child.visibleRegion(), QRegion(child.rect()));
        QCOMPARE(child.paintedRegion, child.visibleRegion());
        QCOMPARE(w.numPaintEvents, 1);
        QCOMPARE(w.visibleRegion(), QRegion(w.rect()));
        QCOMPARE(w.paintedRegion, w.visibleRegion());
    }

    QPalette opaquePalette = child.palette();
    opaquePalette.setColor(child.backgroundRole(), QColor(Qt::red));

    // setting an opaque background on the child should prevent paint-events
    // for the parent in the child area
    {
        child.setPalette(opaquePalette);
        child.setAutoFillBackground(true);
        QApplication::processEvents();

        w.reset();
        child.reset();

        w.update();
        QApplication::processEvents();
        QApplication::processEvents();

        QCOMPARE(w.numPaintEvents, 1);
        QRegion expectedVisible = QRegion(w.rect())
                                  - child.visibleRegion().translated(childOffset);
#ifdef Q_WS_MAC
        QEXPECT_FAIL("", "Task 151858", Continue);
#endif
        QCOMPARE(w.visibleRegion(), expectedVisible);
        QCOMPARE(w.paintedRegion, expectedVisible);
        QCOMPARE(child.numPaintEvents, 0);

        w.reset();
        child.reset();

        child.update();
        QApplication::processEvents();
        QApplication::processEvents();

        QCOMPARE(w.numPaintEvents, 0);
        QCOMPARE(child.numPaintEvents, 1);
        QCOMPARE(child.paintedRegion, child.visibleRegion());

        w.reset();
        child.reset();
    }

    // overlapping sibling
    UpdateWidget sibling(&w);
    child.setGeometry(10, 10, 20, 20);
    sibling.setGeometry(15, 15, 20, 20);
    sibling.show();

    QApplication::processEvents();
    w.reset();
    child.reset();
    sibling.reset();

    const QPoint siblingOffset = sibling.mapToParent(QPoint());

    sibling.update();
    QApplication::processEvents();
    QApplication::processEvents();

    // child is opaque, sibling transparent
    {
        QCOMPARE(sibling.numPaintEvents, 1);
        QCOMPARE(sibling.paintedRegion, sibling.visibleRegion());

        QCOMPARE(child.numPaintEvents, 1);
        QCOMPARE(child.paintedRegion.translated(childOffset),
                 child.visibleRegion().translated(childOffset)
                 & sibling.visibleRegion().translated(siblingOffset));

        QCOMPARE(w.numPaintEvents, 1);
#ifdef Q_WS_MAC
        QEXPECT_FAIL("", "Task 151858", Continue);
#endif
        QCOMPARE(w.paintedRegion,
                 w.visibleRegion() & sibling.visibleRegion().translated(siblingOffset));
        QCOMPARE(w.paintedRegion,
                 (w.visibleRegion() - child.visibleRegion().translated(childOffset))
                 & sibling.visibleRegion().translated(siblingOffset));

    }
    w.reset();
    child.reset();
    sibling.reset();

    sibling.setPalette(opaquePalette);
    sibling.setAutoFillBackground(true);

    sibling.update();
    QApplication::processEvents();
    QApplication::processEvents();

    // child opaque, sibling opaque
    {
        QCOMPARE(sibling.numPaintEvents, 1);
        QCOMPARE(sibling.paintedRegion, sibling.visibleRegion());

        QCOMPARE(child.numPaintEvents, 0);
        QEXPECT_FAIL("", "Task 151858", Continue);
        QCOMPARE(child.visibleRegion(),
                 QRegion(child.rect())
                 - sibling.visibleRegion().translated(siblingOffset - childOffset));

        QCOMPARE(w.numPaintEvents, 0);
#ifdef Q_WS_MAC
        QEXPECT_FAIL("", "Task 151858", Continue);
#endif
        QCOMPARE(w.visibleRegion(),
                 QRegion(w.rect())
                 - child.visibleRegion().translated(childOffset)
                 - sibling.visibleRegion().translated(siblingOffset));
    }
}

void tst_QWidget::isOpaque()
{
#ifndef Q_WS_MAC
    QWidget w;
    QVERIFY(QWidgetBackingStore::isOpaque(&w));

    QWidget child(&w);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    child.setAutoFillBackground(true);
    QVERIFY(QWidgetBackingStore::isOpaque(&child));

    QPalette palette;

    // background color

    palette = child.palette();
    palette.setColor(child.backgroundRole(), QColor(255, 0, 0, 127));
    child.setPalette(palette);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    palette.setColor(child.backgroundRole(), QColor(255, 0, 0, 255));
    child.setPalette(palette);
    QVERIFY(QWidgetBackingStore::isOpaque(&child));

    palette.setColor(QPalette::Window, QColor(0, 0, 255, 127));
    w.setPalette(palette);

    QVERIFY(!QWidgetBackingStore::isOpaque(&w));

    child.setAutoFillBackground(false);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    // Qt::WA_OpaquePaintEvent

    child.setAttribute(Qt::WA_OpaquePaintEvent);
    QVERIFY(QWidgetBackingStore::isOpaque(&child));

    child.setAttribute(Qt::WA_OpaquePaintEvent, false);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    // Qt::WA_NoSystemBackground

    child.setAttribute(Qt::WA_NoSystemBackground);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    child.setAttribute(Qt::WA_NoSystemBackground, false);
    QVERIFY(!QWidgetBackingStore::isOpaque(&child));

    palette.setColor(QPalette::Window, QColor(0, 0, 255, 255));
    w.setPalette(palette);
    QVERIFY(QWidgetBackingStore::isOpaque(&w));

    w.setAttribute(Qt::WA_NoSystemBackground);
    QVERIFY(!QWidgetBackingStore::isOpaque(&w));

    w.setAttribute(Qt::WA_NoSystemBackground, false);
    QVERIFY(QWidgetBackingStore::isOpaque(&w));
#endif
}

class DestroyedSlotChecker : public QObject
{
    Q_OBJECT

public:
    bool wasQWidget;

    DestroyedSlotChecker()
        : wasQWidget(false)
    {
    }

public slots:
    void destroyedSlot(QObject *object)
    {
        wasQWidget = (qobject_cast<QWidget *>(object) != 0 || object->isWidgetType());
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

    QVERIFY(checker.wasQWidget == true);
}

Q_DECLARE_METATYPE(QList<QRect>)

void tst_QWidget::setWindowGeometry_data()
{
    QTest::addColumn<QList<QRect> >("rects");
    QTest::addColumn<int>("windowFlags");

    QList<QList<QRect> > rects;
    rects << (QList<QRect>()
              << QRect(100, 100, 200, 200)
              << QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100)
              << QRect(50, 100, 0, 200)
              << QRect(100, 50, 200, 0)
              << QRect(50, 50, 0, 0))
          << (QList<QRect>()
              << QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100)
              << QRect(50, 100, 0, 200)
              << QRect(100, 50, 200, 0)
              << QRect(50, 50, 0, 0)
              << QRect(100, 100, 200, 200))
          << (QList<QRect>()
              << QRect(50, 100, 0, 200)
              << QRect(100, 50, 200, 0)
              << QRect(50, 50, 0, 0)
              << QRect(100, 100, 200, 200)
              << QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100))
          << (QList<QRect>()
              << QRect(100, 50, 200, 0)
              << QRect(50, 50, 0, 0)
              << QRect(100, 100, 200, 200)
              << QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100)
              << QRect(50, 100, 0, 200))
          << (QList<QRect>()
              << QRect(50, 50, 0, 0)
              << QRect(100, 100, 200, 200)
              << QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100)
              << QRect(50, 100, 0, 200)
              << QRect(100, 50, 200, 0));

    QList<int> windowFlags;
    windowFlags << 0
                << Qt::FramelessWindowHint
#ifdef Q_WS_X11
                << Qt::X11BypassWindowManagerHint
#endif
                ;

    foreach (QList<QRect> l, rects) {
        QRect rect = l.first();
        foreach (int windowFlag, windowFlags) {
            QTest::newRow(QString("%1,%2 %3x%4, flags %5")
                          .arg(rect.x())
                          .arg(rect.y())
                          .arg(rect.width())
                          .arg(rect.height())
                          .arg(windowFlag, 0, 16))
                << l
                << windowFlag;
        }
    }
}

void tst_QWidget::setWindowGeometry()
{
    QFETCH(QList<QRect>, rects);
    QFETCH(int, windowFlags);
    QRect rect = rects.takeFirst();

    {
        // test setGeometry() without actually showing the window
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));

        widget.setGeometry(rect);
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // setGeometry() without showing
        foreach (QRect r, rects) {
            widget.setGeometry(r);
            QTest::qWait(100);
            QCOMPARE(widget.geometry(), r);
        }
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
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // setGeometry() while shown
        foreach (QRect r, rects) {
            widget.setGeometry(r);
            QTest::qWait(100);
            QCOMPARE(widget.geometry(), r);
        }
        widget.setGeometry(rect);
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // now hide
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // setGeometry() after hide()
        foreach (QRect r, rects) {
            widget.setGeometry(r);
            QTest::qWait(100);
            QCOMPARE(widget.geometry(), r);
        }
        widget.setGeometry(rect);
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // show() again, geometry() should still be the same
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // final hide(), again geometry() should be unchanged
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);
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
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // setGeometry() while shown
        foreach (QRect r, rects) {
            widget.setGeometry(r);
            QTest::qWait(100);
            QCOMPARE(widget.geometry(), r);
        }
        widget.setGeometry(rect);
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // now hide
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // setGeometry() after hide()
        foreach (QRect r, rects) {
            widget.setGeometry(r);
            QTest::qWait(100);
            QCOMPARE(widget.geometry(), r);
        }
        widget.setGeometry(rect);
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // show() again, geometry() should still be the same
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);

        // final hide(), again geometry() should be unchanged
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.geometry(), rect);
    }
}

#ifdef Q_WS_WIN
void tst_QWidget::setGeometry_win()
{
    QWidget widget;
    widget.setGeometry(0, 600, 100,100);
    widget.show();
    widget.setWindowState(widget.windowState() | Qt::WindowMaximized);
    QRect geom = widget.normalGeometry();
    widget.close();
    widget.setGeometry(geom);
    widget.setWindowState(widget.windowState() | Qt::WindowMaximized);
    widget.show();
    RECT rt;
    ::GetWindowRect(widget.internalWinId(), &rt);
    QVERIFY(rt.left <= 0);
    QVERIFY(rt.top <= 0);
}
#endif

void tst_QWidget::windowMoveResize_data()
{
    setWindowGeometry_data();
}

void tst_QWidget::windowMoveResize()
{
    QFETCH(QList<QRect>, rects);
    QFETCH(int, windowFlags);

    QRect rect = rects.takeFirst();

    {
        // test setGeometry() without actually showing the window
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));

        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // move() without showing
        foreach (QRect r, rects) {
            widget.move(r.topLeft());
            widget.resize(r.size());
            QTest::qWait(100);
            QCOMPARE(widget.pos(), r.topLeft());
            QCOMPARE(widget.size(), r.size());
        }
    }

    {
        // move() first, then show()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));

        widget.move(rect.topLeft());
        widget.resize(rect.size());
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);
#ifdef Q_WS_MAC
        QEXPECT_FAIL("50,50 0x0, flags 0",
                     "Showing a window with 0x0 size shifts it up.",
                     Continue);
#endif
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // move() while shown
        foreach (QRect r, rects) {
#ifdef Q_WS_X11
            if ((widget.width() == 0 || widget.height() == 0) && r.width() != 0 && r.height() != 0) {
                QEXPECT_FAIL("50,100 0x200, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
                QEXPECT_FAIL("100,50 200x0, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
                QEXPECT_FAIL("50,50 0x0, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
            }
#endif
            widget.move(r.topLeft());
            widget.resize(r.size());
            QTest::qWait(100);
            QCOMPARE(widget.pos(), r.topLeft());
            QCOMPARE(widget.size(), r.size());
        }
        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // now hide
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // move() after hide()
        foreach (QRect r, rects) {
            widget.move(r.topLeft());
            widget.resize(r.size());
            QTest::qWait(100);
            QCOMPARE(widget.pos(), r.topLeft());
            QCOMPARE(widget.size(), r.size());
        }
        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // show() again, pos() should be the same
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // final hide(), again pos() should be unchanged
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());
    }

    {
        // show() first, then move()
        QWidget widget;
        if (windowFlags != 0)
            widget.setWindowFlags(Qt::WindowFlags(windowFlags));

        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
#ifdef Q_WS_X11
        QEXPECT_FAIL("50,100 0x200, flags 0",
                     "First resize after show of zero-sized gets wrong win_gravity.",
                     Continue);
        QEXPECT_FAIL("100,50 200x0, flags 0",
                     "First resize after show of zero-sized gets wrong win_gravity.",
                     Continue);
        QEXPECT_FAIL("50,50 0x0, flags 0",
                     "First resize after show of zero-sized gets wrong win_gravity.",
                     Continue);
#endif
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // move() while shown
        foreach (QRect r, rects) {
#ifdef Q_WS_X11
            if ((widget.width() == 0 || widget.height() == 0) && r.width() != 0 && r.height() != 0) {
                QEXPECT_FAIL("50,100 0x200, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
                QEXPECT_FAIL("100,50 200x0, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
                QEXPECT_FAIL("50,50 0x0, flags 0",
                             "First resize after show of zero-sized gets wrong win_gravity.",
                             Continue);
            }
#endif
            widget.move(r.topLeft());
            widget.resize(r.size());
            QTest::qWait(100);
            QCOMPARE(widget.pos(), r.topLeft());
            QCOMPARE(widget.size(), r.size());
        }
        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // now hide
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // move() after hide()
        foreach (QRect r, rects) {
            widget.move(r.topLeft());
            widget.resize(r.size());
            QTest::qWait(100);
            QCOMPARE(widget.pos(), r.topLeft());
            QCOMPARE(widget.size(), r.size());
        }
        widget.move(rect.topLeft());
        widget.resize(rect.size());
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // show() again, pos() should be the same
        widget.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(&widget);
#endif
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());

        // final hide(), again pos() should be unchanged
        widget.hide();
        QTest::qWait(100);
        QCOMPARE(widget.pos(), rect.topLeft());
        QCOMPARE(widget.size(), rect.size());
    }
}

void tst_QWidget::deleteStyle()
{
    QWidget widget;
    widget.setStyle(new QWindowsStyle);
    widget.show();
    delete widget.style();
    qApp->processEvents();
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

class TopLevelFocusCheck: public QWidget
{
    Q_OBJECT
public:
    QLineEdit* edit;
    TopLevelFocusCheck(QWidget* parent = 0) : QWidget(parent)
    {
        edit = new QLineEdit(this);
        edit->hide();
        edit->installEventFilter(this);
    }

public slots:
    void mouseDoubleClickEvent ( QMouseEvent * /*event*/ )
    {
        edit->show();
        edit->setFocus(Qt::OtherFocusReason);
        qApp->processEvents();
    }
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == edit && event->type()== QEvent::FocusOut) {
            edit->hide();
            return true;
        }
        return false;
    }
};

void tst_QWidget::multipleToplevelFocusCheck()
{
    TopLevelFocusCheck w1;
    TopLevelFocusCheck w2;

    w1.resize(200, 200);
    w1.show();
    w2.resize(200,200);
    w2.show();


#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&w1);
    qt_x11_wait_for_window_manager(&w2);
#endif
    QTest::qWait(100);

    w1.activateWindow();
    QApplication::setActiveWindow(&w1);
    QApplication::processEvents();
    QTest::mouseDClick(&w1, Qt::LeftButton);
    QCOMPARE(QApplication::focusWidget(), w1.edit);

    w2.activateWindow();
    QApplication::setActiveWindow(&w2);
    QApplication::processEvents();
    QTest::mouseClick(&w2, Qt::LeftButton);
#ifdef Q_WS_QWS
    QEXPECT_FAIL("", "embedded toplevels take focus anyway", Continue);
#endif
    QCOMPARE(QApplication::focusWidget(), (QWidget *)0);

    QTest::mouseDClick(&w2, Qt::LeftButton);
    QCOMPARE(QApplication::focusWidget(), w2.edit);

    w1.activateWindow();
    QApplication::setActiveWindow(&w1);
    QApplication::processEvents();
    QTest::mouseDClick(&w1, Qt::LeftButton);
    QCOMPARE(QApplication::focusWidget(), w1.edit);

    w2.activateWindow();
    QApplication::setActiveWindow(&w2);
    QApplication::processEvents();
    QTest::mouseClick(&w2, Qt::LeftButton);
    QCOMPARE(QApplication::focusWidget(), (QWidget *)0);
}

void tst_QWidget::setFocus()
{
    {
        // move focus to another window
        testWidget->activateWindow();
        QApplication::setActiveWindow(testWidget);
        if (testWidget->focusWidget())
            testWidget->focusWidget()->clearFocus();
        else
            testWidget->clearFocus();

        // window and children never shown, nobody gets focus
        QWidget window;

        QWidget child1(&window);
        child1.setFocusPolicy(Qt::StrongFocus);

        QWidget child2(&window);
        child2.setFocusPolicy(Qt::StrongFocus);

        child1.setFocus();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), &child1);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child2.setFocus();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), &child2);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));
    }

    {
        // window and children show, but window not active, nobody gets focus
        QWidget window;

        QWidget child1(&window);
        child1.setFocusPolicy(Qt::StrongFocus);

        QWidget child2(&window);
        child2.setFocusPolicy(Qt::StrongFocus);

        window.show();

        // note: window may be active, but we don't want it to be
        testWidget->activateWindow();
        QApplication::setActiveWindow(testWidget);
        if (testWidget->focusWidget())
            testWidget->focusWidget()->clearFocus();
        else
            testWidget->clearFocus();

        child1.setFocus();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), &child1);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child2.setFocus();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), &child2);
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));
    }

    {
        // window and children show, but window *is* active, children get focus
        QWidget window;

        QWidget child1(&window);
        child1.setFocusPolicy(Qt::StrongFocus);

        QWidget child2(&window);
        child2.setFocusPolicy(Qt::StrongFocus);

        window.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(testWidget);
        QApplication::setActiveWindow(&window);
#else
        window.activateWindow();
        QApplication::processEvents();
#endif

        child1.setFocus();
        QVERIFY(child1.hasFocus());
        QCOMPARE(window.focusWidget(), &child1);
        QCOMPARE(QApplication::focusWidget(), &child1);

        child2.setFocus();
        QVERIFY(child2.hasFocus());
        QCOMPARE(window.focusWidget(), &child2);
        QCOMPARE(QApplication::focusWidget(), &child2);
    }

    {
        // window shown and active, children created, don't get focus, but get focus when shown
        QWidget window;

        window.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(testWidget);
        QApplication::setActiveWindow(&window);
#else
        window.activateWindow();
#endif

        QWidget child1(&window);
        child1.setFocusPolicy(Qt::StrongFocus);

        QWidget child2(&window);
        child2.setFocusPolicy(Qt::StrongFocus);

        child1.setFocus();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child1.show();
        QApplication::processEvents();
        QVERIFY(child1.hasFocus());
        QCOMPARE(window.focusWidget(), &child1);
        QCOMPARE(QApplication::focusWidget(), &child1);

        child2.setFocus();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), &child1);
        QCOMPARE(QApplication::focusWidget(), &child1);

        child2.show();
        QVERIFY(child2.hasFocus());
        QCOMPARE(window.focusWidget(), &child2);
        QCOMPARE(QApplication::focusWidget(), &child2);
    }

    {
        // window shown and active, children created, don't get focus,
        // even after setFocus(), hide(), then show()
        QWidget window;

        window.show();
#ifdef Q_WS_X11
        qt_x11_wait_for_window_manager(testWidget);
        QApplication::setActiveWindow(&window);
#else
        window.activateWindow();
#endif

        QWidget child1(&window);
        child1.setFocusPolicy(Qt::StrongFocus);

        QWidget child2(&window);
        child2.setFocusPolicy(Qt::StrongFocus);

        child1.setFocus();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child1.hide();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child1.show();
        QVERIFY(!child1.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child2.setFocus();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child2.hide();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));

        child2.show();
        QVERIFY(!child2.hasFocus());
        QCOMPARE(window.focusWidget(), static_cast<QWidget *>(0));
        QCOMPARE(QApplication::focusWidget(), static_cast<QWidget *>(0));
    }
}

void tst_QWidget::setCursor()
{
    {
        QWidget window;
        QWidget child(&window);

        QVERIFY(!window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));

        window.setCursor(window.cursor());
        QVERIFY(window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window.cursor().shape());
    }

    // do it again, but with window show()n
    {
        QWidget window;
        QWidget child(&window);
        window.show();

        QVERIFY(!window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));

        window.setCursor(window.cursor());
        QVERIFY(window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window.cursor().shape());
    }


    {
        QWidget window;
        QWidget child(&window);

        window.setCursor(Qt::WaitCursor);
        QVERIFY(window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window.cursor().shape());
    }

    // same thing again, just with window show()n
    {
        QWidget window;
        QWidget child(&window);

        window.show();
        window.setCursor(Qt::WaitCursor);
        QVERIFY(window.testAttribute(Qt::WA_SetCursor));
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window.cursor().shape());
    }

    // reparenting child should not cause the WA_SetCursor to become set
    {
        QWidget window;
        QWidget window2;
        QWidget child(&window);

        window.setCursor(Qt::WaitCursor);

        child.setParent(0);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), QCursor().shape());

        child.setParent(&window2);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window2.cursor().shape());

            window2.setCursor(Qt::WaitCursor);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window2.cursor().shape());
    }

    // again, with windows show()n
    {
        QWidget window;
        QWidget window2;
        QWidget child(&window);

        window.setCursor(Qt::WaitCursor);
        window.show();

        child.setParent(0);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), QCursor().shape());

        child.setParent(&window2);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window2.cursor().shape());

        window2.show();
        window2.setCursor(Qt::WaitCursor);
        QVERIFY(!child.testAttribute(Qt::WA_SetCursor));
        QCOMPARE(child.cursor().shape(), window2.cursor().shape());
    }
}

class EventSpy : public QObject
{
public:
    EventSpy(QWidget *widget, QEvent::Type event)
        : m_widget(widget), eventToSpy(event), m_count(0)
    {
        if (m_widget)
            m_widget->installEventFilter(this);
    }

    QWidget *widget() const { return m_widget; }
    int count() const { return m_count; }
    void clear() { m_count = 0; }

protected:
    bool eventFilter(QObject *object, QEvent *event)
    {
        if (event->type() == eventToSpy)
            ++m_count;
        return  QObject::eventFilter(object, event);
    }

private:
    QWidget *m_widget;
    QEvent::Type eventToSpy;
    int m_count;
};


void tst_QWidget::testWindowIconChangeEventPropagation()
{
    // Create widget hierarchy.
    QWidget topLevelWidget;
    QWidget topLevelChild(&topLevelWidget);

    QDialog dialog(&topLevelWidget);
    QWidget dialogChild(&dialog);

    QWidgetList widgets;
    widgets << &topLevelWidget << &topLevelChild
            << &dialog << &dialogChild;
    QCOMPARE(widgets.count(), 4);

    // Create spy lists.
    QList <EventSpy *> applicationEventSpies;
    QList <EventSpy *> widgetEventSpies;
    foreach (QWidget *widget, widgets) {
        applicationEventSpies.append(new EventSpy(widget, QEvent::ApplicationWindowIconChange));
        widgetEventSpies.append(new EventSpy(widget, QEvent::WindowIconChange));
    }

    // QApplication::setWindowIcon
    const QIcon windowIcon = qApp->style()->standardIcon(QStyle::SP_TitleBarMenuButton);
    qApp->setWindowIcon(windowIcon);

    for (int i = 0; i < widgets.count(); ++i) {
        // Check QEvent::ApplicationWindowIconChange
        EventSpy *spy = applicationEventSpies.at(i);
        QWidget *widget = spy->widget();
        if (widget->isWindow()) {
            QCOMPARE(spy->count(), 1);
            QCOMPARE(widget->windowIcon(), windowIcon);
        } else {
            QCOMPARE(spy->count(), 0);
        }
        spy->clear();

        // Check QEvent::WindowIconChange
        spy = widgetEventSpies.at(i);
        QCOMPARE(spy->count(), 1);
        spy->clear();
    }

    // Set icon on a top-level widget.
    topLevelWidget.setWindowIcon(*new QIcon);

    for (int i = 0; i < widgets.count(); ++i) {
        // Check QEvent::ApplicationWindowIconChange
        EventSpy *spy = applicationEventSpies.at(i);
        QCOMPARE(spy->count(), 0);
        spy->clear();

        // Check QEvent::WindowIconChange
        spy = widgetEventSpies.at(i);
        QWidget *widget = spy->widget();
        if (widget == &topLevelWidget) {
            QCOMPARE(widget->windowIcon(), QIcon());
            QCOMPARE(spy->count(), 1);
        } else if (topLevelWidget.isAncestorOf(widget)) {
            QCOMPARE(spy->count(), 1);
        } else {
            QCOMPARE(spy->count(), 0);
        }
        spy->clear();
    }

    // Cleanup.
    for (int i = 0; i < widgets.count(); ++i) {
        delete applicationEventSpies.at(i);
        delete widgetEventSpies.at(i);
    }
}

#ifdef Q_WS_X11
void tst_QWidget::minAndMaxSizeWithX11BypassWindowManagerHint()
{
    // Same size as in QWidget::create_sys().
    const QSize desktopSize = QApplication::desktop()->size();
    const QSize originalSize(desktopSize.width() / 2, desktopSize.height() * 4 / 10);

    { // Maximum size.
    QWidget widget(0, Qt::X11BypassWindowManagerHint);

    const QSize newMaximumSize = widget.size().boundedTo(originalSize) - QSize(10, 10);
    widget.setMaximumSize(newMaximumSize);
    QCOMPARE(widget.size(), newMaximumSize);

    widget.show();
    qt_x11_wait_for_window_manager(&widget);
    QCOMPARE(widget.size(), newMaximumSize);
    }

    { // Minimum size.
    QWidget widget(0, Qt::X11BypassWindowManagerHint);

    const QSize newMinimumSize = widget.size().expandedTo(originalSize) + QSize(10, 10);
    widget.setMinimumSize(newMinimumSize);
    QCOMPARE(widget.size(), newMinimumSize);

    widget.show();
    qt_x11_wait_for_window_manager(&widget);
    QCOMPARE(widget.size(), newMinimumSize);
    }
}
#endif

class EventRecorder : public QObject
{
    Q_OBJECT

public:
    typedef QList<QPair<QWidget *, QEvent::Type> > EventList;

    EventRecorder(QObject *parent = 0)
        : QObject(parent)
    { }

    EventList eventList()
    {
        return events;
    }

    void clear()
    {
        events.clear();
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        QWidget *widget = qobject_cast<QWidget *>(object);
        if (widget && !event->spontaneous())
            events.append(qMakePair(widget, event->type()));
        return false;
    }

private:
    EventList events;
};

void tst_QWidget::compatibilityChildInsertedEvents()
{
    EventRecorder::EventList expected;

    {
        // no children created, not shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        QCoreApplication::sendPostedEvents();

        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Polish)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1));
        QCOMPARE(spy.eventList(), expected);
    }

    {
        // no children, shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        widget.show();
        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::Polish)
            << qMakePair(&widget, QEvent::Move)
            << qMakePair(&widget, QEvent::Resize)
            << qMakePair(&widget, QEvent::Show)
            << qMakePair(&widget, QEvent::ShowToParent);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        QCoreApplication::sendPostedEvents();
        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1))
#if defined(Q_WS_X11)
            << qMakePair(&widget, QEvent::UpdateRequest)
#endif
            ;
        QCOMPARE(spy.eventList(), expected);
    }

    {
        // 2 children, not shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        QWidget child1(&widget);
        QWidget child2;
        child2.setParent(&widget);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 2)));

        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildAdded);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        QCoreApplication::sendPostedEvents();
        expected =
            EventRecorder::EventList()
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInsertedRequest)
            << qMakePair(&widget, QEvent::ChildInserted)
            << qMakePair(&widget, QEvent::ChildInserted)
#endif
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Polish)
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1))
            << qMakePair(&widget, QEvent::Type(QEvent::User + 2));
        QCOMPARE(spy.eventList(), expected);
    }

    {
        // 2 children, widget shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        QWidget child1(&widget);
        QWidget child2;
        child2.setParent(&widget);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 2)));

        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildAdded);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        widget.show();
        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::Polish)
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInserted)
            << qMakePair(&widget, QEvent::ChildInserted)
#endif
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::Move)
            << qMakePair(&widget, QEvent::Resize)
            << qMakePair(&widget, QEvent::Show)
            << qMakePair(&widget, QEvent::ShowToParent);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        QCoreApplication::sendPostedEvents();
        expected =
            EventRecorder::EventList()
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInsertedRequest)
#endif
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1))
            << qMakePair(&widget, QEvent::Type(QEvent::User + 2))
#if defined(Q_WS_X11)
            << qMakePair(&widget, QEvent::UpdateRequest)
#endif
            ;
        QCOMPARE(spy.eventList(), expected);
    }

    {
        // 2 children, but one is reparented away, not shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        QWidget child1(&widget);
        QWidget child2;
        child2.setParent(&widget);
        child2.setParent(0);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 2)));

        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildRemoved);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        QCoreApplication::sendPostedEvents();
        expected =
            EventRecorder::EventList()
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInsertedRequest)
            << qMakePair(&widget, QEvent::ChildInserted)
#endif
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Polish)
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1))
            << qMakePair(&widget, QEvent::Type(QEvent::User + 2));
        QCOMPARE(spy.eventList(), expected);
    }

    {
        // 2 children, but one is reparented away, then widget is shown
        QWidget widget;
        EventRecorder spy;
        widget.installEventFilter(&spy);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 1)));

        QWidget child1(&widget);
        QWidget child2;
        child2.setParent(&widget);
        child2.setParent(0);

        QCoreApplication::postEvent(&widget, new QEvent(QEvent::Type(QEvent::User + 2)));

        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildAdded)
            << qMakePair(&widget, QEvent::ChildRemoved);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        widget.show();
        expected =
            EventRecorder::EventList()
            << qMakePair(&widget, QEvent::Polish)
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInserted)
#endif
            << qMakePair(&widget, QEvent::ChildPolished)
            << qMakePair(&widget, QEvent::Move)
            << qMakePair(&widget, QEvent::Resize)
            << qMakePair(&widget, QEvent::Show)
            << qMakePair(&widget, QEvent::ShowToParent);
        QCOMPARE(spy.eventList(), expected);
        spy.clear();

        QCoreApplication::sendPostedEvents();
        expected =
            EventRecorder::EventList()
#ifdef QT_HAS_QT3SUPPORT
            << qMakePair(&widget, QEvent::ChildInsertedRequest)
#endif
            << qMakePair(&widget, QEvent::PolishRequest)
            << qMakePair(&widget, QEvent::Type(QEvent::User + 1))
            << qMakePair(&widget, QEvent::Type(QEvent::User + 2))
#if defined(Q_WS_X11)
            << qMakePair(&widget, QEvent::UpdateRequest)
#endif
            ;
        QCOMPARE(spy.eventList(), expected);
    }
}

class RenderWidget : public QWidget
{
public:
    RenderWidget(QWidget *source)
        : source(source), ellipse(false) {}

    void setEllipseEnabled(bool enable = true)
    {
        ellipse = enable;
        update();
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        if (ellipse) {
            QPainter painter(this);
            painter.fillRect(rect(), Qt::red);
            painter.end();
            QRegion regionToRender = QRegion(0, 0, source->width(), source->height() / 2,
                                             QRegion::Ellipse);
            source->render(this, QPoint(0, 30), regionToRender);
        } else {
            source->render(this);
        }
    }

private:
    QWidget *source;
    bool ellipse;
};

void tst_QWidget::render()
{
    QCalendarWidget source;
    source.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&source);
#endif

    // Render the entire source into target.
    RenderWidget target(&source);
    target.resize(source.size());
    target.show();
#ifdef Q_WS_X11
    qt_x11_wait_for_window_manager(&target);
#endif

    qApp->processEvents();
    qApp->sendPostedEvents();
    QTest::qWait(500);

    QImage sourceImage = QPixmap::grabWidget(&source).toImage();
    QImage targetImage = QPixmap::grabWidget(&target).toImage();
    QCOMPARE(sourceImage, targetImage);

    // Fill target.rect() will Qt::red and render
    // QRegion(0, 0, source->width(), source->height() / 2, QRegion::Ellipse)
    // of source into target with offset (0, 30).
    target.setEllipseEnabled();
    qApp->processEvents();
    qApp->sendPostedEvents();

    targetImage = QPixmap::grabWidget(&target).toImage();
    QVERIFY(sourceImage != targetImage);

    QCOMPARE(targetImage.pixel(target.width() / 2, 29), QColor(Qt::red).rgb());
    QCOMPARE(targetImage.pixel(target.width() / 2, 30), sourceImage.pixel(source.width() / 2, 0));
}

QTEST_MAIN(tst_QWidget)
#include "tst_qwidget.moc"
