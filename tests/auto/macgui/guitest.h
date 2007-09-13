#ifndef GUITEST_H
#define GUITEST_H

#include <QAccessibleInterface>
#include <QSet>
#include <QWidget>
#include <QPainter>

QT_USE_NAMESPACE

/*
    GuiTest provides tools for:
     - navigating the Qt Widget hiearchy using the accessibilty APIs.
     - Simulating platform mouse and keybord events.
*/

/*
    InterfaceChildPair specifies an accessibilty interface item.
*/
class InterfaceChildPair {
public:
    InterfaceChildPair() : iface(0), possibleChild(0) {}
    InterfaceChildPair(QAccessibleInterface *iface, int possibleChild)
    :iface(iface), possibleChild(possibleChild)    
    { }
    
    QAccessibleInterface *iface;
    int possibleChild;
};

class TestBase {
public:
    virtual bool operator()(InterfaceChildPair candidate) = 0;
    virtual ~TestBase() {}
};

/*
    WidgetNavigator navigates a Qt GUI hierarchy using the QAccessibility APIs.
*/
class WidgetNavigator {
public:
    WidgetNavigator() {};
    ~WidgetNavigator();

    void printAll(QWidget *widget);
    void printAll(InterfaceChildPair interface);
    
    InterfaceChildPair find(QAccessible::Text textType, const QString &text, QWidget *start);
    InterfaceChildPair find(QAccessible::Text textType, const QString &text, QAccessibleInterface *start);

    InterfaceChildPair recursiveSearch(TestBase *test, QAccessibleInterface *iface, int possibleChild);
    
    void deleteInDestructor(QAccessibleInterface * interface);
    static QWidget *getWidget(InterfaceChildPair interface);
private:
    QSet<QAccessibleInterface *> interfaces;
};

/*
    NativeEvents contains platform-specific code for simulating mouse and keybord events.
    (Implemented so far: mouseClick on Mac)
*/
namespace NativeEvents {
    enum MousePosition { UpdatePosition, DontUpdatePosition };
    /*
        Simulates a mouse click with button at globalPos.
    */
    void mouseClick(const QPoint &globalPos, Qt::MouseButtons buttons, MousePosition updateMouse = DontUpdatePosition);
};

class ColorWidget : public QWidget
{
public:
    ColorWidget(QWidget *parent = 0, QColor color = QColor(Qt::red))
       : QWidget(parent), color(color) {}
    
    QColor color;

protected:
    void paintEvent(QPaintEvent  *)
    {
        QPainter p(this);
        p.fillRect(this->rect(), color);
    }
};

class DelayedAction : public QObject
{
Q_OBJECT
public:
    DelayedAction() : delay(0), next(0) {}
    virtual ~DelayedAction(){}
public slots:
    virtual void run();
public:
    int delay;
    DelayedAction *next;
};

class ClickLaterAction : public DelayedAction
{
Q_OBJECT
public:
    ClickLaterAction(InterfaceChildPair interface, Qt::MouseButtons buttons = Qt::LeftButton);
    ClickLaterAction(QWidget *widget, Qt::MouseButtons buttons = Qt::LeftButton);
protected slots:
    void run();
private:
    bool useInterface;
    InterfaceChildPair interface;
    QWidget *widget;
    Qt::MouseButtons buttons;
};

/*
    
*/
class GuiTester : public QObject
{
Q_OBJECT
public:
    GuiTester();
    ~GuiTester();
    enum Direction {Horizontal = 1, Vertical = 2, HorizontalAndVertical = 3};
    Q_DECLARE_FLAGS(Directions, Direction)
    bool isFilled(const QImage image, const QRect &rect, const QColor &color);
    bool isContent(const QImage image, const QRect &rect, Directions directions = HorizontalAndVertical);
protected slots:
    void exitLoopSlot();
protected:
    void clickLater(InterfaceChildPair interface, Qt::MouseButtons buttons = Qt::LeftButton, int delay = 300);
    void clickLater(QWidget *widget, Qt::MouseButtons buttons = Qt::LeftButton, int delay = 300);

    void clearSequence();
    void addToSequence(DelayedAction *action, int delay = 0);
    void runSequence();
    WidgetNavigator wn;
private:
    QSet<DelayedAction *> actions;
    DelayedAction *startAction;
    DelayedAction *lastAction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GuiTester::Directions)

#endif
