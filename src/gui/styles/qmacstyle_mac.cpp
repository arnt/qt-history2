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

#include "qmacstyle_mac.h"

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)

#include <qpainter.h>
#include <private/qpainter_p.h>
#include <qpaintengine_mac.h>
#include <qpaintdevice.h>
#include <qpixmap.h>
#include <qmap.h>
#include <qpointer.h>

#include "qmacstyleqd_mac.h"
#include "qmacstylecg_mac.h"

QPixmap qt_mac_convert_iconref(IconRef icon, int width, int height);
struct QMacStylePrivate {
    struct PolicyState {
        static QMap<const QWidget*, QMacStyle::FocusRectPolicy> focusMap;
        static QMap<const QWidget*, QMacStyle::WidgetSizePolicy> sizeMap;
        static void watchObject(const QObject *o);
    };
};
QMap<const QWidget*, QMacStyle::FocusRectPolicy> QMacStylePrivate::PolicyState::focusMap;
QMap<const QWidget*, QMacStyle::WidgetSizePolicy> QMacStylePrivate::PolicyState::sizeMap;
class QMacStylePrivateObjectWatcher : public QObject
{
    Q_OBJECT
public:
    QMacStylePrivateObjectWatcher(QObject *p) : QObject(p) { }
public slots:
    void destroyedObject(QObject *);
};
#include "qmacstyle_mac.moc"

void QMacStylePrivate::PolicyState::watchObject(const QObject *o)
{
    static QPointer<QMacStylePrivateObjectWatcher> watcher;
    if(!watcher)
        watcher = new QMacStylePrivateObjectWatcher(NULL);
    QObject::connect(o, SIGNAL(destroyed(QObject*)), watcher, SLOT(destroyedObject(QObject*)));
}
void QMacStylePrivateObjectWatcher::destroyedObject(QObject *o)
{
    QMacStylePrivate::PolicyState::focusMap.remove((QWidget*)o);
    QMacStylePrivate::PolicyState::sizeMap.remove((QWidget*)o);
}


/*!
    \class QMacStyle qmacstyle_mac.h
    \brief The QMacStyle class implements an Appearance Manager style.

    \ingroup appearance

    This class is implemented as a wrapper to the Apple Appearance
    Manager. This allows your application to be styled by whatever
    theme your Macintosh is using. This is done by having primitives
    in QStyle implemented in terms of what the Macintosh would
    normally theme (i.e. the Finder).

    There are additional issues that should be taken
    into consideration to make an application compatible with the
    \link http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/index.html
    Aqua Style Guidelines \endlink. Some of these issues are outlined
    below.

    \list

    \i Layout - The restrictions on window layout are such that some
    aspects of layout that are style-dependent cannot be achieved
    using QLayout. Changes are being considered (and feedback would be
    appreciated) to make layouts QStyle-able. Some of the restrictions
    involve horizontal and vertical widget alignment and widget size
    (covered below).

    \i Widget size - Aqua allows widgets to have specific fixed sizes.  Qt
    does not fully implement this behavior so as to maintain cross-platform
    compatibility. As a result some widgets sizes may be inappropriate (and
    subsequently not rendered correctly by the Appearance Manager).The
    QWidget::sizeHint() will return the appropriate size for many
    managed widgets (widgets enumerated in \l QStyle::ContentsType).

    \i Effects - QMacStyle uses Appearance Manager for performing most of
    the drawing, but also uses emulation in a few cases where Appearance
    Manager does not provide the required functionality (for example, QPushButton
    pulsing effects). We tried to make the emulation as close to the original
    as possible. Please report any issues you see in effects or non-standard
    widgets.

    \endlist

    There are other issues that need to be considered in the feel of
    your application (including the general color scheme to match the
    Aqua colors). The Guidelines mentioned above will remain current
    with new advances and design suggestions for Mac OS X.

    Note that the functions provided by QMacStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation.
*/


/*!
    \enum QMacStyle::WidgetSizePolicy

    \value SizeSmall
    \value SizeLarge
    \value SizeMini
    \value SizeDefault
*/

/*!
    Constructs a QMacStyle object.
*/
QMacStyle::QMacStyle() : QWindowsStyle()
{
    qd_style = 0;
    cg_style = 0;
}

/*!
    Destructs a QMacStyle object.
*/
QMacStyle::~QMacStyle()
{
    if(qd_style)
        delete qd_style;
    if(cg_style)
        delete cg_style;
}

/*! \reimp */
void QMacStyle::polish(QApplication* app)
{
#if !defined(QMAC_NO_COREGRAPHICS) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER && !getenv("QT_MAC_USE_APPMANAGER")) {
        if(!cg_style)
            cg_style = new QMacStyleCG();
        cg_style->polish(app);
    }
#endif
    {
        if(!qd_style)
            qd_style = new QMacStyleQD();
        qd_style->polish(app);
    }
}

/*! \reimp */
void QMacStyle::polish(QWidget* w)
{
    correctStyle(w)->polish(w);
}

/*! \reimp */
void QMacStyle::unPolish(QWidget* w)
{
    correctStyle(w)->unPolish(w);
}

/*! \reimp */
int QMacStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    return correctStyle(widget)->pixelMetric(metric, widget);
}

/*! \reimp */
int QMacStyle::styleHint(StyleHint sh, const QWidget *w,
                         const Q3StyleOption &opt, QStyleHintReturn *d) const
{
    return correctStyle(w)->styleHint(sh, w, opt, d);
}

/*! \reimp */
QPixmap QMacStyle::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                               const QPalette &pal, const Q3StyleOption &opt) const
{
    return correctStyle(&pixmap)->stylePixmap(pixmaptype, pixmap, pal, opt);
}

/*! \reimp */
QPixmap QMacStyle::stylePixmap(StylePixmap stylepixmap, const QWidget *widget,
                               const Q3StyleOption &opt) const
{
    return correctStyle(widget)->stylePixmap(stylepixmap, widget, opt);
}

/*!
    \enum QMacStyle::FocusRectPolicy

    This type is used to signify a widget's focus rectangle policy.

    \value FocusEnabled  show a focus rectangle when the widget has focus.
    \value FocusDisabled  never show a focus rectangle for the widget.
    \value FocusDefault  show a focus rectangle when the widget has
    focus and the widget is a QSpinWidget, QDateTimeEdit, QLineEdit,
    QListBox, QListView, editable QTextEdit, or one of their
    subclasses.
*/

/*!
    Sets the focus rectangle policy of \a w. The \a policy can be one of
    \l{QMacStyle::FocusRectPolicy}.

    \sa focusRectPolicy()
*/
void QMacStyle::setFocusRectPolicy(QWidget *w, FocusRectPolicy policy)
{
    QMacStylePrivate::PolicyState::focusMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
    if (w->hasFocus()) {
        w->clearFocus();
        w->setFocus();
    }
}

/*!
    Returns the focus rectangle policy for the widget \a w.

    The focus rectangle policy can be one of \l{QMacStyle::FocusRectPolicy}.

    \sa setFocusRectPolicy()
*/
QMacStyle::FocusRectPolicy QMacStyle::focusRectPolicy(const QWidget *w)
{
    if (QMacStylePrivate::PolicyState::focusMap.contains(w))
        return QMacStylePrivate::PolicyState::focusMap[w];
    return FocusDefault;
}

/*!
    Sets the widget size policy of \a w. The \a policy can be one of
    \l{QMacStyle::WidgetSizePolicy}.

    \sa widgetSizePolicy()
*/
void QMacStyle::setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy)
{
    QMacStylePrivate::PolicyState::sizeMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
}

/*!
    Returns the widget size policy for the widget \a w.

    The widget size policy can be one of \l{QMacStyle::WidgetSizePolicy}.

    \sa setWidgetSizePolicy()
*/
QMacStyle::WidgetSizePolicy QMacStyle::widgetSizePolicy(const QWidget *w)
{
    WidgetSizePolicy ret = SizeDefault;
    if(w) {
        if (QMacStylePrivate::PolicyState::sizeMap.contains(w))
            ret = QMacStylePrivate::PolicyState::sizeMap[w];
        if(ret == SizeDefault) {
	    for(QWidget *p = w->parentWidget(); p; p = p->parentWidget()) {
                if (QMacStylePrivate::PolicyState::sizeMap.contains(p)) {
                    ret = QMacStylePrivate::PolicyState::sizeMap[p];
                    if(ret != SizeDefault)
                        break;
                }
                if(p->isTopLevel())
                    break;
            }
        }
    }
    return ret;
}

QStyle *QMacStyle::correctStyle(const QPainter *p) const
{
    return correctStyle(p ? p->device() : 0);
}

QStyle *QMacStyle::correctStyle(const QPaintDevice *pdev) const
{
#if !defined(QMAC_NO_COREGRAPHICS) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    bool ret_cg_style = QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER;
    if(ret_cg_style && pdev && pdev->paintEngine())
        ret_cg_style = (pdev->paintEngine()->type() == QPaintEngine::CoreGraphics);
    if(ret_cg_style && !getenv("QT_MAC_USE_APPMANAGER")) {
        if(!cg_style)
            cg_style = new QMacStyleCG();
        return cg_style;
    } else
#else
    Q_UNUSED(pdev);
#endif
    {
        if(!qd_style)
            qd_style = new QMacStyleQD();
        return qd_style;
    }
    return 0;
}

/*! \reimp */
void QMacStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                               const QWidget *w) const
{
    correctStyle(p)->drawPrimitive(pe, opt, p, w);
}

/*! \reimp */
void QMacStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                            const QWidget *w) const
{
    correctStyle(p)->drawControl(ce, opt, p, w);
}

/*! \reimp */
QRect QMacStyle::subRect(SubRect sr, const QStyleOption *opt, const QWidget *w) const
{
    return correctStyle(w)->subRect(sr, opt, w);
}

/*! \reimp */
void QMacStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                   const QWidget *w) const
{
    correctStyle(p)->drawComplexControl(cc, opt, p, w);
}

/*! \reimp */
QStyle::SubControl QMacStyle::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                              const QPoint &pt, const QWidget *w) const
{
    return correctStyle(w)->querySubControl(cc, opt, pt, w);
}

/*! \reimp */
QRect QMacStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                        SubControl sc, const QWidget *w) const
{
    return correctStyle(w)->querySubControlMetrics(cc, opt, sc, w);
}

/*! \reimp */
QSize QMacStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &sz,
                                  const QFontMetrics &fm, const QWidget *w) const
{
    return correctStyle(w)->sizeFromContents(ct, opt, sz, fm, w);
}

#endif
