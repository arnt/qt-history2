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

#include "qcontext2dcanvas.h"

#include "context2d.h"
#include "domimage.h"

#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDateTime>

struct FakeDomEvent;

Q_DECLARE_METATYPE(FakeDomEvent)

struct FakeDomEvent
{
    enum KeyCodes  {
        DOM_VK_UNDEFINED            = 0x0,
        DOM_VK_RIGHT_ALT            = 0x12,
        DOM_VK_LEFT_ALT             = 0x12,
        DOM_VK_LEFT_CONTROL         = 0x11,
        DOM_VK_RIGHT_CONTROL        = 0x11,
        DOM_VK_LEFT_SHIFT           = 0x10,
        DOM_VK_RIGHT_SHIFT          = 0x10,
        DOM_VK_META                 = 0x9D,
        DOM_VK_BACK_SPACE           = 0x08,
        DOM_VK_CAPS_LOCK            = 0x14,
        DOM_VK_DELETE               = 0x7F,
        DOM_VK_END                  = 0x23,
        DOM_VK_ENTER                = 0x0D,
        DOM_VK_ESCAPE               = 0x1B,
        DOM_VK_HOME                 = 0x24,
        DOM_VK_NUM_LOCK             = 0x90,
        DOM_VK_PAUSE                = 0x13,
        DOM_VK_PRINTSCREEN          = 0x9A,
        DOM_VK_SCROLL_LOCK          = 0x91,
        DOM_VK_SPACE                = 0x20,
        DOM_VK_TAB                  = 0x09,
        DOM_VK_LEFT                 = 0x25,
        DOM_VK_RIGHT                = 0x27,
        DOM_VK_UP                   = 0x26,
        DOM_VK_DOWN                 = 0x28,
        DOM_VK_PAGE_DOWN            = 0x22,
        DOM_VK_PAGE_UP              = 0x21,
        DOM_VK_F1                   = 0x70,
        DOM_VK_F2                   = 0x71,
        DOM_VK_F3                   = 0x72,
        DOM_VK_F4                   = 0x73,
        DOM_VK_F5                   = 0x74,
        DOM_VK_F6                   = 0x75,
        DOM_VK_F7                   = 0x76,
        DOM_VK_F8                   = 0x77,
        DOM_VK_F9                   = 0x78,
        DOM_VK_F10                  = 0x79,
        DOM_VK_F11                  = 0x7A,
        DOM_VK_F12                  = 0x7B,
        DOM_VK_F13                  = 0xF000,
        DOM_VK_F14                  = 0xF001,
        DOM_VK_F15                  = 0xF002,
        DOM_VK_F16                  = 0xF003,
        DOM_VK_F17                  = 0xF004,
        DOM_VK_F18                  = 0xF005,
        DOM_VK_F19                  = 0xF006,
        DOM_VK_F20                  = 0xF007,
        DOM_VK_F21                  = 0xF008,
        DOM_VK_F22                  = 0xF009,
        DOM_VK_F23                  = 0xF00A,
        DOM_VK_F24                  = 0xF00B
    };
    static int qtToDomKey(int keyCode) {
        switch (keyCode) {
        case Qt::Key_Backspace:
            return  DOM_VK_BACK_SPACE;
        case Qt::Key_Enter:
            return  DOM_VK_ENTER;
        case Qt::Key_Return:
            return  DOM_VK_ENTER;
        case Qt::Key_NumLock:
            return  DOM_VK_NUM_LOCK;
        case Qt::Key_Alt:
            return  DOM_VK_RIGHT_ALT;
        case Qt::Key_Control:
            return  DOM_VK_LEFT_CONTROL;
        case Qt::Key_Shift:
            return  DOM_VK_LEFT_SHIFT;
        case Qt::Key_Meta:
            return  DOM_VK_META;
        case Qt::Key_CapsLock:
            return  DOM_VK_CAPS_LOCK;
        case Qt::Key_Delete:
            return  DOM_VK_DELETE;
        case Qt::Key_End:
            return  DOM_VK_END;
        case Qt::Key_Escape:
            return  DOM_VK_ESCAPE;
        case Qt::Key_Home:
            return  DOM_VK_HOME;
        case Qt::Key_Pause:
            return  DOM_VK_PAUSE;
        case Qt::Key_Print:
            return  DOM_VK_PRINTSCREEN;
        case Qt::Key_ScrollLock:
            return  DOM_VK_SCROLL_LOCK;
        case Qt::Key_Left:
            return  DOM_VK_LEFT;
        case Qt::Key_Right:
            return  DOM_VK_RIGHT;
        case Qt::Key_Up:
            return  DOM_VK_UP;
        case Qt::Key_Down:
            return  DOM_VK_DOWN;
        case Qt::Key_PageDown:
            return  DOM_VK_PAGE_DOWN;
        case Qt::Key_PageUp:
            return  DOM_VK_PAGE_UP;
        case Qt::Key_F1:
            return  DOM_VK_F1;
        case Qt::Key_F2:
            return  DOM_VK_F2;
        case Qt::Key_F3:
            return  DOM_VK_F3;
        case Qt::Key_F4:
            return  DOM_VK_F4;
        case Qt::Key_F5:
            return  DOM_VK_F5;
        case Qt::Key_F6:
            return  DOM_VK_F6;
        case Qt::Key_F7:
            return  DOM_VK_F7;
        case Qt::Key_F8:
            return  DOM_VK_F8;
        case Qt::Key_F9:
            return  DOM_VK_F9;
        case Qt::Key_F10:
            return  DOM_VK_F10;
        case Qt::Key_F11:
            return  DOM_VK_F11;
        case Qt::Key_F12:
            return  DOM_VK_F12;
        case Qt::Key_F13:
            return  DOM_VK_F13;
        case Qt::Key_F14:
            return  DOM_VK_F14;
        case Qt::Key_F15:
            return  DOM_VK_F15;
        case Qt::Key_F16:
            return  DOM_VK_F16;
        case Qt::Key_F17:
            return  DOM_VK_F17;
        case Qt::Key_F18:
            return  DOM_VK_F18;
        case Qt::Key_F19:
            return  DOM_VK_F19;
        case Qt::Key_F20:
            return  DOM_VK_F20;
        case Qt::Key_F21:
            return  DOM_VK_F21;
        case Qt::Key_F22:
            return  DOM_VK_F22;
        case Qt::Key_F23:
            return  DOM_VK_F23;
        case Qt::Key_F24:
            return  DOM_VK_F24;
        }
        return keyCode;
    }

    static QScriptValue m_proto;

    FakeDomEvent()
    {
    }

    FakeDomEvent(QMouseEvent *e, QScriptEngine *eng)
    {
        setupDefaults(eng);
        setupModifiers(e, eng);
        m_proto.setProperty("type",
                            eng->scriptValue(QLatin1String("mouseevent")));
        int button = 0;
        if (e->button() == Qt::RightButton)
            button = 2;
        else if (e->button() == Qt::MidButton)
            button = 1;
        m_proto.setProperty("button", eng->scriptValue(button));

        m_proto.setProperty("clientX", eng->scriptValue(e->x()));
        m_proto.setProperty("clientY", eng->scriptValue(e->y()));
        m_proto.setProperty("layerX", eng->scriptValue(e->x()));
        m_proto.setProperty("layerY", eng->scriptValue(e->y()));
        m_proto.setProperty("pageX", eng->scriptValue(e->x()));
        m_proto.setProperty("pageY", eng->scriptValue(e->y()));
        m_proto.setProperty("screenX", eng->scriptValue(e->globalX()));
        m_proto.setProperty("screenY", eng->scriptValue(e->globalY()));

        eng->setDefaultPrototype(qMetaTypeId<FakeDomEvent>(), m_proto);
    }
    FakeDomEvent(QKeyEvent *e, QScriptEngine *eng)
    {
        setupDefaults(eng);
        setupModifiers(e, eng);
        m_proto.setProperty("type", eng->scriptValue(QLatin1String("keyevent")));

        m_proto.setProperty("isChar", eng->scriptValue(!e->text().isEmpty()));
        m_proto.setProperty("charCode", eng->scriptValue(e->text()));
        m_proto.setProperty("keyCode", eng->scriptValue(qtToDomKey(e->key())));
        m_proto.setProperty("which", eng->scriptValue(e->key()));

        eng->setDefaultPrototype(qMetaTypeId<FakeDomEvent>(), m_proto);
    }


    static void setup(QScriptEngine *e)
    {
        qRegisterMetaType<FakeDomEvent>();

        m_proto = e->newObject();

        setupDefaults(e);

        m_proto.setProperty("bubbles", e->scriptValue(0));
        m_proto.setProperty("cancelBubble", e->scriptValue(0));
        m_proto.setProperty("cancelable", e->scriptValue(0));
        m_proto.setProperty("currentTarget", e->scriptValue(0));
        m_proto.setProperty("detail", e->scriptValue(0));
        m_proto.setProperty("eventPhase", e->scriptValue(0));
        m_proto.setProperty("explicitOriginalTarget", e->scriptValue(0));
        m_proto.setProperty("originalTarget", e->scriptValue(0));
        m_proto.setProperty("relatedTarget", e->scriptValue(0));
        m_proto.setProperty("target", e->scriptValue(0));
        m_proto.setProperty("view", e->scriptValue(0));
    }

    static void setupDefaults(QScriptEngine *e)
    {
        m_proto.setProperty("timeStamp", e->scriptValue(QDateTime::currentDateTime().toTime_t()));
        m_proto.setProperty("button", e->scriptValue(0));
        m_proto.setProperty("charCode", e->scriptValue(0));
        m_proto.setProperty("clientX", e->scriptValue(0));
        m_proto.setProperty("clientY", e->scriptValue(0));
        m_proto.setProperty("isChar", e->scriptValue(0));
        m_proto.setProperty("keyCode", e->scriptValue(0));
        m_proto.setProperty("layerX", e->scriptValue(0));
        m_proto.setProperty("layerY", e->scriptValue(0));
        m_proto.setProperty("pageX", e->scriptValue(0));
        m_proto.setProperty("pageY", e->scriptValue(0));
        m_proto.setProperty("screenX", e->scriptValue(0));
        m_proto.setProperty("screenY", e->scriptValue(0));
        m_proto.setProperty("which ", e->scriptValue(0));
    }
    void setupModifiers(QInputEvent *e, QScriptEngine *eng)
    {
        if (e->modifiers() & Qt::AltModifier)
            m_proto.setProperty("altKey", eng->scriptValue(true));
        else
            m_proto.setProperty("altKey", eng->scriptValue(false));

        if (e->modifiers() & Qt::ControlModifier)
            m_proto.setProperty("ctrlKey", eng->scriptValue(true));
        else
            m_proto.setProperty("ctrlKey", eng->scriptValue(false));

        if (e->modifiers() & Qt::MetaModifier)
            m_proto.setProperty("metaKey", eng->scriptValue(true));
        else
            m_proto.setProperty("metaKey", eng->scriptValue(false));

        if (e->modifiers() & Qt::ShiftModifier)
            m_proto.setProperty("shiftKey", eng->scriptValue(true));
        else
            m_proto.setProperty("shiftKey", eng->scriptValue(true));
    }
};

QScriptValue FakeDomEvent::m_proto;

static QScriptValue getElementById(QScriptContext *context, QScriptEngine *env)
{
    QString el  = context->argument(0).toString();
    QList<QObject*> lst =
        QCoreApplication::instance()->findChildren<QObject*>(el);
    if (lst.isEmpty()) {
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (el == widget->objectName() ) {
                lst << widget;
                break;
            } else {
                lst = widget->findChildren<QObject*>(el);
            }

            if (lst.count())
                break;
        }
    }
    //qDebug()<<"serching element "<<el<<lst.count();

    return env->newQObject(lst[0]);
}

static QScriptValue setInterval(QScriptContext *context, QScriptEngine *env)
{
    QScriptValue func = context->argument(0);
    qnumber interval  = context->argument(1).toNumber();
    QScriptValue val = env->globalObject().property("_qcontext");
    QContext2DCanvas *canvas =
        qobject_cast<QContext2DCanvas*>(val.toQObject());
    if (!canvas) {
        qWarning("Couldn't find a QContext2DCanvas!");
        return env->undefinedValue();
    }
    if (!func.isFunction()) {
        QString funcStr = func.toString();
        if (funcStr.endsWith("()")) {
            funcStr.chop(2);
        }
        func = env->globalObject().property(funcStr);
    }
    if (!func.isFunction()) {
        qDebug()<<"Couldn't find function "
                <<context->argument(0).toString();
        return env->undefinedValue();
    }

    //qDebug()<<"setInterval "<<func.isFunction()<<", "<<interval;
    canvas->setInterval(func, interval);

    return env->undefinedValue();
}


static QScriptValue setTimeout(QScriptContext *ctx, QScriptEngine *env)
{
    return setInterval(ctx, env);
}

QContext2DCanvas::QContext2DCanvas(QWidget *parent)
    : QWidget(parent),
      m_firstRun(true)
{
    setObjectName("tutorial");
    m_context = new Context2D(this);

    m_self = m_engine.newQObject(this);
    m_self.ref();

    FakeDomEvent::setup(&m_engine);
    DomImage::setup(&m_engine);

    m_doc = m_engine.newObject();
    m_doc.ref();
    m_doc.setProperty("getElementById",
                    m_engine.newFunction(::getElementById,
                                            /*argc = */ 1));

    m_engine.globalObject().setProperty("document", m_doc);


    m_engine.globalObject().setProperty(
        "setInterval",
        m_engine.newFunction(::setInterval, 2));
    m_engine.globalObject().setProperty(
        "setTimeout",
        m_engine.newFunction(::setTimeout, 2));
    m_engine.globalObject().setProperty("_qcontext", m_self);

    connect(&m_timer, SIGNAL(timeout()),
            SLOT(update()));
}

QScriptEngine *QContext2DCanvas::engine()
{
    return &m_engine;
}

void QContext2DCanvas::setScriptContents(const QString &txt)
{
    m_timer.stop();
    m_intervalFunc     = QScriptValue();
    m_keyDownHandler   = QScriptValue();
    m_keyUpHandler     = QScriptValue();
    m_mouseDownHandler = QScriptValue();
    m_mouseUpHandler   = QScriptValue();
    m_mouseMoveHandler = QScriptValue();
    m_firstRun = true;
    m_script = txt;
    m_context->clear();
    update();
}

void QContext2DCanvas::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

    QScriptValue ret;
    if (m_intervalFunc.isValid()) {
        //qDebug()<<"Calling function ";
        m_context->begin();
        ret = m_intervalFunc.call(m_engine.globalObject());
    } else {
        //qDebug()<<"Didn't find inter.";
        ret = m_engine.evaluate(m_script);
    }
    if (!m_script.isEmpty() && m_engine.hasUncaughtException()) {
        int lineno = m_engine.uncaughtExceptionLineNumber();
        QString msg = ret.toString();
        qDebug() << msg <<", at "<< lineno;
        emit error(msg, lineno);
        m_timer.stop();
        return;
    }
    p.drawPixmap(0, 0, m_context->end());

    if (m_firstRun) {
        m_keyDownHandler = m_doc.property("onkeydown");
        m_keyUpHandler = m_doc.property("onkeyup");
        m_mouseDownHandler = m_doc.property("onmousedown");
        m_mouseUpHandler = m_doc.property("onmouseup");
        m_mouseMoveHandler = m_doc.property("onmousemove");
        setMouseTracking(m_mouseMoveHandler.isValid());
    }

    m_firstRun = false;
}


void QContext2DCanvas::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if (m_mouseMoveHandler.isValid()) {
        FakeDomEvent event(e, &m_engine);
        QScriptValueList args;
        args << event.m_proto;
        //qDebug()<<"invoke mouse move";
        m_mouseMoveHandler.call(m_engine.globalObject(), args);
    }
}


void QContext2DCanvas::mousePressEvent(QMouseEvent *e)
{
    if (m_mouseDownHandler.isValid()) {
        //qDebug()<<"invoke mouse down";
        FakeDomEvent event(e, &m_engine);
        QScriptValueList args;
        args << event.m_proto;
        m_mouseDownHandler.call(m_engine.globalObject(), args);
    }
}


void QContext2DCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_mouseUpHandler.isValid()) {
        //qDebug()<<"invoke mouse up";
        FakeDomEvent event(e, &m_engine);
        QScriptValueList args;
        args << event.m_proto;
        m_mouseUpHandler.call(m_engine.globalObject(), args);
    }
}


void QContext2DCanvas::keyPressEvent(QKeyEvent *e)
{
    if (m_keyDownHandler.isValid()) {
        //qDebug()<<"invoke key handler "<<m_keyDownHandler.toString();;
        FakeDomEvent event(e, &m_engine);
        QScriptValueList args;
        args << event.m_proto;
        m_keyDownHandler.call(m_engine.globalObject(), args);
    }
}

void QContext2DCanvas::keyReleaseEvent(QKeyEvent *e)
{
    if (m_keyUpHandler.isValid()) {
        //qDebug()<<"invoke key handler "<<m_keyUpHandler.toString();;
        FakeDomEvent event(e, &m_engine);
        QScriptValueList args;
        args << event.m_proto;
        m_keyUpHandler.call(m_engine.globalObject(), args);
    }
}


void QContext2DCanvas::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    //we delete th
    //delete m_context;
    //m_context = new Context2D(this);
    m_context->setSize(e->size().width(), e->size().height());
}

QObject * QContext2DCanvas::getContext(const QString &str) const
{
    //qDebug()<<"getting context "<<str<<m_context;
    if (str != QLatin1String("2d")) {
        qDebug()<<"Passing invalid context "<<str;
    }

    m_context->begin();
    return m_context;
}

QContext2DCanvas::~QContext2DCanvas()
{
    m_self.deref();
    m_doc.deref();
}

void QContext2DCanvas::setInterval(const QScriptValue &func,
                                   qreal interval)
{
    m_intervalFunc = func;
    m_timer.setInterval((int)interval);
    m_timer.start();
}

void QContext2DCanvas::resize(int w, int h)
{
    QWidget::resize(w, h);
}

