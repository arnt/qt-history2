/****************************************************************************
**
** Implementation of QEffects functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"
#ifndef QT_NO_EFFECTS
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qeffects_p.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qtimer.h"
#include "qdatetime.h"
#include "qpointer.h"
#include "qscrollview.h"

/*
  Internal class to get access to protected QWidget-members
*/

class QAccessWidget : public QWidget
{
    friend class QAlphaWidget;
    friend class QRollEffect;
public:
    QAccessWidget(QWidget* parent=0, const char* name=0, Qt::WFlags f = 0)
        : QWidget(parent, name, f) {}
};

/*
  Internal class QAlphaWidget.

  The QAlphaWidget is shown while the animation lasts
  and displays the pixmap resulting from the alpha blending.
*/

class QAlphaWidget: public QWidget, private QEffects
{
    Q_OBJECT
public:
    QAlphaWidget(QWidget* w, Qt::WFlags f = 0);

    void run(int time);

protected:
    void paintEvent(QPaintEvent* e);
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject* o, QEvent* e);
    void alphaBlend();

protected slots:
    void render();

private:
    QPixmap pm;
    double alpha;
    QImage back;
    QImage front;
    QImage mixed;
    QPointer<QAccessWidget> widget;
    int duration;
    int elapsed;
    bool showWidget;
    QTimer anim;
    QTime checkTime;
};

static QAlphaWidget* q_blend = 0;

/*
  Constructs a QAlphaWidget.
*/
QAlphaWidget::QAlphaWidget(QWidget* w, Qt::WFlags f)
    : QWidget(0, "qt internal alpha effect widget", f)
{
    setEnabled(false);

    pm.setOptimization(QPixmap::BestOptim);
    setAttribute(Qt::WA_NoSystemBackground, true);
    widget = (QAccessWidget*)w;
    alpha = 0;
}

/*
  \reimp
*/
void QAlphaWidget::paintEvent(QPaintEvent*)
{
    bitBlt(this, QPoint(0,0), &pm);
}

/*
  Starts the alphablending animation.
  The animation will take about \a time ms
*/
void QAlphaWidget::run(int time)
{
    duration = time;

    if (duration < 0)
        duration = 150;

    if (!widget)
        return;

    elapsed = 0;
    checkTime.start();

    showWidget = true;
    qApp->installEventFilter(this);

    move(widget->geometry().x(),widget->geometry().y());
    resize(widget->size().width(), widget->size().height());

    front = QImage(widget->size(), 32);
    front = QPixmap::grabWidget(widget);

    back = QImage(widget->size(), 32);
    back = QPixmap::grabWindow(QApplication::desktop()->winId(),
                                widget->geometry().x(), widget->geometry().y(),
                                widget->geometry().width(), widget->geometry().height());

    if (!back.isNull() && checkTime.elapsed() < duration / 2) {
        mixed = back.copy();
        pm = mixed;
        show();

        connect(&anim, SIGNAL(timeout()), this, SLOT(render()));
        anim.start(1);
    } else {
        duration = 0;
        render();
    }
}

/*
  \reimp
*/
bool QAlphaWidget::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type()) {
    case QEvent::Move:
        if (o != widget)
            break;
        move(widget->geometry().x(),widget->geometry().y());
        update();
        break;
    case QEvent::Hide:
    case QEvent::Close:
        if (o != widget)
            break;
    case QEvent::MouseButtonPress:
#ifndef QT_NO_SCROLLVIEW
        if (qt_cast<QScrollView*>(o))
            break;
#endif
    case QEvent::MouseButtonDblClick:
        setEnabled(true);
        showWidget = false;
        render();
        break;
    case QEvent::KeyPress:
        {
            QKeyEvent *ke = (QKeyEvent*)e;
            if (ke->key() == Qt::Key_Escape)
                showWidget = false;
            else
                duration = 0;
            render();
            break;
        }
    default:
        break;
    }
    return QWidget::eventFilter(o, e);
}

/*
  \reimp
*/
void QAlphaWidget::closeEvent(QCloseEvent *e)
{
    e->accept();
    if (!q_blend)
        return;

    showWidget = false;
    render();

    QWidget::closeEvent(e);
}

/*
  Render alphablending for the time elapsed.

  Show the blended widget and free all allocated source
  if the blending is finished.
*/
void QAlphaWidget::render()
{
    int tempel = checkTime.elapsed();
    if (elapsed >= tempel)
        elapsed++;
    else
        elapsed = tempel;

    if (duration != 0)
        alpha = tempel / double(duration);
    else
        alpha = 1;
    if (alpha >= 1 || !showWidget) {
        anim.stop();
        qApp->removeEventFilter(this);

        if (widget) {
            if (!showWidget) {
                widget->hide();
            } else if (duration) {
                bool nsb = testAttribute(Qt::WA_NoSystemBackground);
                widget->setAttribute(Qt::WA_NoSystemBackground, true);
                widget->show();
                widget->setAttribute(Qt::WA_NoSystemBackground, nsb);
            } else {
                widget->show();
            }
        }
        q_blend = 0;
        deleteLater();
    } else {
        alphaBlend();
        pm = mixed;
        repaint();
    }
}

/*
  Calculate an alphablended image.
*/
void QAlphaWidget::alphaBlend()
{
    const double ia = 1-alpha;
    const int sw = front.width();
    const int sh = front.height();
    switch(front.depth()) {
    case 32:
        {
            Q_UINT32** md = (Q_UINT32**)mixed.jumpTable();
            Q_UINT32** bd = (Q_UINT32**)back.jumpTable();
            Q_UINT32** fd = (Q_UINT32**)front.jumpTable();

            for (int sy = 0; sy < sh; sy++) {
                Q_UINT32* bl = ((Q_UINT32*)bd[sy]);
                Q_UINT32* fl = ((Q_UINT32*)fd[sy]);
                for (int sx = 0; sx < sw; sx++) {
                    Q_UINT32 bp = bl[sx];
                    Q_UINT32 fp = fl[sx];

                    ((Q_UINT32*)(md[sy]))[sx] =  qRgb(int (qRed(bp)*ia + qRed(fp)*alpha),
                                                    int (qGreen(bp)*ia + qGreen(fp)*alpha),
                                                    int (qBlue(bp)*ia + qBlue(fp)*alpha));
                }
            }
        }
    default:
        break;
    }
}

/*
  Internal class QRollEffect

  The QRollEffect widget is shown while the animation lasts
  and displays a scrolling pixmap.
*/

class QRollEffect : public QWidget, private QEffects
{
    Q_OBJECT
public:
    QRollEffect(QWidget* w, Qt::WFlags f, DirFlags orient);

    void run(int time);

protected:
    void paintEvent(QPaintEvent*);
    bool eventFilter(QObject*, QEvent*);
    void closeEvent(QCloseEvent*);

private slots:
    void scroll();

private:
    QPointer<QAccessWidget> widget;

    int currentHeight;
    int currentWidth;
    int totalHeight;
    int totalWidth;

    int duration;
    int elapsed;
    bool done;
    bool showWidget;
    int orientation;

    QTimer anim;
    QTime checkTime;

    QPixmap pm;
};

static QRollEffect* q_roll = 0;

/*
  Construct a QRollEffect widget.
*/
QRollEffect::QRollEffect(QWidget* w, Qt::WFlags f, DirFlags orient)
    : QWidget(0, "qt internal roll effect widget", f), orientation(orient)
{
    setEnabled(false);
    widget = (QAccessWidget*) w;
    Q_ASSERT(widget);

    setAttribute(Qt::WA_NoSystemBackground, true);

    if (widget->testAttribute(Qt::WA_Resized)) {
        totalWidth = widget->width();
        totalHeight = widget->height();
    } else {
        totalWidth = widget->sizeHint().width();
        totalHeight = widget->sizeHint().height();
    }

    currentHeight = totalHeight;
    currentWidth = totalWidth;

    if (orientation & (RightScroll|LeftScroll))
        currentWidth = 0;
    if (orientation & (DownScroll|UpScroll))
        currentHeight = 0;

    pm.setOptimization(QPixmap::BestOptim);
    pm = QPixmap::grabWidget(widget);
}

/*
  \reimp
*/
void QRollEffect::paintEvent(QPaintEvent*)
{
    int x = orientation & RightScroll ? qMin(0, currentWidth - totalWidth) : 0;
    int y = orientation & DownScroll ? qMin(0, currentHeight - totalHeight) : 0;

    bitBlt(this, x, y, &pm,
                  0, 0, pm.width(), pm.height(), true);
}

/*
  \reimp
*/
bool QRollEffect::eventFilter(QObject* o, QEvent* e)
{
    switch (e->type()) {
    case QEvent::Move:
        if (o != widget)
            break;
        move(widget->geometry().x(),widget->geometry().y());
        update();
        break;
    case QEvent::Hide:
    case QEvent::Close:
        if (o != widget || done)
            break;
        setEnabled(true);
        showWidget = false;
        done = true;
        scroll();
        break;
    case QEvent::MouseButtonPress:
#ifndef QT_NO_SCROLLVIEW
        if (qt_cast<QScrollView*>(o))
            break;
#endif
    case QEvent::MouseButtonDblClick:
        if (done)
            break;
        setEnabled(true);
        showWidget = false;
        done = true;
        scroll();
        break;
    case QEvent::KeyPress:
        {
            QKeyEvent *ke = (QKeyEvent*)e;
            if (ke->key() == Qt::Key_Escape)
                showWidget = false;
            done = true;
            scroll();
            break;
        }
    default:
        break;
    }
    return QWidget::eventFilter(o, e);
}

/*
  \reimp
*/
void QRollEffect::closeEvent(QCloseEvent *e)
{
    e->accept();
    if (done)
        return;

    showWidget = false;
    done = true;
    scroll();

    QWidget::closeEvent(e);
}

/*
  Start the animation.

  The animation will take about \a time ms, or is
  calculated if \a time is negative
*/
void QRollEffect::run(int time)
{
    if (!widget)
        return;

    duration  = time;
    elapsed = 0;

    if (duration < 0) {
        int dist = 0;
        if (orientation & (RightScroll|LeftScroll))
            dist += totalWidth - currentWidth;
        if (orientation & (DownScroll|UpScroll))
            dist += totalHeight - currentHeight;
        duration = qMin(qMax(dist/3, 50), 120);
    }

    connect(&anim, SIGNAL(timeout()), this, SLOT(scroll()));

    move(widget->geometry().x(),widget->geometry().y());
    resize(qMin(currentWidth, totalWidth), qMin(currentHeight, totalHeight));

    show();

    qApp->installEventFilter(this);

    showWidget = true;
    done = false;
    anim.start(1);
    checkTime.start();
}

/*
  Roll according to the time elapsed.
*/
void QRollEffect::scroll()
{
    if (!done && widget) {
        int tempel = checkTime.elapsed();
        if (elapsed >= tempel)
            elapsed++;
        else
            elapsed = tempel;

        if (currentWidth != totalWidth) {
            currentWidth = totalWidth * (elapsed/duration)
                + (2 * totalWidth * (elapsed%duration) + duration)
                / (2 * duration);
            // equiv. to int((totalWidth*elapsed) / duration + 0.5)
            done = (currentWidth >= totalWidth);
        }
        if (currentHeight != totalHeight) {
            currentHeight = totalHeight * (elapsed/duration)
                + (2 * totalHeight * (elapsed%duration) + duration)
                / (2 * duration);
            // equiv. to int((totalHeight*elapsed) / duration + 0.5)
            done = (currentHeight >= totalHeight);
        }
        done = (currentHeight >= totalHeight) &&
               (currentWidth >= totalWidth);

        int w = totalWidth;
        int h = totalHeight;
        int x = widget->geometry().x();
        int y = widget->geometry().y();

        if (orientation & RightScroll || orientation & LeftScroll)
            w = qMin(currentWidth, totalWidth);
        if (orientation & DownScroll || orientation & UpScroll)
            h = qMin(currentHeight, totalHeight);

        setUpdatesEnabled(false);
        if (orientation & UpScroll)
            y = widget->geometry().y() + qMax(0, totalHeight - currentHeight);
        if (orientation & LeftScroll)
            x = widget->geometry().x() + qMax(0, totalWidth - currentWidth);
        if (orientation & UpScroll || orientation & LeftScroll)
            move(x, y);

        resize(w, h);
        setUpdatesEnabled(true);
        repaint();
    }
    if (done) {
        anim.stop();
        qApp->removeEventFilter(this);
        if (widget) {
            if (!showWidget) {
                widget->hide();
            } else {
                bool nsb = testAttribute(Qt::WA_NoSystemBackground);
                widget->setAttribute(Qt::WA_NoSystemBackground, true);
                widget->show();
                widget->setAttribute(Qt::WA_NoSystemBackground, nsb);
            }
        }
        q_roll = 0;
        deleteLater();
    }
}

/*
  Delete this after timeout
*/

#include "qeffects.moc"

/*!
    Scroll widget \a w in \a time ms. \a orient may be 1 (vertical), 2
    (horizontal) or 3 (diagonal).
*/
void qScrollEffect(QWidget* w, QEffects::DirFlags orient, int time)
{
    if (q_roll) {
        delete q_roll;
        q_roll = 0;
    }

    qApp->sendPostedEvents(w, QEvent::Move);
    qApp->sendPostedEvents(w, QEvent::Resize);
#ifdef Q_WS_X11
    Qt::WFlags flags = Qt::WStyle_Customize | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop
        | (w->isPopup() ? Qt::WFlags(Qt::WType_Popup) : (Qt::WX11BypassWM | Qt::WStyle_Tool));
#else
    Qt::WFlags flags = Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop;
#endif

    // those can popups - they would steal the focus, but are disabled
    q_roll = new QRollEffect(w, flags, orient);
    q_roll->run(time);
}

/*!
    Fade in widget \a w in \a time ms.
*/
void qFadeEffect(QWidget* w, int time)
{
    if (q_blend) {
        delete q_blend;
        q_blend = 0;
    }

    qApp->sendPostedEvents(w, QEvent::Move);
    qApp->sendPostedEvents(w, QEvent::Resize);

#ifdef Q_WS_X11
    Qt::WFlags flags = Qt::WStyle_Customize | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop
        | (w->isPopup() ? QFlag(Qt::WType_Popup) : (Qt::WX11BypassWM | Qt::WStyle_Tool));
#else
    Qt::WFlags flags = Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop;
#endif

    // those can popups - they would steal the focus, but are disabled
    q_blend = new QAlphaWidget(w, flags);

    q_blend->run(time);
}
#endif //QT_NO_EFFECTS
