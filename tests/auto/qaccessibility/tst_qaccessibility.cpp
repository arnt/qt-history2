/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <Qt3Support/Qt3Support>
#include <QtTest/QtTest>

#include <math.h>

#include "QtTest/qtestaccessible.h"
#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif


static inline bool verifyChild(QWidget *child, QAccessibleInterface *interface,
                               int index, const QRect &domain)
{
    if (!child) {
        qWarning("tst_QAccessibility::verifyChild: null pointer to child.");
        return false;
    }

    if (!interface) {
        qWarning("tst_QAccessibility::verifyChild: null pointer to interface.");
        return false;
    }

    // QAccessibleInterface::childAt():
    // Calculate global child position and check that the interface
    // returns the correct index for that position.
    QPoint globalChildPos = child->mapToGlobal(QPoint(0, 0));
    int indexFromChildAt = interface->childAt(globalChildPos.x(), globalChildPos.y());
    if (indexFromChildAt != index) {
        qWarning("tst_QAccessibility::verifyChild (childAt()):");
        qWarning() << "Expected:" << index;
        qWarning() << "Actual:  " << indexFromChildAt;
        return false;
    }

    // QAccessibleInterface::rect():
    // Calculate global child geometry and check that the interface
    // returns a QRect which is equal to the calculated QRect.
    const QRect expectedGlobalRect = QRect(globalChildPos, child->size());
    const QRect rectFromInterface = interface->rect(index);
    if (expectedGlobalRect != rectFromInterface) {
        qWarning("tst_QAccessibility::verifyChild (rect()):");
        qWarning() << "Expected:" << expectedGlobalRect;
        qWarning() << "Actual:  " << rectFromInterface;
        return false;
    }

    // Verify that the child is within its domain.
    if (!domain.contains(rectFromInterface)) {
        qWarning("tst_QAccessibility::verifyChild: Child is not within its domain.");
        return false;
    }

    // Verify that we get a valid QAccessibleInterface for the child.
    QAccessibleInterface *childInterface = QAccessible::queryAccessibleInterface(child);
    if (!childInterface) {
        qWarning("tst_QAccessibility::verifyChild: Failed to retrieve interface for child.");
        return false;
    }

    // QAccessibleInterface::indexOfChild():
    // Verify that indexOfChild() returns an index equal to the index passed by,
    // or -1 if child is "Self" (index == 0).
    int indexFromIndexOfChild = interface->indexOfChild(childInterface);
    delete childInterface;
    int expectedIndex = index == 0 ? -1 : index;
    if (indexFromIndexOfChild != expectedIndex) {
        qWarning("tst_QAccessibility::verifyChild (indexOfChild()):");
        qWarning() << "Expected:" << expectedIndex;
        qWarning() << "Actual:  " << indexFromIndexOfChild;
        return false;
    }
    return true;
}

static inline int indexOfChild(QAccessibleInterface *parentInterface, QWidget *childWidget)
{
    if (!parentInterface || !childWidget)
        return -1;
    QAccessibleInterface *childInterface = QAccessibleInterface::queryAccessibleInterface(childWidget);
    if (!childInterface)
        return -1;
    int index = parentInterface->indexOfChild(childInterface);
    delete childInterface;
    return index;
}


//TESTED_FILES=

class tst_QAccessibility : public QObject
{
    Q_OBJECT
public:
    tst_QAccessibility();
    virtual ~tst_QAccessibility();



public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void eventTest();
    void customWidget();
    void deletedWidget();

    void childCount();
    void childAt(); // also indexOfChild
    void relationTo();
    void navigateGeometric();
    void navigateHierarchy();
    void navigateSlider();
    void navigateCovered();
    void navigateControllers();
    void navigateLabels();
    void text();
    void setText();
    void hideShowTest();

    void userActionCount();
    void actionText();
    void doAction();

    void buttonTest();
    void sliderTest();
    void scrollBarTest();
    void tabTest();
    void menuTest();
    void spinBoxTest();
    void doubleSpinBoxTest();
    void textEditTest();
    void textBrowserTest();
    void listViewTest();
    void mdiAreaTest();
    void mdiSubWindowTest();
    void lineEditTest();
    void workspaceTest();
    void dialogButtonBoxTest();
    void dialTest();
    void rubberBandTest();
    void abstractScrollAreaTest();
    void scrollAreaTest();
    void tableWidgetTest();
    void tableViewTest();
    void calendarWidgetTest();

private:
    QWidget *createGUI();
};

const double Q_PI = 3.14159265358979323846;

QString eventName(const int ev)
{
    switch(ev) {
    case 0x0001: return "SoundPlayed";
    case 0x0002: return "Alert";
    case 0x0003: return "ForegroundChanged";
    case 0x0004: return "MenuStart";
    case 0x0005: return "MenuEnd";
    case 0x0006: return "PopupMenuStart";
    case 0x0007: return "PopupMenuEnd";
    case 0x000C: return "ContextHelpStart";
    case 0x000D: return "ContextHelpEnd";
    case 0x000E: return "DragDropStart";
    case 0x000F: return "DragDropEnd";
    case 0x0010: return "DialogStart";
    case 0x0011: return "DialogEnd";
    case 0x0012: return "ScrollingStart";
    case 0x0013: return "ScrollingEnd";
    case 0x0018: return "MenuCommand";
    case 0x8000: return "ObjectCreated";
    case 0x8001: return "ObjectDestroyed";
    case 0x8002: return "ObjectShow";
    case 0x8003: return "ObjectHide";
    case 0x8004: return "ObjectReorder";
    case 0x8005: return "Focus";
    case 0x8006: return "Selection";
    case 0x8007: return "SelectionAdd";
    case 0x8008: return "SelectionRemove";
    case 0x8009: return "SelectionWithin";
    case 0x800A: return "StateChanged";
    case 0x800B: return "LocationChanged";
    case 0x800C: return "NameChanged";
    case 0x800D: return "DescriptionChanged";
    case 0x800E: return "ValueChanged";
    case 0x800F: return "ParentChanged";
    case 0x80A0: return "HelpChanged";
    case 0x80B0: return "DefaultActionChanged";
    case 0x80C0: return "AcceleratorChanged";
    default: return "Unknown Event";
    }
}

static QString stateNames(int state)
{
    QString stateString;
    if (state == 0x00000000) stateString += " Normal";
    if (state & 0x00000001) stateString += " Unavailable";
    if (state & 0x00000002) stateString += " Selected";
    if (state & 0x00000004) stateString += " Focused";
    if (state & 0x00000008) stateString += " Pressed";
    if (state & 0x00000010) stateString += " Checked";
    if (state & 0x00000020) stateString += " Mixed";
    if (state & 0x00000040) stateString += " ReadOnly";
    if (state & 0x00000080) stateString += " HotTracked";
    if (state & 0x00000100) stateString += " DefaultButton";
    if (state & 0x00000200) stateString += " Expanded";
    if (state & 0x00000400) stateString += " Collapsed";
    if (state & 0x00000800) stateString += " Busy";
    if (state & 0x00001000) stateString += " Floating";
    if (state & 0x00002000) stateString += " Marqueed";
    if (state & 0x00004000) stateString += " Animated";
    if (state & 0x00008000) stateString += " Invisible";
    if (state & 0x00010000) stateString += " Offscreen";
    if (state & 0x00020000) stateString += " Sizeable";
    if (state & 0x00040000) stateString += " Moveable";
    if (state & 0x00080000) stateString += " SelfVoicing";
    if (state & 0x00100000) stateString += " Focusable";
    if (state & 0x00200000) stateString += " Selectable";
    if (state & 0x00400000) stateString += " Linked";
    if (state & 0x00800000) stateString += " Traversed";
    if (state & 0x01000000) stateString += " MultiSelectable";
    if (state & 0x02000000) stateString += " ExtSelectable";
    if (state & 0x04000000) stateString += " AlertLow";
    if (state & 0x08000000) stateString += " AlertMedium";
    if (state & 0x10000000) stateString += " AlertHigh";
    if (state & 0x20000000) stateString += " Protected";
    if (state & 0x3fffffff) stateString += " Valid";

    if (stateString.isEmpty())
        stateString = "Unknown state " + QString::number(state);

    return stateString;
}

QAccessible::State state(QWidget * const widget)
{
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(widget);
    Q_ASSERT(iface);
    QAccessible::State state = iface->state(0);
    delete iface;
    return state;
}

void printState(QWidget * const widget)
{
    qDebug() << "State for" << widget->className() << stateNames(state(widget));
}

void printState(QAccessibleInterface * const iface, const int child = 0)
{
    qDebug() << "State for" << iface->object()->className() << "child" << child
             <<  iface->text(QAccessible::Name, child) << stateNames(iface->state(child));
}


class QtTestAccessibleWidget: public QWidget
{
    Q_OBJECT
public:
    QtTestAccessibleWidget(QWidget *parent, const char *name): QWidget(parent, name)
    {
	setBackgroundColor(QColor("black")); //black is beautiful
	setFixedSize(5, 5);
    }
};

#ifdef QTEST_ACCESSIBILITY
class QtTestAccessibleWidgetIface: public QAccessibleWidget
{
public:
    QtTestAccessibleWidgetIface(QtTestAccessibleWidget *w): QAccessibleWidget(w) {}
    QString text(Text t, int control) const
    {
	if (t == Help)
	    return QString::fromLatin1("Help yourself");
	return QAccessibleWidget::text(t, control);
    }
    static QAccessibleInterface *ifaceFactory(const QString &key, QObject *o)
    {
	if (key == "QtTestAccessibleWidget")
	    return new QtTestAccessibleWidgetIface(static_cast<QtTestAccessibleWidget*>(o));
	return 0;
    }
};
#endif

tst_QAccessibility::tst_QAccessibility()
{
}

tst_QAccessibility::~tst_QAccessibility()
{
}

void tst_QAccessibility::initTestCase()
{
#ifdef QTEST_ACCESSIBILITY
    QTestAccessibility::initialize();
    QAccessible::installFactory(QtTestAccessibleWidgetIface::ifaceFactory);
#endif
}

void tst_QAccessibility::cleanupTestCase()
{
#ifdef QTEST_ACCESSIBILITY
    QTestAccessibility::cleanup();
#endif
}

void tst_QAccessibility::init()
{
    QTestAccessibility::clearEvents();
}

void tst_QAccessibility::cleanup()
{
#ifdef QTEST_ACCESSIBILITY
    const EventList list = QTestAccessibility::events();
    if (!list.isEmpty()) {
	qWarning("%d accessibility event(s) were not handled in testfunction '%s':", list.count(),
		 QString(QTest::currentTestFunction()).ascii());
	for (int i = 0; i < list.count(); ++i)
	    qWarning(" %d: Object: %p Event: '%s' (%d) Child: %d", i + 1, list.at(i).object,
		     eventName(list.at(i).event).ascii(), list.at(i).event, list.at(i).child);
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::eventTest()
{
#ifdef QTEST_ACCESSIBILITY
    QPushButton* button = new QPushButton(0, "Olaf");

    button->show();
    QVERIFY_EVENT(button, 0, QAccessible::ObjectShow);

    QTest::mouseClick(button, Qt::LeftButton, 0, QPoint(button->width()-7,button->height()-5));
    QVERIFY_EVENT(button, 0, QAccessible::StateChanged);
    QVERIFY_EVENT(button, 0, QAccessible::StateChanged);

    button->hide();
    QVERIFY_EVENT(button, 0, QAccessible::ObjectHide);

    delete button;
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::customWidget()
{
#ifdef QTEST_ACCESSIBILITY
    QtTestAccessibleWidget* widget = new QtTestAccessibleWidget(0, "Heinz");

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(widget);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)widget);
    QCOMPARE(iface->object()->name(), "Heinz");
    QCOMPARE(iface->text(QAccessible::Help, 0).latin1(), "Help yourself");

    delete iface;
    delete widget;
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::deletedWidget()
{
#ifdef QTEST_ACCESSIBILITY
    QtTestAccessibleWidget *widget = new QtTestAccessibleWidget(0, "Ralf");
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(widget);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)widget);

    delete widget;
    widget = 0;
    QVERIFY(!iface->isValid());
    delete iface;
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

QWidget *tst_QAccessibility::createGUI()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = new QWidget(0, Qt::X11BypassWindowManagerHint);
    QGridLayout *grid = new QGridLayout(toplevel, 2, 2);

    // topLeft - hierarchies
    Q3VBox *topLeft = new Q3VBox(toplevel, "topLeft");
    grid->addWidget(topLeft, 0, 0);

	Q3VButtonGroup *group1 = new Q3VButtonGroup("Title1:", topLeft, "group1");
        /*QPushButton *pb1 = */ new QPushButton("Button&1", group1, "pb1");
	Q3VButtonGroup *group2 = new Q3VButtonGroup("Title2:", topLeft, "group2");
	/*QPushButton *pb2 = */ new QPushButton("Button2", group2, "pb2");

	Q3WidgetStack *stack = new Q3WidgetStack(topLeft, "stack");
	    QLabel *page1 = new QLabel("Page 1", stack, "page1");
	    stack->addWidget(page1);
	    QLabel *page2 = new QLabel("Page 2", stack, "page2");
	    stack->addWidget(page2);
	    QLabel *page3 = new QLabel("Page 3", stack, "page3");
	    stack->addWidget(page3);

    // topRight - controlling
    Q3VBox *topRight= new Q3VBox(toplevel, "topRight");
    grid->addWidget(topRight, 0, 1);

	QPushButton *pbOk = new QPushButton("Ok", topRight, "pbOk" );
	pbOk->setDefault(TRUE);
	QSlider *slider = new QSlider(Qt::Horizontal, topRight, "slider");
	QLCDNumber *sliderLcd = new QLCDNumber(topRight, "sliderLcd");
	QSpinBox *spinBox = new QSpinBox(topRight, "spinBox");

	connect(pbOk, SIGNAL(clicked()), toplevel, SLOT(close()) );
	connect(slider, SIGNAL(valueChanged(int)), sliderLcd, SLOT(display(int)));
	connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

        spinBox->setValue(50);

    // bottomLeft - labeling and controlling
    Q3HBox *bottomLeft = new Q3HBox(toplevel, "bottomLeft");
    grid->addWidget(bottomLeft, 1, 0);

	QLabel *label = new QLabel("This is a &lineedit:", bottomLeft, "label");
	QLineEdit *lineedit = new QLineEdit(bottomLeft, "lineedit");
	label->setBuddy(lineedit);
	QLabel *label2 = new QLabel(bottomLeft, "label2");

	connect(lineedit, SIGNAL(textChanged(const QString&)), label2, SLOT(setText(const QString&)));

	Q3VButtonGroup *radiogroup = new Q3VButtonGroup("Exclusive &choices:", bottomLeft, "radiogroup");
	    QLineEdit *frequency = new QLineEdit(radiogroup, "frequency");
	    frequency->setText("100 Mhz");
	    QRadioButton *radioAM = new QRadioButton("&AM", radiogroup, "radioAM");
	    /* QRadioButton *radioFM = */ new QRadioButton("&FM", radiogroup, "radioFM");
	    /* QRadioButton *radioSW = */ new QRadioButton("&Shortwave", radiogroup, "radioSW");

    // bottomRight - ### empty
    Q3HBox *bottomRight = new Q3HBox(toplevel, "bottomRight");
    grid->addWidget(bottomRight, 1, 1);

    toplevel->adjustSize(); // sends layout and child events

    // some tooltips
    QToolTip::add(label, "A label");
    QToolTip::add(lineedit, "A line edit");
    // some whatsthis
    QWhatsThis::add(label, "A label displays static text");
    QWhatsThis::add(frequency, "You can enter a single line of text here");

    radioAM->setFocus();
    QTestAccessibility::clearEvents();
    return toplevel;
#else
    Q_ASSERT(0); // this function cannot be called without accessibility support
    return 0;
#endif
}

void tst_QAccessibility::childAt()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = createGUI();
    QAccessibleInterface *acc_toplevel = QAccessible::queryAccessibleInterface(toplevel);
    QVERIFY(acc_toplevel);
    // this is necessary to have the layout setup correctly
    toplevel->show();

    QObjectList children = toplevel->queryList("QWidget", 0, 0, 0);
    for (int c = 1; c <= children.count(); ++c) {
	QWidget *child = qobject_cast<QWidget*>(children.at(c-1));
	QAccessibleInterface *acc_child = QAccessible::queryAccessibleInterface(child);
	QVERIFY(acc_child);
	QCOMPARE(acc_child->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask, QAccessible::Child);

	QPoint center(child->mapToGlobal(child->rect().center()));
	QRect childRect(child->geometry());
	childRect.moveCenter(center);

	QCOMPARE(acc_child->rect(0), childRect);
	QCOMPARE(acc_toplevel->childAt(childRect.center().x(), childRect.center().y()), c);
	QCOMPARE(acc_toplevel->childAt(childRect.left(), childRect.top()), c);
	QCOMPARE(acc_toplevel->childAt(childRect.left(), childRect.bottom()), c);
	QCOMPARE(acc_toplevel->childAt(childRect.right(), childRect.top()), c);
	QCOMPARE(acc_toplevel->childAt(childRect.right(), childRect.bottom()), c);

	QCOMPARE(acc_toplevel->indexOfChild(acc_child), c);
        delete acc_child;
    }

    delete acc_toplevel;
    delete toplevel;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::childCount()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = createGUI();
    QObject *topLeft = toplevel->child("topLeft");
    QObject *topRight = toplevel->child("topRight");
    QObject *bottomLeft = toplevel->child("bottomLeft");
    QObject *bottomRight = toplevel->child("bottomRight");

    QAccessibleInterface* acc_toplevel = QAccessible::queryAccessibleInterface(toplevel);
    QAccessibleInterface* acc_topLeft = QAccessible::queryAccessibleInterface(topLeft);
    QAccessibleInterface* acc_topRight = QAccessible::queryAccessibleInterface(topRight);
    QAccessibleInterface* acc_bottomLeft = QAccessible::queryAccessibleInterface(bottomLeft);
    QAccessibleInterface* acc_bottomRight = QAccessible::queryAccessibleInterface(bottomRight);

    QVERIFY(acc_toplevel);
    QVERIFY(acc_topLeft);
    QVERIFY(acc_topRight);
    QVERIFY(acc_bottomLeft);
    QVERIFY(acc_bottomRight);

    QCOMPARE(acc_toplevel->childCount(), 0);
    toplevel->show();
    QCOMPARE(acc_toplevel->childCount(), toplevel->queryList("QWidget", 0, 0, 0).count());
    QCOMPARE(acc_topLeft->childCount(), topLeft->queryList("QWidget", 0, 0, 0).count());
    QCOMPARE(acc_topRight->childCount(), topRight->queryList("QWidget", 0, 0, 0).count());
    QCOMPARE(acc_bottomLeft->childCount(), bottomLeft->queryList("QWidget", 0, 0, 0).count());
    QCOMPARE(acc_bottomRight->childCount(), bottomRight->queryList("QWidget", 0, 0, 0).count());

    delete acc_toplevel;
    delete acc_topLeft;
    delete acc_topRight;
    delete acc_bottomLeft;
    delete acc_bottomRight;
    delete toplevel;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::relationTo()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = createGUI();
    QObject *topLeft = toplevel->child("topLeft");
    QObject *topRight = toplevel->child("topRight");
    QObject *bottomLeft = toplevel->child("bottomLeft");
    QObject *bottomRight = toplevel->child("bottomRight");

    toplevel->show();

    QAccessibleInterface *acc_toplevel = QAccessible::queryAccessibleInterface(toplevel);

	QAccessibleInterface *acc_topLeft = QAccessible::queryAccessibleInterface(topLeft);
	    QAccessibleInterface *acc_group1 = QAccessible::queryAccessibleInterface(topLeft->child("group1"));
            QVERIFY(topLeft->child("group1"));
		QAccessibleInterface *acc_pb1 = QAccessible::queryAccessibleInterface(topLeft->child("group1")->child("pb1"));
	    QAccessibleInterface *acc_group2 = QAccessible::queryAccessibleInterface(topLeft->child("group2"));
	    QAccessibleInterface *acc_pb2 = 0;
	    QAccessibleInterface *acc_stack = QAccessible::queryAccessibleInterface(topLeft->child("stack"));
		QAccessibleInterface *acc_page1 = QAccessible::queryAccessibleInterface(topLeft->child("stack")->child("page1"));
		QAccessibleInterface *acc_page2 = QAccessible::queryAccessibleInterface(topLeft->child("stack")->child("page2"));
		QAccessibleInterface *acc_page3 = QAccessible::queryAccessibleInterface(topLeft->child("stack")->child("page3"));
	QAccessibleInterface *acc_topRight = QAccessible::queryAccessibleInterface(topRight);
	    QAccessibleInterface *acc_pbOk = QAccessible::queryAccessibleInterface(topRight->child("pbOk"));
	    QAccessibleInterface *acc_slider = QAccessible::queryAccessibleInterface(topRight->child("slider"));
	    QAccessibleInterface *acc_spinBox = QAccessible::queryAccessibleInterface(topRight->child("spinBox"));
	    QAccessibleInterface *acc_sliderLcd = QAccessible::queryAccessibleInterface(topRight->child("sliderLcd"));

	QAccessibleInterface *acc_bottomLeft = QAccessible::queryAccessibleInterface(bottomLeft);
	    QAccessibleInterface *acc_label = QAccessible::queryAccessibleInterface(bottomLeft->child("label"));
	    QAccessibleInterface *acc_lineedit = QAccessible::queryAccessibleInterface(bottomLeft->child("lineedit"));
	    QAccessibleInterface *acc_label2 = QAccessible::queryAccessibleInterface(bottomLeft->child("label2"));
	    QAccessibleInterface *acc_radiogroup = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup"));
		QAccessibleInterface *acc_radioAM = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("radioAM"));
		QAccessibleInterface *acc_radioFM = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("radioFM"));
		QAccessibleInterface *acc_radioSW = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("radioSW"));
		QAccessibleInterface *acc_frequency = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("frequency"));

	QAccessibleInterface *acc_bottomRight = QAccessible::queryAccessibleInterface(bottomRight);

    QVERIFY(acc_toplevel);
    QVERIFY(acc_topLeft);
    QVERIFY(acc_topRight);
    QVERIFY(acc_bottomLeft);
    QVERIFY(acc_bottomRight);
    QVERIFY(acc_group1);
    QVERIFY(acc_group2);
    QVERIFY(acc_stack);
    QVERIFY(acc_page1);
    QVERIFY(acc_page2);
    QVERIFY(acc_page3);
    QVERIFY(acc_pbOk);
    QVERIFY(acc_slider);
    QVERIFY(acc_spinBox);
    QVERIFY(acc_sliderLcd);
    QVERIFY(acc_label);
    QVERIFY(acc_lineedit);
    QVERIFY(acc_radiogroup);
    QVERIFY(acc_radioAM);
    QVERIFY(acc_radioFM);
    QVERIFY(acc_radioSW);
    QVERIFY(acc_frequency);

    // hierachy relations
    QCOMPARE(acc_toplevel->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Self);
    QCOMPARE(acc_toplevel->relationTo(1, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);
    QCOMPARE(acc_toplevel->relationTo(0, acc_toplevel, 1) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);

    QCOMPARE(acc_toplevel->relationTo(0, acc_topLeft, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);
    QCOMPARE(acc_toplevel->relationTo(0, acc_topRight, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);
    QCOMPARE(acc_toplevel->relationTo(0, acc_bottomLeft, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);
    QCOMPARE(acc_toplevel->relationTo(0, acc_bottomRight, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);

    QCOMPARE(acc_toplevel->relationTo(0, acc_group1, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);
    QCOMPARE(acc_toplevel->relationTo(0, acc_page1, 0) & QAccessible::HierarchyMask,
	QAccessible::Ancestor);
    QCOMPARE(acc_group1->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Descendent);
    QCOMPARE(acc_stack->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Descendent);
    QCOMPARE(acc_page1->relationTo(0, acc_stack, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);
    QCOMPARE(acc_page1->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Descendent);

    QCOMPARE(acc_topLeft->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);
    QCOMPARE(acc_topRight->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);
    QCOMPARE(acc_bottomLeft->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);
    QCOMPARE(acc_bottomRight->relationTo(0, acc_toplevel, 0) & QAccessible::HierarchyMask,
	QAccessible::Child);

    QCOMPARE(acc_topLeft->relationTo(0, acc_topRight, 0) & QAccessible::HierarchyMask,
	QAccessible::Sibling);
    QCOMPARE(acc_topLeft->relationTo(0, acc_bottomLeft, 0) & QAccessible::HierarchyMask,
	QAccessible::Sibling);
    QCOMPARE(acc_topLeft->relationTo(0, acc_bottomRight, 0) & QAccessible::HierarchyMask,
	QAccessible::Sibling);

    QCOMPARE(acc_pb1->relationTo(0, acc_pb2, 0), QAccessible::Unrelated);

    // geometrical relations - only valid for siblings
    QCOMPARE(acc_topLeft->relationTo(0, acc_topRight, 0), QAccessible::Sibling | QAccessible::Left);
    QCOMPARE(acc_topLeft->relationTo(0, acc_bottomLeft, 0), QAccessible::Sibling | QAccessible::Up);
    QCOMPARE(acc_topLeft->relationTo(0, acc_bottomRight, 0), QAccessible::Sibling | QAccessible::Left | QAccessible::Up);

    QCOMPARE(acc_bottomRight->relationTo(0, acc_topLeft, 0), QAccessible::Sibling | QAccessible::Right | QAccessible::Down);
    QCOMPARE(acc_bottomRight->relationTo(0, acc_topRight, 0), QAccessible::Sibling | QAccessible::Down);
    QCOMPARE(acc_bottomRight->relationTo(0, acc_bottomLeft, 0), QAccessible::Sibling | QAccessible::Right);
    QCOMPARE(acc_group1->relationTo(0, acc_group2, 0), QAccessible::Sibling | QAccessible::Up);
    QCOMPARE(acc_group2->relationTo(0, acc_group1, 0), QAccessible::Sibling | QAccessible::Down);

    // Covers/Covered tested in navigateCovered

    // logical relations - focus
    QCOMPARE(acc_radioAM->relationTo(0, acc_radioFM, 0) & QAccessible::FocusChild,
	QAccessible::Unrelated);
    QCOMPARE(acc_radioAM->relationTo(0, acc_radiogroup, 0) & QAccessible::FocusChild,
	QAccessible::FocusChild);
    QCOMPARE(acc_radioAM->relationTo(0, acc_bottomLeft, 0) & QAccessible::FocusChild,
	QAccessible::FocusChild);
    QCOMPARE(acc_radioAM->relationTo(0, acc_topLeft, 0) & QAccessible::FocusChild,
	QAccessible::Unrelated);
    QCOMPARE(acc_radioAM->relationTo(0, acc_toplevel, 0) & QAccessible::FocusChild,
	QAccessible::FocusChild);

    // logical relations - labels
    QCOMPARE(acc_label->relationTo(0, acc_lineedit, 0) & QAccessible::LogicalMask,
	QAccessible::Label);
    QCOMPARE(acc_lineedit->relationTo(0, acc_label, 0) & QAccessible::LogicalMask,
	QAccessible::Labelled);
    QCOMPARE(acc_label->relationTo(0, acc_radiogroup, 0) & QAccessible::LogicalMask,
	QAccessible::Unrelated);
    QCOMPARE(acc_lineedit->relationTo(0, acc_lineedit, 0) & QAccessible::LogicalMask,
	QAccessible::Unrelated);

    QEXPECT_FAIL("", "Make me accessible", Continue);
    QCOMPARE(acc_radiogroup->relationTo(0, acc_radioAM, 0) & QAccessible::LogicalMask,
        QAccessible::Label | QAccessible::Controlled);
    QEXPECT_FAIL("", "Make me accessible", Continue);
    QCOMPARE(acc_radiogroup->relationTo(0, acc_radioFM, 0) & QAccessible::LogicalMask,
        QAccessible::Label | QAccessible::Controlled);
    QEXPECT_FAIL("", "Make me accessible", Continue);
    QCOMPARE(acc_radiogroup->relationTo(0, acc_radioSW, 0) & QAccessible::LogicalMask,
        QAccessible::Label | QAccessible::Controlled);
    QCOMPARE(acc_radiogroup->relationTo(0, acc_frequency, 0) & QAccessible::LogicalMask,
	QAccessible::Label);
    QCOMPARE(acc_frequency->relationTo(0, acc_radiogroup, 0) & QAccessible::LogicalMask,
	QAccessible::Labelled);
    QCOMPARE(acc_radiogroup->relationTo(0, acc_lineedit, 0) & QAccessible::LogicalMask,
	QAccessible::Unrelated);

    // logical relations - controller
    QCOMPARE(acc_pbOk->relationTo(0, acc_toplevel, 0) & QAccessible::LogicalMask,
	QAccessible::Controller);
    QCOMPARE(acc_slider->relationTo(0, acc_sliderLcd, 0) & QAccessible::LogicalMask,
	QAccessible::Controller);
    QCOMPARE(acc_spinBox->relationTo(0, acc_slider, 0) & QAccessible::LogicalMask,
	QAccessible::Controller);
    QCOMPARE(acc_lineedit->relationTo(0, acc_label2, 0) & QAccessible::LogicalMask,
	QAccessible::Controller);

    delete acc_toplevel;
    delete acc_topLeft;
    delete acc_group1;
    delete acc_pb1;
    delete acc_group2;
    delete acc_stack;
    delete acc_page1;
    delete acc_page2;
    delete acc_page3;
    delete acc_topRight;
    delete acc_pbOk;
    delete acc_slider;
    delete acc_spinBox;
    delete acc_sliderLcd;
    delete acc_bottomLeft;
    delete acc_label;
    delete acc_lineedit;
    delete acc_label2;
    delete acc_radiogroup;
    delete acc_radioAM;
    delete acc_radioFM;
    delete acc_radioSW;
    delete acc_frequency;
    delete acc_bottomRight;

    delete toplevel;

    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateGeometric()
{
#ifdef QTEST_ACCESSIBILITY
    {
    static const int skip = 20; //speed the test up significantly
    static const double step = Q_PI / 180;
    QWidget *w = new QWidget(0, "Josef");
    w->setFixedSize(400, 400);

    // center widget
    QtTestAccessibleWidget *center = new QtTestAccessibleWidget(w, "Sol");
    center->move(200, 200);

    // arrange 360 widgets around it in a circle
    QtTestAccessibleWidget *aw = 0;
    int i;
    for (i = 0; i < 360; i += skip) {
	aw = new QtTestAccessibleWidget(w, QString::number(i).latin1());
	aw->move( int(200.0 + 100.0 * sin(step * (double)i)), int(200.0 + 100.0 * cos(step * (double)i)) );
    }

    aw = new QtTestAccessibleWidget(w, "Earth");
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(center);
    QAccessibleInterface *target = 0;
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());

    w->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(w);
#endif


    // let one widget rotate around center
    for (i = 0; i < 360; i+=skip) {
	aw->move( int(200.0 + 75.0 * sin(step * (double)i)), int(200.0 + 75.0 * cos(step * (double)i)) );

	if (i < 45 || i > 315) {
	    QCOMPARE(iface->navigate(QAccessible::Down, 0, &target), 0);
	} else if ( i < 135 ) {
	    QCOMPARE(iface->navigate(QAccessible::Right, 0, &target), 0);
	} else if ( i < 225 ) {
	    QCOMPARE(iface->navigate(QAccessible::Up, 0, &target), 0);
	} else {
	    QCOMPARE(iface->navigate(QAccessible::Left, 0, &target), 0);
        }

	QVERIFY(target);
	QVERIFY(target->isValid());
	QVERIFY(target->object());
	QCOMPARE(target->object()->name(), aw->name());
        delete target; target = 0;
    }

    // test invisible widget
    target = QAccessible::queryAccessibleInterface(aw);
    QVERIFY(!(target->state(0) & QAccessible::Invisible));
    aw->hide();
    QVERIFY(target->state(0) & QAccessible::Invisible);
    delete target; target = 0;

    aw->move(center->x() + 10, center->y());
    QCOMPARE(iface->navigate(QAccessible::Right, 0, &target), 0);
    QVERIFY(target);
    QVERIFY(target->isValid());
    QVERIFY(target->object());
    QVERIFY(QString(target->object()->name()) != "Earth");
    delete target; target = 0;

    aw->move(center->x() - 10, center->y());
    QCOMPARE(iface->navigate(QAccessible::Left, 0, &target), 0);
    QVERIFY(target);
    QVERIFY(target->isValid());
    QVERIFY(target->object());
    QVERIFY(QString(target->object()->name()) != "Earth");
    delete target; target = 0;

    aw->move(center->x(), center->y() + 10);
    QCOMPARE(iface->navigate(QAccessible::Down, 0, &target), 0);
    QVERIFY(target);
    QVERIFY(target->isValid());
    QVERIFY(target->object());
    QVERIFY(QString(target->object()->name()) != "Earth");
    delete target; target = 0;

    aw->move(center->x(), center->y() - 10);
    QCOMPARE(iface->navigate(QAccessible::Up, 0, &target), 0);
    QVERIFY(target);
    QVERIFY(target->isValid());
    QVERIFY(target->object());
    QVERIFY(QString(target->object()->name()) != "Earth");
    delete target; target = 0;

    delete iface;
    delete w;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateSlider()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QSlider *slider = new QSlider(0, "Slidy");
    slider->show();
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(slider);
    QAccessibleInterface *target = 0;
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->childCount(), 3);
    QCOMPARE(iface->navigate(QAccessible::Child, 1, &target), 1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 2, &target), 2);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 3, &target), 3);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 4, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 0, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, -42, &target), -1);
    QVERIFY(target == 0);

    delete iface;
    delete slider;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateCovered()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QWidget *w = new QWidget(0, "Harry");
    QWidget *w1 = new QWidget(w, "1");
    QWidget *w2 = new QWidget(w, "2");
    w->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(w);
#endif

    w->setFixedSize(6, 6);
    w1->setFixedSize(5, 5);
    w2->setFixedSize(5, 5);
    w2->move(0, 0);
    w1->raise();

    QAccessibleInterface *iface1 = QAccessible::queryAccessibleInterface(w1);
    QVERIFY(iface1 != 0);
    QVERIFY(iface1->isValid());
    QAccessibleInterface *iface2 = QAccessible::queryAccessibleInterface(w2);
    QVERIFY(iface2 != 0);
    QVERIFY(iface2->isValid());
    QAccessibleInterface *iface3 = 0;

    QCOMPARE(iface1->navigate(QAccessible::Covers, -42, &iface3), -1);
    QVERIFY(iface3 == 0);
    QCOMPARE(iface1->navigate(QAccessible::Covers, 0, &iface3), -1);
    QVERIFY(iface3 == 0);
    QCOMPARE(iface1->navigate(QAccessible::Covers, 2, &iface3), -1);
    QVERIFY(iface3 == 0);

    for (int loop = 0; loop < 2; ++loop) {
	for (int x = 0; x < w->width(); ++x) {
	    for (int y = 0; y < w->height(); ++y) {
		w1->move(x, y);
		if (w1->geometry().intersects(w2->geometry())) {
		    QVERIFY(iface1->relationTo(0, iface2, 0) & QAccessible::Covers);
		    QVERIFY(iface2->relationTo(0, iface1, 0) & QAccessible::Covered);
		    QCOMPARE(iface1->navigate(QAccessible::Covered, 1, &iface3), 0);
		    QVERIFY(iface3 != 0);
		    QVERIFY(iface3->isValid());
		    QCOMPARE(iface3->object(), iface2->object());
                    delete iface3; iface3 = 0;
		    QCOMPARE(iface2->navigate(QAccessible::Covers, 1, &iface3), 0);
		    QVERIFY(iface3 != 0);
		    QVERIFY(iface3->isValid());
		    QCOMPARE(iface3->object(), iface1->object());
                    delete iface3; iface3 = 0;
		} else {
		    QVERIFY(!(iface1->relationTo(0, iface2, 0) & QAccessible::Covers));
		    QVERIFY(!(iface2->relationTo(0, iface1, 0) & QAccessible::Covered));
		    QCOMPARE(iface1->navigate(QAccessible::Covered, 1, &iface3), -1);
		    QVERIFY(iface3 == 0);
		    QCOMPARE(iface1->navigate(QAccessible::Covers, 1, &iface3), -1);
		    QVERIFY(iface3 == 0);
		    QCOMPARE(iface2->navigate(QAccessible::Covered, 1, &iface3), -1);
		    QVERIFY(iface3 == 0);
		    QCOMPARE(iface2->navigate(QAccessible::Covers, 1, &iface3), -1);
		    QVERIFY(iface3 == 0);
		}
	    }
	}
	if (!loop) {
	    // switch children for second loop
	    w2->raise();
	    QAccessibleInterface *temp = iface1;
	    iface1 = iface2;
	    iface2 = temp;
	}
    }
    delete iface1; iface1 = 0;
    delete iface2; iface2 = 0;
    iface1 = QAccessible::queryAccessibleInterface(w1);
    QVERIFY(iface1 != 0);
    QVERIFY(iface1->isValid());
    iface2 = QAccessible::queryAccessibleInterface(w2);
    QVERIFY(iface2 != 0);
    QVERIFY(iface2->isValid());

    w1->move(0,0);
    w2->move(0,0);
    w1->raise();
    QVERIFY(iface1->relationTo(0, iface2, 0) & QAccessible::Covers);
    QVERIFY(iface2->relationTo(0, iface1, 0) & QAccessible::Covered);
    QVERIFY(!(iface1->state(0) & QAccessible::Invisible));
    w1->hide();
    QVERIFY(iface1->state(0) & QAccessible::Invisible);
    QVERIFY(!(iface1->relationTo(0, iface2, 0) & QAccessible::Covers));
    QVERIFY(!(iface2->relationTo(0, iface1, 0) & QAccessible::Covered));
    QCOMPARE(iface2->navigate(QAccessible::Covered, 1, &iface3), -1);
    QVERIFY(iface3 == 0);
    QCOMPARE(iface1->navigate(QAccessible::Covers, 1, &iface3), -1);
    QVERIFY(iface3 == 0);

    delete iface1; iface1 = 0;
    delete iface2; iface2 = 0;
    delete w;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateHierarchy()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QWidget *w = new QWidget(0, "Hans");
    w->show();
    QWidget *w1 = new QWidget(w, "1");
    w1->show();
    QWidget *w2 = new QWidget(w, "2");
    w2->show();
    QWidget *w3 = new QWidget(w, "3");
    w3->show();
    QWidget *w31 = new QWidget(w3, "31");
    w31->show();

    QAccessibleInterface *target = 0;
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(w);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());

    QCOMPARE(iface->navigate(QAccessible::Sibling, -42, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Sibling, 42, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 15, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 0, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Child, 1, &target), 0);
    QVERIFY(target != 0);
    QVERIFY(target->isValid());
    QCOMPARE(target->object(), (QObject*)w1);
    delete iface; iface = 0;

    QCOMPARE(target->navigate(QAccessible::Sibling, 0, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Sibling, 42, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Sibling, -42, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Sibling, 2, &iface), 0);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)w2);
    delete target; target = 0;
    QCOMPARE(iface->navigate(QAccessible::Sibling, 3, &target), 0);
    QVERIFY(target != 0);
    QVERIFY(target->isValid());
    QCOMPARE(target->object(), (QObject*)w3);
    delete iface; iface = 0;

    QCOMPARE(target->navigate(QAccessible::Child, 1, &iface), 0);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)w31);
    delete target; target = 0;

    QCOMPARE(iface->navigate(QAccessible::Sibling, -1, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Sibling, 0, &target), -1);
    QVERIFY(target == 0);
    QCOMPARE(iface->navigate(QAccessible::Sibling, 1, &target), 0);
    QVERIFY(target != 0);
    QVERIFY(target->isValid());
    QCOMPARE(target->object(), (QObject*)w31);
    delete iface; iface = 0;

    QCOMPARE(target->navigate(QAccessible::Ancestor, 42, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Ancestor, -1, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Ancestor, 0, &iface), -1);
    QVERIFY(iface == 0);
    QCOMPARE(target->navigate(QAccessible::Ancestor, 1, &iface), 0);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)w3);
    delete iface; iface = 0;
    QCOMPARE(target->navigate(QAccessible::Ancestor, 2, &iface), 0);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)w);
    delete iface; iface = 0;
    QCOMPARE(target->navigate(QAccessible::Ancestor, 3, &iface), 0);
    QVERIFY(iface != 0);
    QVERIFY(iface->isValid());
    QCOMPARE(iface->object(), (QObject*)qApp);
    delete iface; iface = 0;
    delete target; target = 0;

    delete w;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateControllers()
{
#ifdef QTEST_ACCESSIBILITY
    {
    Q3VBox vbox;
    QSlider	slider(&vbox);
    QSpinBox	spinBox(&vbox);
    QLCDNumber	lcd1(&vbox);
    QLCDNumber	lcd2(&vbox);
    QLabel	label(&vbox);
    vbox.show();

    slider.setObjectName("slider");
    spinBox.setObjectName("spinBox");
    lcd1.setObjectName("lcd1");
    lcd2.setObjectName("lcd2");
    label.setObjectName("label");

    QTestAccessibility::clearEvents();

    connect(&slider, SIGNAL(valueChanged(int)), &lcd1, SLOT(display(int)));
    connect(&slider, SIGNAL(valueChanged(int)), &lcd2, SLOT(display(int)));
    connect(&spinBox, SIGNAL(valueChanged(int)), &lcd2, SLOT(display(int)));
    connect(&spinBox, SIGNAL(valueChanged(int)), &lcd1, SLOT(display(int)));
    connect(&spinBox, SIGNAL(valueChanged(const QString&)), &label, SLOT(setText(const QString&)));

    QAccessibleInterface *acc_slider = QAccessible::queryAccessibleInterface(&slider);
    QAccessibleInterface *acc_spinBox = QAccessible::queryAccessibleInterface(&spinBox);
    QAccessibleInterface *acc_lcd1 = QAccessible::queryAccessibleInterface(&lcd1);
    QAccessibleInterface *acc_lcd2 = QAccessible::queryAccessibleInterface(&lcd2);
    QAccessibleInterface *acc_label = QAccessible::queryAccessibleInterface(&label);

    QVERIFY(acc_slider->relationTo(0, acc_lcd1, 0) & QAccessible::Controller);
    QVERIFY(acc_slider->relationTo(0, acc_lcd2, 0) & QAccessible::Controller);
    QVERIFY(acc_spinBox->relationTo(0, acc_lcd1, 0) & QAccessible::Controller);
    QVERIFY(acc_spinBox->relationTo(0, acc_lcd2, 0) & QAccessible::Controller);
    QVERIFY(acc_spinBox->relationTo(0, acc_label, 0) & QAccessible::Controller);

    QAccessibleInterface *acc_target;
    // from controller
    QCOMPARE(acc_slider->navigate(QAccessible::Controlled, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_slider->navigate(QAccessible::Controlled, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lcd2->object());
    delete acc_target;
    QCOMPARE(acc_slider->navigate(QAccessible::Controlled, 2, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lcd1->object());
    delete acc_target;
    QCOMPARE(acc_slider->navigate(QAccessible::Controlled, 3, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_spinBox->navigate(QAccessible::Controlled, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_spinBox->navigate(QAccessible::Controlled, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lcd1->object());
    delete acc_target;
    QCOMPARE(acc_spinBox->navigate(QAccessible::Controlled, 2, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lcd2->object());
    delete acc_target;
    QCOMPARE(acc_spinBox->navigate(QAccessible::Controlled, 3, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_label->object());
    delete acc_target;
    QCOMPARE(acc_spinBox->navigate(QAccessible::Controlled, 4, &acc_target), -1);
    QVERIFY(!acc_target);

    // to controller
    QCOMPARE(acc_lcd1->navigate(QAccessible::Controller, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_lcd1->navigate(QAccessible::Controller, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_spinBox->object());
    delete acc_target;
    QCOMPARE(acc_lcd1->navigate(QAccessible::Controller, 2, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_slider->object());
    delete acc_target;
    QCOMPARE(acc_lcd1->navigate(QAccessible::Controller, 3, &acc_target), -1);
    QVERIFY(!acc_target);

    delete acc_label;
    delete acc_lcd2;
    delete acc_lcd1;
    delete acc_spinBox;
    delete acc_slider;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::navigateLabels()
{
#ifdef QTEST_ACCESSIBILITY
    {
    Q3VBox vbox;
    Q3HBox hbox(&vbox);

    QLabel	label(&hbox);
    label.setText("This is a lineedit:");
    QLineEdit	lineedit(&hbox);
    label.setBuddy(&lineedit);

    Q3VButtonGroup groupbox(&vbox);
    groupbox.setTitle("Be my children!");
    QRadioButton radio(&groupbox);
    QLabel	label2(&groupbox);
    label2.setText("Another lineedit:");
    QLineEdit	lineedit2(&groupbox);
    label2.setBuddy(&lineedit2);
    Q3GroupBox groupbox2(&groupbox);
    groupbox2.setTitle("Some grand-children");
    QLineEdit	grandchild(&groupbox2);

    Q3GroupBox border(&vbox);
    QLineEdit	lineedit3(&border);
    vbox.show();
    QTestAccessibility::clearEvents();

    QAccessibleInterface *acc_label = QAccessible::queryAccessibleInterface(&label);
    QAccessibleInterface *acc_lineedit = QAccessible::queryAccessibleInterface(&lineedit);
    QAccessibleInterface *acc_groupbox = QAccessible::queryAccessibleInterface(&groupbox);
    QAccessibleInterface *acc_radio = QAccessible::queryAccessibleInterface(&radio);
    QAccessibleInterface *acc_label2 = QAccessible::queryAccessibleInterface(&label2);
    QAccessibleInterface *acc_lineedit2 = QAccessible::queryAccessibleInterface(&lineedit2);
    QAccessibleInterface *acc_groupbox2 = QAccessible::queryAccessibleInterface(&groupbox2);
    QAccessibleInterface *acc_grandchild = QAccessible::queryAccessibleInterface(&grandchild);
    QAccessibleInterface *acc_border = QAccessible::queryAccessibleInterface(&border);
    QAccessibleInterface *acc_lineedit3 = QAccessible::queryAccessibleInterface(&lineedit3);

    QVERIFY(acc_label->relationTo(0, acc_lineedit,0) & QAccessible::Label);
    QVERIFY(acc_groupbox->relationTo(0, acc_radio,0) & QAccessible::Label);
    QVERIFY(acc_groupbox->relationTo(0, acc_lineedit2,0) & QAccessible::Label);
    QVERIFY(acc_groupbox->relationTo(0, acc_groupbox2,0) & QAccessible::Label);
    QVERIFY(acc_groupbox2->relationTo(0, acc_grandchild,0) & QAccessible::Label);
    QVERIFY(!(acc_border->relationTo(0, acc_lineedit3,0) & QAccessible::Label));

    QAccessibleInterface *acc_target;
    // from label
    QCOMPARE(acc_label->navigate(QAccessible::Labelled, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_label->navigate(QAccessible::Labelled, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lineedit->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_label->navigate(QAccessible::Labelled, 2, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_radio->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 2, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_label2->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 3, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_lineedit2->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 4, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_groupbox2->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_groupbox->navigate(QAccessible::Labelled, 5, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_border->navigate(QAccessible::Labelled, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_border->navigate(QAccessible::Labelled, 1, &acc_target), -1);
    QVERIFY(!acc_target);

    // to label
    QCOMPARE(acc_lineedit->navigate(QAccessible::Label, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_lineedit->navigate(QAccessible::Label, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_label->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_lineedit->navigate(QAccessible::Label, 2, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_radio->navigate(QAccessible::Label, 0, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_radio->navigate(QAccessible::Label, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_groupbox->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_radio->navigate(QAccessible::Label, 2, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_lineedit2->navigate(QAccessible::Label, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_label2->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_lineedit2->navigate(QAccessible::Label, 2, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_groupbox->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_lineedit2->navigate(QAccessible::Label, 3, &acc_target), -1);
    QVERIFY(!acc_target);

    QCOMPARE(acc_grandchild->navigate(QAccessible::Label, 1, &acc_target), 0);
    QVERIFY(acc_target->object() == acc_groupbox2->object());
    delete acc_target; acc_target = 0;
    QCOMPARE(acc_grandchild->navigate(QAccessible::Label, 2, &acc_target), -1);
    QVERIFY(!acc_target);
    QCOMPARE(acc_grandchild->navigate(QAccessible::Label, 3, &acc_target), -1);
    QVERIFY(!acc_target);

    delete acc_label;
    delete acc_lineedit;
    delete acc_groupbox;
    delete acc_radio;
    delete acc_label2;
    delete acc_lineedit2;
    delete acc_groupbox2;
    delete acc_grandchild;
    delete acc_border;
    delete acc_lineedit3;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::text()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = createGUI();
    toplevel->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(toplevel);
#endif
    QObject *topLeft = toplevel->child("topLeft");
    QObject *topRight = toplevel->child("topRight");
    QObject *bottomLeft = toplevel->child("bottomLeft");

    QAccessibleInterface *acc_pb1 = QAccessible::queryAccessibleInterface(topLeft->child("pb1"));

    QAccessibleInterface *acc_pbOk = QAccessible::queryAccessibleInterface(topRight->child("pbOk"));
    QAccessibleInterface *acc_slider = QAccessible::queryAccessibleInterface(topRight->child("slider"));
    QAccessibleInterface *acc_spinBox = QAccessible::queryAccessibleInterface(topRight->child("spinBox"));
    QAccessibleInterface *acc_sliderLcd = QAccessible::queryAccessibleInterface(topRight->child("sliderLcd"));

    QAccessibleInterface *acc_label = QAccessible::queryAccessibleInterface(bottomLeft->child("label"));
    QAccessibleInterface *acc_lineedit = QAccessible::queryAccessibleInterface(bottomLeft->child("lineedit"));
    QAccessibleInterface *acc_radiogroup = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup"));
    QVERIFY(bottomLeft->child("radiogroup"));
    QAccessibleInterface *acc_radioAM = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("radioAM"));
    QAccessibleInterface *acc_frequency = QAccessible::queryAccessibleInterface(bottomLeft->child("radiogroup")->child("frequency"));

    QVERIFY(acc_pb1);

    QVERIFY(acc_pbOk);
    QVERIFY(acc_slider);
    QVERIFY(acc_spinBox);
    QVERIFY(acc_sliderLcd);

    QVERIFY(acc_label);
    QVERIFY(acc_lineedit);
    QVERIFY(acc_radiogroup);
    QVERIFY(acc_radioAM);
    QVERIFY(acc_frequency);

    QVERIFY(acc_label->relationTo(0, acc_lineedit, 0) & QAccessible::Label);
    QVERIFY(acc_radiogroup->relationTo(0, acc_frequency, 0) & QAccessible::Label);
    QVERIFY(acc_slider->relationTo(0, acc_sliderLcd, 0) & QAccessible::Controller);
    QVERIFY(acc_spinBox->relationTo(0, acc_slider, 0) & QAccessible::Controller);

    // Name
    QCOMPARE(acc_lineedit->text(QAccessible::Name, 0), acc_label->text(QAccessible::Name,0));
    QCOMPARE(acc_frequency->text(QAccessible::Name, 0), acc_radiogroup->text(QAccessible::Name,0));
    QCOMPARE(acc_sliderLcd->text(QAccessible::Name, 0), acc_slider->text(QAccessible::Value,0));
    QCOMPARE(acc_pbOk->text(QAccessible::Name, 0), QString("Ok"));
    QCOMPARE(acc_radioAM->text(QAccessible::Name, 0), QString("AM"));
    QCOMPARE(acc_pb1->text(QAccessible::Name, 0), QString("Button1"));

    // Description
    QString desc = qobject_cast<QWidget*>(acc_label->object())->toolTip();
    QVERIFY(!desc.isEmpty());
    QCOMPARE(acc_label->text(QAccessible::Description, 0), desc);
    desc = qobject_cast<QWidget*>(acc_lineedit->object())->toolTip();
    QVERIFY(!desc.isEmpty());
    QCOMPARE(acc_lineedit->text(QAccessible::Description, 0), desc);

    // Help
    QString help = qobject_cast<QWidget*>(acc_label->object())->whatsThis();
    QVERIFY(!help.isEmpty());
    QCOMPARE(acc_label->text(QAccessible::Help, 0), help);
    help = qobject_cast<QWidget*>(acc_frequency->object())->whatsThis();
    QVERIFY(!help.isEmpty());
    QCOMPARE(acc_frequency->text(QAccessible::Help, 0), help);

    // Value
    QString value = acc_frequency->object()->property("text").toString();
    QVERIFY(!value.isEmpty());
    QCOMPARE(acc_frequency->text(QAccessible::Value, 0), value);
    value = acc_slider->object()->property("value").toString();
    QVERIFY(!value.isEmpty());
    QCOMPARE(acc_slider->text(QAccessible::Value, 0), value);
    QCOMPARE(acc_spinBox->text(QAccessible::Value, 0), value);

    // Accelerator
    QCOMPARE(acc_pbOk->text(QAccessible::Accelerator, 0), Q3Accel::keyToString(Qt::Key_Enter));
    QCOMPARE(acc_pb1->text(QAccessible::Accelerator, 0), Q3Accel::keyToString(Qt::ALT + Qt::Key_1));
    QCOMPARE(acc_lineedit->text(QAccessible::Accelerator, 0), Q3Accel::keyToString(Qt::ALT) + "L");
    QCOMPARE(acc_frequency->text(QAccessible::Accelerator, 0), Q3Accel::keyToString(Qt::ALT) + "C");

    delete acc_pb1;
    delete acc_pbOk;
    delete acc_slider;
    delete acc_spinBox;
    delete acc_sliderLcd;

    delete acc_label;
    delete acc_lineedit;
    delete acc_radiogroup;
        delete acc_radioAM;
        delete acc_frequency;

    delete toplevel;
    QTestAccessibility::clearEvents();

#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::setText()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget *toplevel = createGUI();
    toplevel->show();
    QObject *bottomLeft = toplevel->child("bottomLeft");

    QAccessibleInterface *acc_lineedit = QAccessible::queryAccessibleInterface(bottomLeft->child("lineedit"));
    // Value, read-write
    QString txt = acc_lineedit->text(QAccessible::Value, 0);
    QVERIFY(txt.isEmpty());
    txt = QLatin1String("Writable");
    acc_lineedit->setText(QAccessible::Value, 0, txt);
    QCOMPARE(acc_lineedit->text(QAccessible::Value, 0), txt);

    // Description, read-only
    txt = acc_lineedit->text(QAccessible::Description, 0);
    QVERIFY(!txt.isEmpty());
    acc_lineedit->setText(QAccessible::Description, 0, QLatin1String(""));
    QCOMPARE(acc_lineedit->text(QAccessible::Description, 0), txt);

    QVERIFY(acc_lineedit);

    delete acc_lineedit;
    delete toplevel;
    QTestAccessibility::clearEvents();

#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::hideShowTest()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget * const window = new QWidget();
    QWidget * const child = new QWidget(window);

    QVERIFY(state(window) & QAccessible::Invisible);
    QVERIFY(state(child)  & QAccessible::Invisible);

    QTestAccessibility::clearEvents();

    // show() and veryfy that both window and child are not invisible and get ObjectShow events.
    window->show();
    QVERIFY(state(window) ^ QAccessible::Invisible);
    QVERIFY(state(child)  ^ QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(window, 0, QAccessible::ObjectShow)));
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(child, 0, QAccessible::ObjectShow)));
    QTestAccessibility::clearEvents();

    // hide() and veryfy that both window and child are invisible and get ObjectHide events.
    window->hide();
    QVERIFY(state(window) & QAccessible::Invisible);
    QVERIFY(state(child)  & QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(window, 0, QAccessible::ObjectHide)));
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(child, 0, QAccessible::ObjectHide)));
    QTestAccessibility::clearEvents();

    delete window;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::userActionCount()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget widget;

    QAccessibleInterface *test = QAccessible::queryAccessibleInterface(&widget);
    QVERIFY(test);
    QVERIFY(test->isValid());
    QCOMPARE(test->userActionCount(0), 0);
    QCOMPARE(test->userActionCount(1), 0);
    QCOMPARE(test->userActionCount(-1), 0);
    delete test; test = 0;

    QFrame frame;

    test = QAccessible::queryAccessibleInterface(&frame);
    QVERIFY(test);
    QVERIFY(test->isValid());
    QCOMPARE(test->userActionCount(0), 0);
    QCOMPARE(test->userActionCount(1), 0);
    QCOMPARE(test->userActionCount(-1), 0);
    delete test; test = 0;

    QLineEdit lineEdit;

    test = QAccessible::queryAccessibleInterface(&lineEdit);
    QVERIFY(test);
    QVERIFY(test->isValid());
    QCOMPARE(test->userActionCount(0), 0);
    QCOMPARE(test->userActionCount(1), 0);
    QCOMPARE(test->userActionCount(-1), 0);
    delete test; test = 0;
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::actionText()
{
#ifdef QTEST_ACCESSIBILITY
    QWidget widget;
    widget.show();

    QAccessibleInterface *test = QAccessible::queryAccessibleInterface(&widget);
    QVERIFY(test);
    QVERIFY(test->isValid());

    QCOMPARE(test->actionText(1, QAccessible::Name, 0), QString());
    QCOMPARE(test->actionText(0, QAccessible::Name, 1), QString());
    QCOMPARE(test->actionText(1, QAccessible::Name, 1), QString());
    QCOMPARE(test->actionText(QAccessible::SetFocus, QAccessible::Name, -1), QString());

    QCOMPARE(test->actionText(QAccessible::DefaultAction, QAccessible::Name, 0), QString("SetFocus"));
    QCOMPARE(test->actionText(QAccessible::SetFocus, QAccessible::Name, 0), QString("SetFocus"));

    delete test; test = 0;

#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::doAction()
{
#ifdef QTEST_ACCESSIBILITY
    QSKIP("TODO: Implement me", SkipAll);
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::buttonTest()
{
//#ifdef QTEST_ACCESSIBILITY
#if 0
    QAccessibleInterface *test = 0;
    Q3VBox vbox;

    // Standard push button
    QPushButton pushButton("Ok", &vbox);

    // toggle push button
    QPushButton togglepush("Toggle", &vbox);
    togglepush.setToggleButton(TRUE);

    // push button with a menu
    QPushButton menuButton("Menu", &vbox);
    Q3PopupMenu buttonMenu(&menuButton);
    buttonMenu.insertItem("Some item");
    menuButton.setPopup(&buttonMenu);

    // standard checkbox
    QCheckBox checkBox("Check me!", &vbox);

    // tristate checkbox
    QCheckBox tristate("Tristate!", &vbox);
    tristate.setTristate(TRUE);

    // radiobutton
    QRadioButton radio("Radio me!", &vbox);

    // standard toolbutton
    QToolButton toolbutton(&vbox);
    toolbutton.setText("Tool");
    toolbutton.setMinimumSize(20,20);

    // standard toolbutton
    QToolButton toggletool(&vbox);
    toggletool.setToggleButton(TRUE);
    toggletool.setText("Toggle");
    toggletool.setMinimumSize(20,20);

    // menu toolbutton
    QToolButton menuToolButton(&vbox);
    menuToolButton.setText("Menu Tool");
    Q3PopupMenu toolMenu(&menuToolButton);
    toolMenu.insertItem("Some item");
    menuToolButton.setPopup(&toolMenu);
    menuToolButton.setMinimumSize(20,20);

    // splitted menu toolbutton
    QToolButton splitToolButton(&vbox);
    splitToolButton.setTextLabel("Split Tool");
    Q3PopupMenu splitMenu(&splitToolButton);
    splitMenu.insertItem("Some item");
    splitToolButton.setPopup(&splitMenu);
    splitToolButton.setPopupDelay(0);
    splitToolButton.setMinimumSize(20,20);

    // test push button
    QVERIFY(QAccessible::queryAccessibleInterface(&pushButton, &test));
    QCOMPARE(test->role(0), QAccessible::PushButton);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Press"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    pushButton.setDown(TRUE);
    QCOMPARE(test->state(0), (int)QAccessible::Pressed);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    test->release();

    // test toggle push button
    QVERIFY(QAccessible::queryAccessibleInterface(&togglepush, &test));
    QCOMPARE(test->role(0), QAccessible::CheckBox);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Uncheck"));
    QCOMPARE(test->state(0), (int)QAccessible::Checked);
    test->release();

    // test menu push button
    QVERIFY(QAccessible::queryAccessibleInterface(&menuButton, &test));
    QCOMPARE(test->role(0), QAccessible::ButtonMenu);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Open"));
    QCOMPARE(test->state(0), (int)QAccessible::HasPopup);
    test->release();

    // test check box
    QVERIFY(QAccessible::queryAccessibleInterface(&checkBox, &test));
    QCOMPARE(test->role(0), QAccessible::CheckBox);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Uncheck"));
    QCOMPARE(test->state(0), (int)QAccessible::Checked);
    test->release();

    // test tristate check box
    QVERIFY(QAccessible::queryAccessibleInterface(&tristate, &test));
    QCOMPARE(test->role(0), QAccessible::CheckBox);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Toggle"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Mixed);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Uncheck"));
    QCOMPARE(test->state(0), (int)QAccessible::Checked);
    test->release();

    // test radiobutton
    QVERIFY(QAccessible::queryAccessibleInterface(&radio, &test));
    QCOMPARE(test->role(0), QAccessible::RadioButton);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Checked);
    test->release();

    // test standard toolbutton
    QVERIFY(QAccessible::queryAccessibleInterface(&toolbutton, &test));
    QCOMPARE(test->role(0), QAccessible::PushButton);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Press"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    test->release();

    // toggle tool button
    QVERIFY(QAccessible::queryAccessibleInterface(&toggletool, &test));
    QCOMPARE(test->role(0), QAccessible::CheckBox);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Check"));
    QCOMPARE(test->state(0), (int)QAccessible::Normal);
    QVERIFY(test->doAction(QAccessible::Press, 0));
    QTest::qWait(500);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Uncheck"));
    QCOMPARE(test->state(0), (int)QAccessible::Checked);
    test->release();

    // test menu toolbutton
    QVERIFY(QAccessible::queryAccessibleInterface(&menuToolButton, &test));
    QCOMPARE(test->role(0), QAccessible::ButtonMenu);
    QCOMPARE(test->defaultAction(0), 1);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Open"));
    QCOMPARE(test->state(0), (int)QAccessible::HasPopup);
    QCOMPARE(test->actionCount(0), 1);
    QCOMPARE(test->actionText(QAccessible::Press, QAccessible::Name, 0), QString("Press"));
    test->release();

    // test splitted menu toolbutton
    QVERIFY(QAccessible::queryAccessibleInterface(&splitToolButton, &test));
    QCOMPARE(test->childCount(), 2);
    QCOMPARE(test->role(0), QAccessible::ButtonDropDown);
    QCOMPARE(test->role(1), QAccessible::PushButton);
    QCOMPARE(test->role(2), QAccessible::ButtonMenu);
    QCOMPARE(test->defaultAction(0), QAccessible::Press);
    QCOMPARE(test->defaultAction(1), QAccessible::Press);
    QCOMPARE(test->defaultAction(2), QAccessible::Press);
    QCOMPARE(test->actionText(test->defaultAction(0), QAccessible::Name, 0), QString("Press"));
    QCOMPARE(test->state(0), (int)QAccessible::HasPopup);
    QCOMPARE(test->actionCount(0), 1);
    QCOMPARE(test->actionText(1, QAccessible::Name, 0), QString("Open"));
    QCOMPARE(test->actionText(test->defaultAction(1), QAccessible::Name, 1), QString("Press"));
    QCOMPARE(test->state(1), (int)QAccessible::Normal);
    QCOMPARE(test->actionText(test->defaultAction(2), QAccessible::Name, 2), QString("Open"));
    QCOMPARE(test->state(2), (int)QAccessible::HasPopup);
    test->release();

    QTestAccessibility::clearEvents();

#else
//    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
    QSKIP("No action interface in Qt 4 yet.", SkipAll);
#endif
}

void tst_QAccessibility::sliderTest()
{
#ifdef QTEST_ACCESSIBILITY
    QAccessibleInterface *test = 0;
    Q3VBox vbox;
    QLabel labelHorizontal("Horizontal", &vbox);
    QSlider sliderHorizontal(Qt::Horizontal, &vbox);
    labelHorizontal.setBuddy(&sliderHorizontal);

    QLabel labelVertical("Vertical", &vbox);
    QSlider sliderVertical(Qt::Vertical, &vbox);
    labelVertical.setBuddy(&sliderVertical);
    vbox.show();

    // test horizontal slider
    test = QAccessible::queryAccessibleInterface(&sliderHorizontal);
    QVERIFY(test);
    QCOMPARE(test->childCount(), 3);
    QCOMPARE(test->role(0), QAccessible::Slider);
    QCOMPARE(test->role(1), QAccessible::PushButton);
    QCOMPARE(test->role(2), QAccessible::Indicator);
    QCOMPARE(test->role(3), QAccessible::PushButton);

    QCOMPARE(test->text(QAccessible::Name, 0), labelHorizontal.text());
    QCOMPARE(test->text(QAccessible::Name, 1), QSlider::tr("Page left"));
    QCOMPARE(test->text(QAccessible::Name, 2), QSlider::tr("Position"));
    QCOMPARE(test->text(QAccessible::Name, 3), QSlider::tr("Page right"));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderHorizontal.value()));
    QCOMPARE(test->text(QAccessible::Value, 1), QString());
    QCOMPARE(test->text(QAccessible::Value, 2), QString::number(sliderHorizontal.value()));
    QCOMPARE(test->text(QAccessible::Value, 3), QString());
// Skip acton tests.
#if 0
    QCOMPARE(test->defaultAction(0), QAccessible::SetFocus);
    QCOMPARE(test->defaultAction(1), QAccessible::Press);
    QCOMPARE(test->defaultAction(2), QAccessible::NoAction);
    QCOMPARE(test->defaultAction(3), QAccessible::Press);
    QCOMPARE(test->actionText(QAccessible::SetFocus, QAccessible::Name, 0), QSlider::tr("Set Focus"));
    QCOMPARE(test->actionText(QAccessible::Press, QAccessible::Name, 1), QSlider::tr("Press"));
    QCOMPARE(test->actionText(QAccessible::Increase, QAccessible::Name, 2), QSlider::tr("Increase"));
    QCOMPARE(test->actionText(QAccessible::Decrease, QAccessible::Name, 2), QSlider::tr("Decrease"));
    QCOMPARE(test->actionText(QAccessible::Press, QAccessible::Name, 3), QSlider::tr("Press"));
    QVERIFY(test->doAction(QAccessible::Press, 3));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderHorizontal.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 3));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(2*sliderHorizontal.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 1));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderHorizontal.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 1));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(0));
    QVERIFY(test->doAction(QAccessible::Increase, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderHorizontal.lineStep()));
    QVERIFY(test->doAction(QAccessible::Increase, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(2*sliderHorizontal.lineStep()));
    QVERIFY(test->doAction(QAccessible::Decrease, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderHorizontal.lineStep()));
    QVERIFY(test->doAction(QAccessible::Decrease, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(0));
#endif
    delete test;

    // test vertical slider
    test = QAccessible::queryAccessibleInterface(&sliderVertical);
    QVERIFY(test);
    QCOMPARE(test->childCount(), 3);
    QCOMPARE(test->role(0), QAccessible::Slider);
    QCOMPARE(test->role(1), QAccessible::PushButton);
    QCOMPARE(test->role(2), QAccessible::Indicator);
    QCOMPARE(test->role(3), QAccessible::PushButton);

    QCOMPARE(test->text(QAccessible::Name, 0), labelVertical.text());
    QCOMPARE(test->text(QAccessible::Name, 1), QSlider::tr("Page up"));
    QCOMPARE(test->text(QAccessible::Name, 2), QSlider::tr("Position"));
    QCOMPARE(test->text(QAccessible::Name, 3), QSlider::tr("Page down"));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderVertical.value()));
    QCOMPARE(test->text(QAccessible::Value, 1), QString());
    QCOMPARE(test->text(QAccessible::Value, 2), QString::number(sliderVertical.value()));
    QCOMPARE(test->text(QAccessible::Value, 3), QString());
// Skip acton tests.
#if 0
    QCOMPARE(test->defaultAction(0), QAccessible::SetFocus);
    QCOMPARE(test->defaultAction(1), QAccessible::Press);
    QCOMPARE(test->defaultAction(2), QAccessible::NoAction);
    QCOMPARE(test->defaultAction(3), QAccessible::Press);
    QCOMPARE(test->actionText(QAccessible::SetFocus, QAccessible::Name, 0), QSlider::tr("Set Focus"));
    QCOMPARE(test->actionText(QAccessible::Press, QAccessible::Name, 1), QSlider::tr("Press"));
    QCOMPARE(test->actionText(QAccessible::Increase, QAccessible::Name, 2), QSlider::tr("Increase"));
    QCOMPARE(test->actionText(QAccessible::Decrease, QAccessible::Name, 2), QSlider::tr("Decrease"));
    QCOMPARE(test->actionText(QAccessible::Press, QAccessible::Name, 3), QSlider::tr("Press"));
    QVERIFY(test->doAction(QAccessible::Press, 3));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderVertical.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 3));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(2*sliderVertical.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 1));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderVertical.pageStep()));
    QVERIFY(test->doAction(QAccessible::Press, 1));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(0));
    QVERIFY(test->doAction(QAccessible::Increase, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderVertical.lineStep()));
    QVERIFY(test->doAction(QAccessible::Increase, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(2*sliderVertical.lineStep()));
    QVERIFY(test->doAction(QAccessible::Decrease, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(sliderVertical.lineStep()));
    QVERIFY(test->doAction(QAccessible::Decrease, 2));
    QCOMPARE(test->text(QAccessible::Value, 0), QString::number(0));
#endif
    delete test;

    // Test that when we hide() a slider, the PageLeft, Indicator, and PageRight also gets the
    // Invisible state bit set.
    enum SubControls { PageLeft = 1, Position = 2, PageRight = 3 };

    QSlider *slider  = new QSlider();
    QAccessibleInterface * const sliderInterface = QAccessible::queryAccessibleInterface(slider);
    QVERIFY(sliderInterface);

    QVERIFY(sliderInterface->state(0)         & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageLeft)  & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(Position)  & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageRight) & QAccessible::Invisible);

    slider->show();
    QVERIFY(sliderInterface->state(0)         ^ QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageLeft)  ^ QAccessible::Invisible);
    QVERIFY(sliderInterface->state(Position)  ^ QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageRight) ^ QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(slider, 0, QAccessible::ObjectShow)));
    QTestAccessibility::clearEvents();

    slider->hide();
    QVERIFY(sliderInterface->state(0)         & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageLeft)  & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(Position)  & QAccessible::Invisible);
    QVERIFY(sliderInterface->state(PageRight) & QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(slider, 0, QAccessible::ObjectHide)));
    QTestAccessibility::clearEvents();

    // Test that the left/right subcontrols are set to unavailable when the slider is at the minimum/maximum.
    slider->show();
    slider->setMinimum(0);
    slider->setMaximum(100);

    slider->setValue(50);
    QVERIFY(sliderInterface->state(PageLeft)  ^ QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(Position)  ^ QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(PageRight) ^ QAccessible::Unavailable);

    slider->setValue(0);
    QVERIFY(sliderInterface->state(PageLeft)  & QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(Position)  ^ QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(PageRight) ^ QAccessible::Unavailable);

    slider->setValue(100);
    QVERIFY(sliderInterface->state(PageLeft)  ^ QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(Position)  ^ QAccessible::Unavailable);
    QVERIFY(sliderInterface->state(PageRight) & QAccessible::Unavailable);

    delete sliderInterface;
    delete slider;

    // Test that the rects are ok.
    {
        QSlider *slider  = new QSlider(Qt::Horizontal);
        slider->show();
#if defined(Q_WS_X11)
        qt_x11_wait_for_window_manager(slider);
#endif
        QAccessibleInterface * const sliderInterface = QAccessible::queryAccessibleInterface(slider);
        QVERIFY(sliderInterface);

        slider->setMinimum(0);
        slider->setMaximum(100);
        slider->setValue(50);

        const QRect sliderRect = sliderInterface->rect(0);
        QVERIFY(sliderRect.isValid());

        // Verify that the sub-control rects are valid and inside the slider rect.
        for (int i = PageLeft; i <= PageRight; ++i) {
            const QRect testRect = sliderInterface->rect(i);
            QVERIFY(testRect.isValid());
            QVERIFY(sliderRect.contains(testRect));
        }

        delete slider;
        delete sliderInterface;
    }


    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::scrollBarTest()
{
#ifdef QTEST_ACCESSIBILITY
    // Test that when we hide() a slider, the PageLeft, Indicator, and PageRight also gets the
    // Invisible state bit set.
    enum SubControls { LineUp = 1,
        PageUp = 2,
        Position = 3,
        PageDown = 4,
        LineDown = 5
 };

    QScrollBar *scrollBar  = new QScrollBar();
    QAccessibleInterface * const scrollBarInterface = QAccessible::queryAccessibleInterface(scrollBar);
    QVERIFY(scrollBarInterface);

    QVERIFY(scrollBarInterface->state(0)         & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageUp)    & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(Position)  & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageDown)  & QAccessible::Invisible);

    scrollBar->show();
    QVERIFY(scrollBarInterface->state(0)         ^ QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageUp)    ^ QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(Position)  ^ QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageDown)  ^ QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(scrollBar, 0, QAccessible::ObjectShow)));
    QTestAccessibility::clearEvents();

    scrollBar->hide();
    QVERIFY(scrollBarInterface->state(0)         & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageUp)    & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(Position)  & QAccessible::Invisible);
    QVERIFY(scrollBarInterface->state(PageDown)  & QAccessible::Invisible);
    QVERIFY(QTestAccessibility::events().contains(QTestAccessibilityEvent(scrollBar, 0, QAccessible::ObjectHide)));
    QTestAccessibility::clearEvents();

    // Test that the left/right subcontrols are set to unavailable when the scrollBar is at the minimum/maximum.
    scrollBar->show();
    scrollBar->setMinimum(0);
    scrollBar->setMaximum(100);

    scrollBar->setValue(50);
    QVERIFY(scrollBarInterface->state(PageUp)    ^ QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(Position)  ^ QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(PageDown)  ^ QAccessible::Unavailable);

    scrollBar->setValue(0);
    QVERIFY(scrollBarInterface->state(PageUp)    & QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(Position)  ^ QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(PageDown)  ^ QAccessible::Unavailable);

    scrollBar->setValue(100);
    QVERIFY(scrollBarInterface->state(PageUp)   ^ QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(Position) ^ QAccessible::Unavailable);
    QVERIFY(scrollBarInterface->state(PageDown) & QAccessible::Unavailable);

    delete scrollBarInterface;
    delete scrollBar;

    // Test that the rects are ok.
    {
        QScrollBar *scrollBar  = new QScrollBar(Qt::Horizontal);
        scrollBar->resize(100, 50);
        scrollBar->show();
#if defined(Q_WS_X11)
        qt_x11_wait_for_window_manager(scrollBar);
#endif
        QAccessibleInterface * const scrollBarInterface = QAccessible::queryAccessibleInterface(scrollBar);
        QVERIFY(scrollBarInterface);

        scrollBar->setMinimum(0);
        scrollBar->setMaximum(100);
        scrollBar->setValue(50);

        QApplication::processEvents();

        const QRect scrollBarRect = scrollBarInterface->rect(0);
        QVERIFY(scrollBarRect.isValid());

        // Verify that the sub-control rects are valid and inside the scrollBar rect.
        for (int i = LineUp; i <= LineDown; ++i) {
            const QRect testRect = scrollBarInterface->rect(i);
            QVERIFY(testRect.isValid());
            QVERIFY(scrollBarRect.contains(testRect));
        }
        delete scrollBarInterface;
        delete scrollBar;
    }

    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif

}

void tst_QAccessibility::tabTest()
{
#ifdef QTEST_ACCESSIBILITY
    QTabBar *tabBar = new QTabBar();
    tabBar->show();

    QAccessibleInterface * const interface = QAccessible::queryAccessibleInterface(tabBar);
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), 2);
    interface->doAction(QAccessible::Press, 1);
    interface->doAction(QAccessible::Press, 2);

    // Test that the Invisible bit for the navigation buttons gets set
    // and cleared correctly.
    QVERIFY(interface->state(1) & QAccessible::Invisible);

    const int lots = 10;
    for (int i = 0; i < lots; ++i)
        tabBar->addTab("Foo");

    QVERIFY((interface->state(1) & QAccessible::Invisible) == false);
    tabBar->hide();
    QVERIFY(interface->state(1) & QAccessible::Invisible);

    tabBar->show();
    tabBar->setCurrentIndex(0);

    // Test that sending a focus action to a tab does not select it.
    interface->doAction(QAccessible::Focus, 2, QVariantList());
    QCOMPARE(tabBar->currentIndex(), 0);

    // Test that sending a press action to a tab selects it.
    interface->doAction(QAccessible::Press, 2, QVariantList());
    QCOMPARE(tabBar->currentIndex(), 1);

    delete tabBar;
    delete interface;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::menuTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QMainWindow mw;
    QMenu *file = mw.menuBar()->addMenu("&File");
    QMenu *fileNew = file->addMenu("&New...");
    fileNew->menuAction()->setShortcut(tr("Ctrl+N"));
    fileNew->addAction("Text file");
    fileNew->addAction("Image file");
    file->addAction("&Open")->setShortcut(tr("Ctrl+O"));
    file->addAction("&Save")->setShortcut(tr("Ctrl+S"));
    file->addSeparator();
    file->addAction("E&xit")->setShortcut(tr("Alt+F4"));

    QMenu *edit = mw.menuBar()->addMenu("&Edit");
    edit->addAction("&Undo")->setShortcut(tr("Ctrl+Z"));
    edit->addAction("&Redo")->setShortcut(tr("Ctrl+Y"));
    edit->addSeparator();
    edit->addAction("Cu&t")->setShortcut(tr("Ctrl+X"));
    edit->addAction("&Copy")->setShortcut(tr("Ctrl+C"));
    edit->addAction("&Paste")->setShortcut(tr("Ctrl+V"));
    edit->addAction("&Delete")->setShortcut(tr("Del"));
    edit->addSeparator();
    edit->addAction("Pr&operties");

    mw.menuBar()->addSeparator();

    QMenu *help = mw.menuBar()->addMenu("&Help");
    help->addAction("&Contents");
    help->addAction("&About");

    mw.menuBar()->addAction("Action!");

    mw.show(); // triggers layout
    QTest::qWait(100);

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(mw.menuBar());
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), 5);
    QCOMPARE(interface->role(0), QAccessible::MenuBar);
    QCOMPARE(interface->role(1), QAccessible::MenuItem);
    QCOMPARE(interface->role(2), QAccessible::MenuItem);
    QCOMPARE(interface->role(3), QAccessible::Separator);
    QCOMPARE(interface->role(4), QAccessible::MenuItem);
    QCOMPARE(interface->role(5), QAccessible::MenuItem);
    QCOMPARE(mw.mapFromGlobal(interface->rect(0).topLeft()), mw.menuBar()->geometry().topLeft());
    QCOMPARE(interface->rect(0).size(), mw.menuBar()->size());

// Not working at the moment, task to fix is #100019.
#if 0
    QVERIFY(interface->rect(0).contains(interface->rect(1)));
    QVERIFY(interface->rect(0).contains(interface->rect(2)));
    // QVERIFY(interface->rect(0).contains(interface->rect(3))); //separator might be invisible
    QVERIFY(interface->rect(0).contains(interface->rect(4)));
    QVERIFY(interface->rect(0).contains(interface->rect(5)));
#endif

    QCOMPARE(interface->text(QAccessible::Name, 1), QString("File"));
    QCOMPARE(interface->text(QAccessible::Name, 2), QString("Edit"));
    QCOMPARE(interface->text(QAccessible::Name, 3), QString());
    QCOMPARE(interface->text(QAccessible::Name, 4), QString("Help"));
    QCOMPARE(interface->text(QAccessible::Name, 5), QString("Action!"));

// Not working at the moment, task to fix is #100019.
#ifndef Q_OS_MAC
    QCOMPARE(interface->text(QAccessible::Accelerator, 1), tr("Alt+F"));
    QCOMPARE(interface->text(QAccessible::Accelerator, 2), tr("Alt+E"));
    QCOMPARE(interface->text(QAccessible::Accelerator, 4), tr("Alt+H"));
    QCOMPARE(interface->text(QAccessible::Accelerator, 3), QString());
    QCOMPARE(interface->text(QAccessible::Accelerator, 4), tr("Alt+H"));
    QCOMPARE(interface->text(QAccessible::Accelerator, 5), QString());
#endif

    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 1), QString("Open"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 2), QString("Open"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 3), QString());
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 4), QString("Open"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 5), QString("Execute"));

    bool menuFade = qApp->isEffectEnabled(Qt::UI_FadeMenu);
    int menuFadeDelay = 300;
    interface->doAction(QAccessible::DefaultAction, 1);
    if(menuFade)
        QTest::qWait(menuFadeDelay);
    QVERIFY(file->isVisible() && !edit->isVisible() && !help->isVisible());
    interface->doAction(QAccessible::DefaultAction, 2);
    if(menuFade)
        QTest::qWait(menuFadeDelay);
    QVERIFY(!file->isVisible() && edit->isVisible() && !help->isVisible());
    interface->doAction(QAccessible::DefaultAction, 3);
    if(menuFade)
        QTest::qWait(menuFadeDelay);
    QVERIFY(!file->isVisible() && !edit->isVisible() && !help->isVisible());
    interface->doAction(QAccessible::DefaultAction, 4);
    if(menuFade)
        QTest::qWait(menuFadeDelay);
    QVERIFY(!file->isVisible() && !edit->isVisible() && help->isVisible());
    interface->doAction(QAccessible::DefaultAction, 5);
    if(menuFade)
        QTest::qWait(menuFadeDelay);
    QVERIFY(!file->isVisible() && !edit->isVisible() && !help->isVisible());

    interface->doAction(QAccessible::DefaultAction, 1);
    delete interface;
    interface = QAccessible::queryAccessibleInterface(file);
    QCOMPARE(interface->childCount(), 5);
    QCOMPARE(interface->role(0), QAccessible::PopupMenu);
    QCOMPARE(interface->role(1), QAccessible::MenuItem);
    QCOMPARE(interface->role(2), QAccessible::MenuItem);
    QCOMPARE(interface->role(3), QAccessible::MenuItem);
    QCOMPARE(interface->role(4), QAccessible::Separator);
    QCOMPARE(interface->role(5), QAccessible::MenuItem);
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 1), QString("Open"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 2), QString("Execute"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 3), QString("Execute"));
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 4), QString());
    QCOMPARE(interface->actionText(QAccessible::DefaultAction, QAccessible::Name, 5), QString("Execute"));

#if QT_VERSION < 0x040102
    QEXPECT_FAIL("", "Submenus don't open, task 99301", Continue);
#endif
    interface->doAction(QAccessible::DefaultAction, 1);
    QTestEventLoop::instance().enterLoop(1);

#if defined (Q_OS_WIN) && QT_VERSION < 0x040300
    QEXPECT_FAIL("", "Don't expect the File menu to be visible in 4.2", Continue);
#endif
    QVERIFY(file->isVisible());
    QVERIFY(fileNew->isVisible());
    QVERIFY(!edit->isVisible());
    QVERIFY(!help->isVisible());

    QTestAccessibility::clearEvents();
    mw.hide();
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::spinBoxTest()
{
#ifdef QTEST_ACCESSIBILITY
    QSpinBox * const spinBox = new QSpinBox();
    spinBox->show();

    QAccessibleInterface * const interface = QAccessible::queryAccessibleInterface(spinBox);
    QVERIFY(interface);

    const QRect widgetRect = spinBox->geometry();
    const QRect accessibleRect = interface->rect(0);
    QCOMPARE(accessibleRect, widgetRect);

    // Test that we get valid rects for all the spinbox child interfaces.
    const int numChildren = interface->childCount();
    for (int i = 1; i <= numChildren; ++i) {
        const QRect childRect = interface->rect(i);
        QVERIFY(childRect.isNull() == false);
    }

    delete spinBox;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::doubleSpinBoxTest()
{
#ifdef QTEST_ACCESSIBILITY
    QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->show();

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(doubleSpinBox);
    QVERIFY(interface);

    const QRect widgetRect = doubleSpinBox->geometry();
    const QRect accessibleRect = interface->rect(0);
    QCOMPARE(accessibleRect, widgetRect);

    // Test that we get valid rects for all the spinbox child interfaces.
    const int numChildren = interface->childCount();
    for (int i = 1; i <= numChildren; ++i) {
        const QRect childRect = interface->rect(i);
        QVERIFY(childRect.isValid());
    }

    delete doubleSpinBox;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::textEditTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QTextEdit edit;
    QString text = "hello world\nhow are you today?\n";
    edit.setText(text);
    edit.show();

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(&edit);
    QCOMPARE(iface->text(QAccessible::Value, 0), text);
    QCOMPARE(iface->childCount(), 4);
    QCOMPARE(iface->text(QAccessible::Value, 2), QString("hello world"));
    QCOMPARE(iface->text(QAccessible::Value, 3), QString("how are you today?"));
    QCOMPARE(iface->text(QAccessible::Value, 4), QString());
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::textBrowserTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QTextBrowser textBrowser;
    QString text = QLatin1String("Hello world\nhow are you today?\n");
    textBrowser.setText(text);
    textBrowser.show();

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&textBrowser);
    QVERIFY(interface);
    QCOMPARE(interface->role(0), QAccessible::StaticText);
    QCOMPARE(interface->text(QAccessible::Value, 0), text);
    QCOMPARE(interface->childCount(), 4);
    QCOMPARE(interface->text(QAccessible::Value, 2), QString("Hello world"));
    QCOMPARE(interface->text(QAccessible::Value, 3), QString("how are you today?"));
    QCOMPARE(interface->text(QAccessible::Value, 4), QString());
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::listViewTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
        QListView listView;
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(&listView);
        QVERIFY(iface);
        QCOMPARE(iface->childCount(), 0);
        delete iface;
    }
    {
    QListWidget listView;
    listView.addItem(tr("A"));
    listView.addItem(tr("B"));
    listView.addItem(tr("C"));
    listView.resize(400,400);
    listView.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(w);
#endif

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(&listView);
    QCOMPARE((int)iface->role(0), (int)QAccessible::Client);
    QCOMPARE((int)iface->role(1), (int)QAccessible::List);
    QCOMPARE(iface->childCount(), 1);
    QAccessibleInterface *child;
    iface->navigate(QAccessible::Child, 1, &child);
    delete iface;
    iface = child;
    QCOMPARE(iface->text(QAccessible::Value, 1), QString("A"));
    QCOMPARE(iface->text(QAccessible::Value, 2), QString("B"));
    QCOMPARE(iface->text(QAccessible::Value, 3), QString("C"));
    
    QAccessibleInterface *childA = 0;
    QCOMPARE(iface->navigate(QAccessible::Child, 1, &childA), 0);
    QVERIFY(childA);
    QCOMPARE(childA->text(QAccessible::Value, 1), QString("A"));
    delete childA;

    QAccessibleInterface *childB = 0;
    QCOMPARE(iface->navigate(QAccessible::Child, 2, &childB), 0);
    QVERIFY(childB);
    QCOMPARE(childB->text(QAccessible::Value, 1), QString("B"));
    delete childB;

    QAccessibleInterface *childC = 0;
    QCOMPARE(iface->navigate(QAccessible::Child, 3, &childC), 0);
    QVERIFY(childC);
    QCOMPARE(childC->text(QAccessible::Value, 1), QString("C"));
    delete childC;

    delete iface;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::mdiAreaTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QMdiArea mdiArea;
    mdiArea.show();
    const int subWindowCount =  5;
    for (int i = 0; i < subWindowCount; ++i)
        mdiArea.addSubWindow(new QWidget)->show();

    QList<QMdiSubWindow *> subWindows = mdiArea.subWindowList();
    QCOMPARE(subWindows.count(), subWindowCount);

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&mdiArea);
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), subWindowCount);

    // Right, right, right, ...
    for (int i = 0; i < subWindowCount; ++i) {
        QAccessibleInterface *destination = 0;
        int index = interface->navigate(QAccessible::Right, i + 1, &destination);
        if (i == subWindowCount - 1) {
            QVERIFY(!destination);
            QCOMPARE(index, -1);
        } else {
            QVERIFY(destination);
            QCOMPARE(index, i + 2);
            QCOMPARE(destination->object(), subWindows.at(i + 1));
            delete destination;
        }
    }

    // Left, left, left, ...
    for (int i = subWindowCount; i > 0; --i) {
        QAccessibleInterface *destination = 0;
        int index = interface->navigate(QAccessible::Left, i, &destination);
        if (i == 1) {
            QVERIFY(!destination);
            QCOMPARE(index, -1);
        } else {
            QVERIFY(destination);
            QCOMPARE(index, i - 1);
            QCOMPARE(destination->object(), subWindows.at(i - 2));
            delete destination;
        }
    }
    // ### Add test for Up and Down.
    
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::mdiSubWindowTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QMdiArea mdiArea;
    mdiArea.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&mdiArea);
#endif
    const int subWindowCount =  5;
    for (int i = 0; i < subWindowCount; ++i)
        mdiArea.addSubWindow(new QPushButton("QAccessibilityTest"))->show();

    QList<QMdiSubWindow *> subWindows = mdiArea.subWindowList();
    QCOMPARE(subWindows.count(), subWindowCount);

    QMdiSubWindow *testWindow = subWindows.at(3);
    QVERIFY(testWindow);
    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(testWindow);

    // childCount
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), 1);

    // setText / text
    QCOMPARE(interface->text(QAccessible::Name, 0), QString());
    QCOMPARE(interface->text(QAccessible::Name, 1), QString());
    testWindow->setWindowTitle(QLatin1String("ReplaceMe"));
    QCOMPARE(interface->text(QAccessible::Name, 0), QLatin1String("ReplaceMe"));
    QCOMPARE(interface->text(QAccessible::Name, 1), QLatin1String("ReplaceMe"));
    interface->setText(QAccessible::Name, 0, QLatin1String("TitleSetOnWindow"));
    QCOMPARE(interface->text(QAccessible::Name, 0), QLatin1String("TitleSetOnWindow"));
    interface->setText(QAccessible::Name, 1, QLatin1String("TitleSetOnChild"));
    QCOMPARE(interface->text(QAccessible::Name, 0), QLatin1String("TitleSetOnChild"));

    // state
    QAccessible::State state = QAccessible::Normal | QAccessible::Focusable
                               | QAccessible::Movable | QAccessible::Sizeable;
    QCOMPARE(interface->state(0), state);
    const QRect originalGeometry = testWindow->geometry();
    testWindow->showMaximized();
    state &= ~QAccessible::Sizeable;
    state &= ~QAccessible::Moveable;
    QCOMPARE(interface->state(0), state);
    testWindow->showNormal();
    testWindow->move(-10, 0);
    QVERIFY(interface->state(0) & QAccessible::Offscreen);
    testWindow->setVisible(false);
    QVERIFY(interface->state(0) & QAccessible::Invisible);
    testWindow->setVisible(true);
    testWindow->setEnabled(false);
    QVERIFY(interface->state(0) & QAccessible::Unavailable);
    testWindow->setEnabled(true);
    qApp->setActiveWindow(&mdiArea);
    mdiArea.setActiveSubWindow(testWindow);
    QVERIFY(testWindow->isAncestorOf(qApp->focusWidget()));
    QVERIFY(interface->state(0) & QAccessible::Focused);
    testWindow->setGeometry(originalGeometry);

    // navigate
    QAccessibleInterface *destination = 0;
    QCOMPARE(interface->navigate(QAccessible::Child, 1, &destination), 1);
    QVERIFY(destination);
    QCOMPARE(destination->object(), testWindow->widget());
    delete destination;
    QCOMPARE(interface->navigate(QAccessible::Left, 0, &destination), 0);
    QVERIFY(destination);
    QCOMPARE(destination->object(), subWindows.at(2));
    delete destination;
    QCOMPARE(interface->navigate(QAccessible::Right, 0, &destination), 0);
    QVERIFY(destination);
    QCOMPARE(destination->object(), subWindows.at(4));
    delete destination;

    // rect
    const QPoint globalPos = testWindow->mapToGlobal(QPoint(0, 0));
    QCOMPARE(interface->rect(0), QRect(globalPos, testWindow->size()));
    testWindow->hide();
    QCOMPARE(interface->rect(0), QRect());
    QCOMPARE(interface->rect(1), QRect());
    testWindow->showMinimized();
    QCOMPARE(interface->rect(0).size(), testWindow->iconSize());
    QCOMPARE(interface->rect(1), QRect());
    testWindow->showNormal();
    testWindow->widget()->hide();
    QCOMPARE(interface->rect(1), QRect());
    testWindow->widget()->show();
    const QRect widgetGeometry = testWindow->contentsRect();
    const QPoint globalWidgetPos = QPoint(globalPos.x() + widgetGeometry.x(),
                                          globalPos.y() + widgetGeometry.y());
    QCOMPARE(interface->rect(1), QRect(globalWidgetPos, widgetGeometry.size()));

    // childAt
    QCOMPARE(interface->childAt(-10, 0), -1);
    QCOMPARE(interface->childAt(globalPos.x(), globalPos.y()), 0);
    QCOMPARE(interface->childAt(globalWidgetPos.x(), globalWidgetPos.y()), 1);
    testWindow->widget()->hide();
    QCOMPARE(interface->childAt(globalWidgetPos.x(), globalWidgetPos.y()), 0);

    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::lineEditTest()
{
#ifdef QTEST_ACCESSIBILITY
    QLineEdit *le = new QLineEdit;
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(le);
    QVERIFY(iface);
    le->show();

    QApplication::processEvents();
    QCOMPARE(iface->childCount(), 0);
    QVERIFY(iface->state(0) & QAccessible::Sizeable);
    QVERIFY(iface->state(0) & QAccessible::Movable);
    QCOMPARE(bool(iface->state(0) & QAccessible::Focusable), le->isActiveWindow());
    QVERIFY(iface->state(0) & QAccessible::Selectable);
    QVERIFY(iface->state(0) & QAccessible::HasPopup);
    QCOMPARE(bool(iface->state(0) & QAccessible::Focused), le->hasFocus());
    
    QWidget *toplevel = new QWidget;
    le->setParent(toplevel);
    toplevel->show();
    QApplication::processEvents();
    QVERIFY(!(iface->state(0) & QAccessible::Sizeable));
    QVERIFY(!(iface->state(0) & QAccessible::Movable));
    QCOMPARE(bool(iface->state(0) & QAccessible::Focusable), le->isActiveWindow());
    QVERIFY(iface->state(0) & QAccessible::Selectable);
    QVERIFY(iface->state(0) & QAccessible::HasPopup);
    QCOMPARE(bool(iface->state(0) & QAccessible::Focused), le->hasFocus());
    delete iface;
    delete le;
    delete toplevel;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::workspaceTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QWorkspace workspace;
    workspace.show();
    const int subWindowCount =  5;
    for (int i = 0; i < subWindowCount; ++i) {
        QWidget *window = workspace.addWindow(new QWidget);
        if (i > 0)
            window->move(window->x() + 1, window->y());
        window->show();
    }

    QWidgetList subWindows = workspace.windowList();
    QCOMPARE(subWindows.count(), subWindowCount);

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&workspace);
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), subWindowCount);

    // Right, right, right, ...
    for (int i = 0; i < subWindowCount; ++i) {
        QAccessibleInterface *destination = 0;
        int index = interface->navigate(QAccessible::Right, i + 1, &destination);
        if (i == subWindowCount - 1) {
            QVERIFY(!destination);
            QCOMPARE(index, -1);
        } else {
            QVERIFY(destination);
            QCOMPARE(index, i + 2);
            QCOMPARE(destination->object(), subWindows.at(i + 1));
            delete destination;
        }
    }

    // Left, left, left, ...
    for (int i = subWindowCount; i > 0; --i) {
        QAccessibleInterface *destination = 0;
        int index = interface->navigate(QAccessible::Left, i, &destination);
        if (i == 1) {
            QVERIFY(!destination);
            QCOMPARE(index, -1);
        } else {
            QVERIFY(destination);
            QCOMPARE(index, i - 1);
            QCOMPARE(destination->object(), subWindows.at(i - 2));
            delete destination;
        }
    }
    // ### Add test for Up and Down.
    
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::dialogButtonBoxTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QDialogButtonBox box(QDialogButtonBox::Reset | 
                         QDialogButtonBox::Help | 
                         QDialogButtonBox::Ok, Qt::Horizontal);
    

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(&box);
    QVERIFY(iface);
    box.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&box);
#endif

    QApplication::processEvents();
    QCOMPARE(iface->childCount(), 3);
    QCOMPARE(iface->role(0), QAccessible::Grouping);
    QCOMPARE(iface->role(1), QAccessible::PushButton);
    QCOMPARE(iface->role(2), QAccessible::PushButton);
    QCOMPARE(iface->role(3), QAccessible::PushButton);
    QStringList actualOrder;
    QAccessibleInterface *child;
    QAccessibleInterface *leftmost;
    iface->navigate(QAccessible::Child, 1, &child);
    // first find the leftmost button
    while (child->navigate(QAccessible::Left, 1, &leftmost) != -1) {
        delete child;
        child = leftmost;
    }
    leftmost = child;

    // then traverse from left to right to find the correct order of the buttons
    int right = 0;
    while (right != -1) {
        actualOrder << leftmost->text(QAccessible::Name, 0);
        right = leftmost->navigate(QAccessible::Right, 1, &child);
        delete leftmost;
        leftmost = child;
    }

    QStringList expectedOrder;
    QDialogButtonBox::ButtonLayout btnlout = 
        QDialogButtonBox::ButtonLayout(QApplication::style()->styleHint(QStyle::SH_DialogButtonLayout));
    switch (btnlout) {
    case QDialogButtonBox::WinLayout:
        expectedOrder << QDialogButtonBox::tr("Reset") 
                      << QDialogButtonBox::tr("OK")
                      << QDialogButtonBox::tr("Help");
        break;
    case QDialogButtonBox::GnomeLayout:
    case QDialogButtonBox::KdeLayout:
    case QDialogButtonBox::MacLayout:
        expectedOrder << QDialogButtonBox::tr("Help") 
                      << QDialogButtonBox::tr("Reset") 
                      << QDialogButtonBox::tr("OK");
        break;
    }
    QCOMPARE(actualOrder, expectedOrder);
    delete iface;
    QApplication::processEvents();
    QTestAccessibility::clearEvents();
    }

    {
    QDialogButtonBox box(QDialogButtonBox::Reset | 
                         QDialogButtonBox::Help | 
                         QDialogButtonBox::Ok, Qt::Horizontal);
    

    // Test up and down navigation
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(&box);
    QVERIFY(iface);
    box.setOrientation(Qt::Vertical);
    box.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&box);
#endif

    QApplication::processEvents();
    QAccessibleInterface *child;
    QStringList actualOrder;
    iface->navigate(QAccessible::Child, 1, &child);
    // first find the topmost button
    QAccessibleInterface *other;
    while (child->navigate(QAccessible::Up, 1, &other) != -1) {
        delete child;
        child = other;
    }
    other = child;

    // then traverse from top to bottom to find the correct order of the buttons
    actualOrder.clear();
    int right = 0;
    while (right != -1) {
        actualOrder << other->text(QAccessible::Name, 0);
        right = other->navigate(QAccessible::Down, 1, &child);
        delete other;
        other = child;
    }

    QStringList expectedOrder;
    expectedOrder << QDialogButtonBox::tr("OK")
                  << QDialogButtonBox::tr("Reset")
                  << QDialogButtonBox::tr("Help");

    QCOMPARE(actualOrder, expectedOrder);
    delete iface;
    QApplication::processEvents();

    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::dialTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QDial dial;
    dial.setValue(20);
    QCOMPARE(dial.value(), 20);
    dial.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&dial);
#endif
    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&dial);
    QVERIFY(interface);

    // Child count; 1 = SpeedoMeter, 2 = SliderHandle.
    QCOMPARE(interface->childCount(), 2);

    QCOMPARE(interface->role(0), QAccessible::Dial);
    QCOMPARE(interface->role(1), QAccessible::Slider);
    QCOMPARE(interface->role(2), QAccessible::Indicator);

    QCOMPARE(interface->text(QAccessible::Value, 0), QString::number(dial.value()));
    QCOMPARE(interface->text(QAccessible::Value, 1), QString::number(dial.value()));
    QCOMPARE(interface->text(QAccessible::Value, 2), QString::number(dial.value()));
    QCOMPARE(interface->text(QAccessible::Name, 0), QLatin1String("QDial"));
    QCOMPARE(interface->text(QAccessible::Name, 1), QLatin1String("SpeedoMeter"));
    QCOMPARE(interface->text(QAccessible::Name, 2), QLatin1String("SliderHandle"));
    QCOMPARE(interface->text(QAccessible::Name, 3), QLatin1String(""));

    QCOMPARE(interface->state(1), interface->state(0));
    QCOMPARE(interface->state(2), interface->state(0) | QAccessible::HotTracked);

    // Rect
    QCOMPARE(interface->rect(0), dial.geometry());
    QVERIFY(interface->rect(1).isValid());
    QVERIFY(dial.geometry().contains(interface->rect(1)));
    QVERIFY(interface->rect(2).isValid());
    QVERIFY(interface->rect(1).contains(interface->rect(2)));
    QVERIFY(!interface->rect(3).isValid());

    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::rubberBandTest()
{
#ifdef QTEST_ACCESSIBILITY
    QRubberBand rubberBand(QRubberBand::Rectangle);
    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&rubberBand);
    QVERIFY(interface);
    QCOMPARE(interface->role(0), QAccessible::Border);
    delete interface;
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::abstractScrollAreaTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QAbstractScrollArea abstractScrollArea;

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&abstractScrollArea);
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), 0);
    QVERIFY(!interface->rect(0).isValid());
    QVERIFY(!interface->rect(1).isValid());
    QCOMPARE(interface->childAt(200, 200), -1);

    abstractScrollArea.resize(400, 400);
    abstractScrollArea.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&abstractScrollArea);
#endif
    const QRect globalGeometry = QRect(abstractScrollArea.mapToGlobal(QPoint(0, 0)),
                                       abstractScrollArea.size());

    // Viewport.
    QCOMPARE(interface->childCount(), 1);
    QWidget *viewport = abstractScrollArea.viewport();
    QVERIFY(viewport);
    QVERIFY(verifyChild(viewport, interface, 1, globalGeometry));

    // Horizontal scrollBar.
    abstractScrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QCOMPARE(interface->childCount(), 2);
    QWidget *horizontalScrollBar = abstractScrollArea.horizontalScrollBar();
    QVERIFY(verifyChild(horizontalScrollBar, interface, 2, globalGeometry));

    // Horizontal scrollBar widgets.
    QLabel *secondLeftLabel = new QLabel(QLatin1String("L2"));
    abstractScrollArea.addScrollBarWidget(secondLeftLabel, Qt::AlignLeft);
    QCOMPARE(interface->childCount(), 3);
    QVERIFY(verifyChild(secondLeftLabel, interface, 3, globalGeometry));

    QLabel *firstLeftLabel = new QLabel(QLatin1String("L1"));
    abstractScrollArea.addScrollBarWidget(firstLeftLabel, Qt::AlignLeft);
    QCOMPARE(interface->childCount(), 4);
    // NB! index = 2 because firstLeftLabel is the first left label.
    QVERIFY(verifyChild(firstLeftLabel, interface, 3, globalGeometry));

    QLabel *secondRightLabel = new QLabel(QLatin1String("R2"));
    abstractScrollArea.addScrollBarWidget(secondRightLabel, Qt::AlignRight);
    QCOMPARE(interface->childCount(), 5);
    QVERIFY(verifyChild(secondRightLabel, interface, 5, globalGeometry));

    QLabel *firstRightLabel = new QLabel(QLatin1String("R1"));
    abstractScrollArea.addScrollBarWidget(firstRightLabel, Qt::AlignRight);
    QCOMPARE(interface->childCount(), 6);
    QVERIFY(verifyChild(secondRightLabel, interface, 6, globalGeometry));

    // Vertical scrollBar.
    abstractScrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QCOMPARE(interface->childCount(), 7);
    QWidget *verticalScrollBar = abstractScrollArea.verticalScrollBar();
    QVERIFY(verifyChild(verticalScrollBar, interface, 7, globalGeometry));

    // Vertical scrollBar widgets.
    QLabel *secondTopLabel = new QLabel(QLatin1String("T2"));
    abstractScrollArea.addScrollBarWidget(secondTopLabel, Qt::AlignTop);
    QCOMPARE(interface->childCount(), 8);
    QVERIFY(verifyChild(secondTopLabel, interface, 8, globalGeometry));

    QLabel *firstTopLabel = new QLabel(QLatin1String("T1"));
    abstractScrollArea.addScrollBarWidget(firstTopLabel, Qt::AlignTop);
    QCOMPARE(interface->childCount(), 9);
    // NB! index = 8 because firstTopLabel is the first top label.
    QVERIFY(verifyChild(firstTopLabel, interface, 8, globalGeometry));

    QLabel *secondBottomLabel = new QLabel(QLatin1String("B2"));
    abstractScrollArea.addScrollBarWidget(secondBottomLabel, Qt::AlignBottom);
    QCOMPARE(interface->childCount(), 10);
    QVERIFY(verifyChild(secondBottomLabel, interface, 10, globalGeometry));

    QLabel *firstBottomLabel = new QLabel(QLatin1String("B1"));
    abstractScrollArea.addScrollBarWidget(firstBottomLabel, Qt::AlignBottom);
    QCOMPARE(interface->childCount(), 11);
    // NB! index = 10 because firstBottomLabel is the first bottom label.
    QVERIFY(verifyChild(firstBottomLabel, interface, 10, globalGeometry));

    // CornerWidget.
    abstractScrollArea.setCornerWidget(new QLabel(QLatin1String("C")));
    QCOMPARE(interface->childCount(), 12);
    QWidget *cornerWidget = abstractScrollArea.cornerWidget();
    QVERIFY(verifyChild(cornerWidget, interface, 12, globalGeometry));

    // Test navigate.
    QAccessibleInterface *target = 0;
    // viewport -> Down -> horizontalScrollBar
    const int viewportIndex = indexOfChild(interface, viewport);
    QVERIFY(viewportIndex != -1);
    const int horizontalScrollBarIndex = indexOfChild(interface, horizontalScrollBar);
    QVERIFY(horizontalScrollBarIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Down, viewportIndex, &target), horizontalScrollBarIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), horizontalScrollBar);
    delete target;
    target = 0;

    // horizontalScrollBar -> Right -> firstRightLabel
    const int firstRightLabelIndex = indexOfChild(interface, firstRightLabel);
    QVERIFY(firstRightLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Right, horizontalScrollBarIndex, &target), firstRightLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), firstRightLabel);
    delete target;
    target = 0;

    // firstRightLabel -> Right -> secondRightLagel
    const int secondRightLabelIndex = indexOfChild(interface, secondRightLabel);
    QVERIFY(secondRightLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Right, firstRightLabelIndex, &target), secondRightLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), secondRightLabel);
    delete target;
    target = 0;

    // secondRightLabel -> Right -> cornerWidget
    const int cornerWidgetIndex = indexOfChild(interface, cornerWidget);
    QVERIFY(cornerWidgetIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Right, secondRightLabelIndex, &target), cornerWidgetIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), cornerWidget);
    delete target;
    target = 0;

    // cornerWidget -> Right -> NOTHING
    QVERIFY(cornerWidgetIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Right, cornerWidgetIndex, &target), -1);
    QVERIFY(!target);

    // cornerWidget -> Up -> secondBottomLabel
    const int secondBottomLabelIndex = indexOfChild(interface, secondBottomLabel);
    QVERIFY(secondBottomLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Up, cornerWidgetIndex, &target), secondBottomLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), secondBottomLabel);
    delete target;
    target = 0;

    // secondBottomLabel -> Up -> firstBottomLabel
    const int firstBottomLabelIndex = indexOfChild(interface, firstBottomLabel);
    QVERIFY(firstBottomLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Up, secondBottomLabelIndex, &target), firstBottomLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), firstBottomLabel);
    delete target;
    target = 0;

    // firstBottomLabel -> Up -> verticalScrollBar
    const int verticalScrollBarIndex = indexOfChild(interface, verticalScrollBar);
    QVERIFY(verticalScrollBarIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Up, firstBottomLabelIndex, &target), verticalScrollBarIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), verticalScrollBar);
    delete target;
    target = 0;

    // verticalScrollBar -> Up -> secondTopLabel
    const int secondTopLabelIndex = indexOfChild(interface, secondTopLabel);
    QVERIFY(secondTopLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Up, verticalScrollBarIndex, &target), secondTopLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), secondTopLabel);
    delete target;
    target = 0;

    // secondTopLabel -> Up -> firstTopLabel
    const int firstTopLabelIndex = indexOfChild(interface, firstTopLabel);
    QVERIFY(firstTopLabelIndex != -1);
    QCOMPARE(interface->navigate(QAccessible::Up, secondTopLabelIndex, &target), firstTopLabelIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), firstTopLabel);
    delete target;
    target = 0;

    // firstTopLabel -> Up -> NOTHING
    QCOMPARE(interface->navigate(QAccessible::Up, firstTopLabelIndex, &target), -1);
    QVERIFY(!target);

    // firstTopLabel -> Left -> viewport
    QCOMPARE(interface->navigate(QAccessible::Left, firstTopLabelIndex, &target), viewportIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), viewport);
    delete target;
    target = 0;

    // firstRightLabel -> Left -> horizontalScrollBar
    QCOMPARE(interface->navigate(QAccessible::Left, firstRightLabelIndex, &target), horizontalScrollBarIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), horizontalScrollBar);
    delete target;
    target = 0;

    // secondTopLabel -> Down -> verticalScrollBar
    QCOMPARE(interface->navigate(QAccessible::Down, secondTopLabelIndex, &target), verticalScrollBarIndex);
    QVERIFY(target);
    QCOMPARE(target->object(), verticalScrollBar);
    delete target;
    target = 0;

    delete interface;
    }

    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::scrollAreaTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QScrollArea scrollArea;
    scrollArea.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&scrollArea);
#endif
    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&scrollArea);
    QVERIFY(interface);
    QCOMPARE(interface->childCount(), 1); // The viewport.
    delete interface;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
}

void tst_QAccessibility::tableWidgetTest()
{
#ifdef QTEST_ACCESSIBILITY
    {
    QTableWidget *w = new QTableWidget(8,4);
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 4; ++c) {
            w->setItem(r, c, new QTableWidgetItem(tr("%1,%2").arg(c).arg(r)));
        }
    }
    w->resize(100, 100);
    w->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(w);
#endif
    QAccessibleInterface *client = QAccessible::queryAccessibleInterface(w);
    QCOMPARE(client->role(0), QAccessible::Client);
    QCOMPARE(client->childCount(), 3);
    QAccessibleInterface *view = 0;
    client->navigate(QAccessible::Child, 1, &view);
    QCOMPARE(view->role(0), QAccessible::Table);
    QAccessibleInterface *ifRow;
    view->navigate(QAccessible::Child, 1, &ifRow);
    QCOMPARE(ifRow->role(0), QAccessible::Row);
    QAccessibleInterface *item;
    int entry = ifRow->navigate(QAccessible::Child, 1, &item);
    QCOMPARE(entry, 1);
    QCOMPARE(item , (QAccessibleInterface*)0);
    QCOMPARE(ifRow->text(QAccessible::Value, 1), QLatin1String("0,0"));
    QCOMPARE(ifRow->text(QAccessible::Value, 2), QLatin1String("1,0"));

    delete ifRow;
    delete view;
    delete client;
    delete w;
    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif

}

class QtTestTableModel: public QAbstractTableModel
{
    Q_OBJECT

signals:
    void invalidIndexEncountered() const;

public:
    QtTestTableModel(int rows = 0, int columns = 0, QObject *parent = 0)
        : QAbstractTableModel(parent),
          row_count(rows),
          column_count(columns) {}

    int rowCount(const QModelIndex& = QModelIndex()) const { return row_count; }
    int columnCount(const QModelIndex& = QModelIndex()) const { return column_count; }

    QVariant data(const QModelIndex &idx, int role) const
    {
        if (!idx.isValid() || idx.row() >= row_count || idx.column() >= column_count) {
            qWarning() << "Invalid modelIndex [%d,%d,%p]" << idx;
            emit invalidIndexEncountered();
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(0);

        return QVariant();
    }

    void removeLastRow()
    {
        beginRemoveRows(QModelIndex(), row_count - 1, row_count - 1);
        --row_count;
        endRemoveRows();
    }

    void removeAllRows()
    {
        beginRemoveRows(QModelIndex(), 0, row_count - 1);
        row_count = 0;
        endRemoveRows();
    }

    void removeLastColumn()
    {
        beginRemoveColumns(QModelIndex(), column_count - 1, column_count - 1);
        --column_count;
        endRemoveColumns();
    }

    void removeAllColumns()
    {
        beginRemoveColumns(QModelIndex(), 0, column_count - 1);
        column_count = 0;
        endRemoveColumns();
    }

    void reset()
    {
        QAbstractTableModel::reset();
    }

    int row_count;
    int column_count;
};

class QtTestDelegate : public QItemDelegate
{
public:
    QtTestDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}

    virtual QSize sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
    {
        return QSize(100,50);
    }
};

void tst_QAccessibility::tableViewTest()
{
#ifdef QTEST_ACCESSIBILITY
    QtTestTableModel *model = new QtTestTableModel(8, 4);
    QTableView *w = new QTableView();
    w->setModel(model);
    w->setItemDelegate(new QtTestDelegate(w));
    w->resize(400,900);
    w->resizeColumnsToContents();
    w->resizeRowsToContents();
    w->show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(w);
#endif
    QAccessibleInterface *client = QAccessible::queryAccessibleInterface(w);
    for (int y = 0; y < 8; ++y) {
        QCOMPARE(client->role(0), QAccessible::Client);
        QRect globalRect = client->rect(0);
        QVERIFY(globalRect.isValid());
        QPoint p = globalRect.topLeft() + QPoint(40,40);
        p.ry() += 50 * y;
        int index = client->childAt(p.x(), p.y());
        QCOMPARE(index, 1);
        QCOMPARE(client->role(index), QAccessible::Table);
        QAccessibleInterface *table;
        client->navigate(QAccessible::Child, 1, &table);
        QVERIFY(table);
        index = table->childAt(p.x(), p.y());
        QCOMPARE(index, y + 1);
        QCOMPARE(table->role(index), QAccessible::Row);
        QAccessibleInterface *row;
        table->navigate(QAccessible::Child, index, &row);
        QVERIFY(row);
        index = row->childAt(p.x(), p.y());
        QVERIFY(index > 0);
        QCOMPARE(row->role(index), QAccessible::Cell);
        QCOMPARE(row->text(QAccessible::Value, index), QString::fromAscii("[%1,0,0]").arg(y));
        delete table;
        delete row;
    }
    delete client;

    delete w;
    delete model;
}

void tst_QAccessibility::calendarWidgetTest()
{
#ifndef QT_NO_CALENDARWIDGET
#ifdef QTEST_ACCESSIBILITY
    {
    QCalendarWidget calendarWidget;

    QAccessibleInterface *interface = QAccessible::queryAccessibleInterface(&calendarWidget);
    QVERIFY(interface);
    QCOMPARE(interface->role(0), QAccessible::Table);
    QCOMPARE(interface->childCount(), 0);
    QVERIFY(!interface->rect(0).isValid());
    QVERIFY(!interface->rect(1).isValid());
    QCOMPARE(interface->childAt(200, 200), -1);

    calendarWidget.resize(400, 300);
    calendarWidget.show();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&calendarWidget);
#endif

    // 1 = navigationBar, 2 = view.
    QCOMPARE(interface->childCount(), 2);

    const QRect globalGeometry = QRect(calendarWidget.mapToGlobal(QPoint(0, 0)),
                                       calendarWidget.size());
    QCOMPARE(interface->rect(0), globalGeometry);

    QWidget *navigationBar = 0;
    foreach (QObject *child, calendarWidget.children()) {
        if (child->objectName() == QLatin1String("qt_calendar_navigationbar")) {
            navigationBar = static_cast<QWidget *>(child);
            break;
        }
    }
    QVERIFY(navigationBar);
    QVERIFY(verifyChild(navigationBar, interface, 1, globalGeometry));

    QAbstractItemView *calendarView = 0;
    foreach (QObject *child, calendarWidget.children()) {
        if (child->objectName() == QLatin1String("qt_calendar_calendarview")) {
            calendarView = static_cast<QAbstractItemView *>(child);
            break;
        }
    }
    QVERIFY(calendarView);
    QVERIFY(verifyChild(calendarView, interface, 2, globalGeometry));

    // Hide navigation bar.
    calendarWidget.setNavigationBarVisible(false);
    QCOMPARE(interface->childCount(), 1);
    QVERIFY(!navigationBar->isVisible());

    QVERIFY(verifyChild(calendarView, interface, 1, globalGeometry));

    // Show navigation bar.
    calendarWidget.setNavigationBarVisible(true);
    QCOMPARE(interface->childCount(), 2);
    QVERIFY(navigationBar->isVisible());

    // Navigate to the navigation bar via Child.
    QAccessibleInterface *navigationBarInterface = 0;
    QCOMPARE(interface->navigate(QAccessible::Child, 1, &navigationBarInterface), 1);
    QVERIFY(navigationBarInterface);
    QCOMPARE(navigationBarInterface->object(), navigationBar);
    delete navigationBarInterface;
    navigationBarInterface = 0;

    // Navigate to the view via Child.
    QAccessibleInterface *calendarViewInterface = 0;
    QCOMPARE(interface->navigate(QAccessible::Child, 2, &calendarViewInterface), 2);
    QVERIFY(calendarViewInterface);
    QCOMPARE(calendarViewInterface->object(), calendarView);
    delete calendarViewInterface;
    calendarViewInterface = 0;

    QAccessibleInterface *doesNotExistsInterface = 0;
    QCOMPARE(interface->navigate(QAccessible::Child, 3, &doesNotExistsInterface), -1);
    QVERIFY(!doesNotExistsInterface);

    // Navigate from navigation bar -> view (Down).
    QCOMPARE(interface->navigate(QAccessible::Down, 1, &calendarViewInterface), 2);
    QVERIFY(calendarViewInterface);
    QCOMPARE(calendarViewInterface->object(), calendarView);
    delete calendarViewInterface;
    calendarViewInterface = 0;

    // Navigate from view -> navigation bar (Up).
    QCOMPARE(interface->navigate(QAccessible::Up, 2, &navigationBarInterface), 1);
    QVERIFY(navigationBarInterface);
    QCOMPARE(navigationBarInterface->object(), navigationBar);
    delete navigationBarInterface;
    navigationBarInterface = 0;

    }
    QTestAccessibility::clearEvents();
#else
    QSKIP("Test needs Qt >= 0x040000 and accessibility support.", SkipAll);
#endif
#endif // QT_NO_CALENDARWIDGET
}

QTEST_MAIN(tst_QAccessibility)
#include "tst_qaccessibility.moc"
