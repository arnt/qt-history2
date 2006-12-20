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

#include "qwindowsvistastyle.h"
#include "qwindowsvistastyle_p.h"

#if !defined(QT_NO_STYLE_WINDOWSVISTA) || defined(QT_PLUGIN)

static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  4; // menu item ver text margin
static const int windowsArrowHMargin	 =  6; // arrow horizontal margin
static const int windowsRightBorder      = 15; // right border on windows

// Runtime resolved theme engine function calls

typedef bool (WINAPI *PtrIsAppThemed)();
typedef bool (WINAPI *PtrIsThemeActive)();
typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
typedef HTHEME (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HTHEME (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrCloseThemeData)(HTHEME hTheme);
typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef HRESULT (WINAPI *PtrDrawThemeBackgroundEx)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);
typedef HRESULT (WINAPI *PtrGetCurrentThemeName)(OUT LPWSTR pszThemeFileName, int cchMaxNameChars, OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars, OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);
typedef HRESULT (WINAPI *PtrGetThemeBool)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT BOOL *pfVal);
typedef HRESULT (WINAPI *PtrGetThemeColor)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor);
typedef HRESULT (WINAPI *PtrGetThemeEnumValue)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemeFilename)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);
typedef HRESULT (WINAPI *PtrGetThemeFont)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OUT LOGFONT *pFont);
typedef HRESULT (WINAPI *PtrGetThemeInt)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemeIntList)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT INTLIST *pIntList);
typedef HRESULT (WINAPI *PtrGetThemeMargins)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins);
typedef HRESULT (WINAPI *PtrGetThemeMetric)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
typedef HRESULT (WINAPI *PtrGetThemePosition)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT POINT *pPoint);
typedef HRESULT (WINAPI *PtrGetThemeRect)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT RECT *pRect);
typedef HRESULT (WINAPI *PtrGetThemeString)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);
typedef HRESULT (WINAPI *PtrGetThemeTransitionDuration)(HTHEME hTheme, int iPartId, int iStateFromId, int iStateToId, int iPropId, int *pDuration);

static PtrIsAppThemed pIsAppThemed = 0;
static PtrIsThemeActive pIsThemeActive = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrCloseThemeData pCloseThemeData = 0;
static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrDrawThemeBackgroundEx pDrawThemeBackgroundEx = 0;
static PtrGetCurrentThemeName pGetCurrentThemeName = 0;
static PtrGetThemeBool pGetThemeBool = 0;
static PtrGetThemeColor pGetThemeColor = 0;
static PtrGetThemeEnumValue pGetThemeEnumValue = 0;
static PtrGetThemeFilename pGetThemeFilename = 0;
static PtrGetThemeFont pGetThemeFont = 0;
static PtrGetThemeInt pGetThemeInt = 0;
static PtrGetThemeIntList pGetThemeIntList = 0;
static PtrGetThemeMargins pGetThemeMargins = 0;
static PtrGetThemeMetric pGetThemeMetric = 0;
static PtrGetThemePartSize pGetThemePartSize = 0;
static PtrGetThemePosition pGetThemePosition = 0;
static PtrGetThemeRect pGetThemeRect = 0;
static PtrGetThemeString pGetThemeString = 0;
static PtrGetThemeTransitionDuration pGetThemeTransitionDuration= 0;

/* \internal
    Checks if we should use Vista style , or if we should
    fall back to Windows style.
*/
bool QWindowsVistaStylePrivate::useVista()
{
    return (QWindowsVistaStylePrivate::useXP() &&
            (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA &&
             QSysInfo::WindowsVersion < QSysInfo::WV_NT_based));
}


QWindowsVistaStyle::QWindowsVistaStyle()
    : QWindowsXPStyle(*new QWindowsVistaStylePrivate)
{
}

//convert Qt state flags to uxtheme button states
int buttonStateId(int flags, int partId)
{
    int stateId = 0;
    if (partId == BP_RADIOBUTTON || partId == BP_CHECKBOX) {
        if (!(flags & QStyle::State_Enabled))
            stateId = RBS_UNCHECKEDDISABLED;
        else if (flags & QStyle::State_Sunken)
            stateId = RBS_UNCHECKEDPRESSED;
        else if (flags & QStyle::State_MouseOver)
            stateId = RBS_UNCHECKEDHOT;
        else
            stateId = RBS_UNCHECKEDNORMAL;

        if (flags & QStyle::State_On)
            stateId += RBS_CHECKEDNORMAL-1;

    } else if (partId == BP_PUSHBUTTON) {
        if (!(flags & QStyle::State_Enabled))
            stateId = PBS_DISABLED;
        else if (flags & (QStyle::State_Sunken | QStyle::State_On))
            stateId = PBS_PRESSED;
        else if (flags & QStyle::State_MouseOver)
            stateId = PBS_HOT;
        else
            stateId = PBS_NORMAL;
    } else {
        Q_ASSERT(1);
    }
    return stateId;
}

void Animation::paint(QPainter *painter, const QStyleOption *option)
{
    Q_UNUSED(option);
    Q_UNUSED(painter);
}

/*
* ! \internal
*
* Helperfunction to paint the current transition state
* between two animation frames.
*
* The result is a blended image consisting of
* ((alpha)*_primaryImage) + ((1-alpha)*_secondaryImage)
*
*/

void Animation::drawBlendedImage(QPainter *painter, QRect rect, float alpha) {
    if (_secondaryImage.isNull() || _primaryImage.isNull())
        return;

    if (_tempImage.isNull())
        _tempImage = _secondaryImage;

    const int a = qRound(alpha*256);
    const int ia = 256 - a;
    const int sw = _primaryImage.width();
    const int sh = _primaryImage.height();
    const int bpl = _primaryImage.bytesPerLine();
    switch(_primaryImage.depth()) {
    case 32:
        {
            uchar *mixed_data = _tempImage.bits();
            const uchar *back_data = _primaryImage.bits();
            const uchar *front_data = _secondaryImage.bits();
            for (int sy = 0; sy < sh; sy++) {
                quint32* mixed = (quint32*)mixed_data;
                const quint32* back = (const quint32*)back_data;
                const quint32* front = (const quint32*)front_data;
                for (int sx = 0; sx < sw; sx++) {
                    quint32 bp = back[sx];
                    quint32 fp = front[sx];
                    mixed[sx] =  qRgba ((qRed(bp)*ia + qRed(fp)*a)>>8,
                                        (qGreen(bp)*ia + qGreen(fp)*a)>>8,
                                        (qBlue(bp)*ia + qBlue(fp)*a)>>8,
                                        (qAlpha(bp)*ia + qAlpha(fp)*a)>>8);
                }
                mixed_data += bpl;
                back_data += bpl;
                front_data += bpl;
            }
        }
    default:
        break;
    }
    painter->drawImage(rect, _tempImage);
}

/*
* ! \internal
*
* Paints a transition state. The result will be a mix between the
* initial and final state of the transition, depending on the
* time difference between _startTime and current time.
*/

void Transition::paint(QPainter *painter, const QStyleOption *option)
{
    QTime current = QTime::currentTime();
    float alpha = 1.0;
    if (_duration > 0) {
        int timeDiff = _startTime.msecsTo(current);
        alpha = timeDiff/(float)_duration;
        if (timeDiff > _duration) {
            _running = false;
            alpha = 1.0;
        }
    }
    else {
        _running = false;
    }
    drawBlendedImage(painter, option->rect, alpha);
}

/*
* ! \internal
*
* Paints a pulse. The result will be a mix between the
* primary and secondary pulse images depending on the
* time difference between _startTime and current time.
*/


void Pulse::paint(QPainter *painter, const QStyleOption *option)
{
    float alpha = 1.0;
    if (_duration > 0) {
        QTime current = QTime::currentTime();
        int timeDiff = _startTime.msecsTo(current) % _duration*2;
        if (timeDiff > _duration)
            timeDiff = _duration*2 - timeDiff;
        alpha = timeDiff/(float)_duration;
    } else {
        _running = false;
    }
    drawBlendedImage(painter, option->rect, alpha);
}


/*!
 \reimp
 *
 * Animations are used for some state transitions on specific widgets.
 *
 * Only one running animation can exist for a widget at any specific time.
 * Animations can be added through QWindowsVistaStylePrivate::startAnimation(Animation *)
 * and any existing animation on a widget can be retrieved with
 * QWindowsVistaStylePrivate::widgetAnimation(Widget *).
 *
 * Once an animation has been started, QWindowsVistaStylePrivate::timerEvent(QTimerEvent *)
 * will continuously call update() on the widget until it is stopped, meaning that drawPrimitive
 * will be called many times until the transition has completed. During this time, the result
 * will be retrieved by the Animation::paint(...) function and not by the style itself.
 *
 * To determine if a transition should occur, the style needs to know the previous state of the
 * widget as well as the current one. This is solved by updating dynamic properties on the widget
 * every time the function is called.
 *
 * Transitions interrupting existing transitions should allways be smooth, so whenever a hover-transition
 * is started on a pulsating button, it uses the current frame of the pulse-animation as the
 * starting image for the hover transition.
 *
 */
void QWindowsVistaStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                    QPainter *painter, const QWidget *widget) const
{
    QWindowsVistaStylePrivate *d = const_cast<QWindowsVistaStylePrivate*>(d_func());

    int state = option->state;
    if (!QWindowsVistaStylePrivate::useVista()) {
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
        return;
    }

    QRect oldRect;
    QRect newRect;

    if (widget && d->transitionsEnabled())
    {
        /* all widgets that supports state transitions : */
        if ((qobject_cast<const QLineEdit*>(widget) && element == PE_FrameLineEdit) ||
            (qobject_cast<const QRadioButton*>(widget)&& element == PE_IndicatorRadioButton) ||
            (qobject_cast<const QCheckBox*>(widget) && element == PE_IndicatorCheckBox) ||
            (qobject_cast<const QGroupBox *>(widget)&& element == PE_IndicatorCheckBox) ||
            (qobject_cast<const QToolButton*>(widget) && element == PE_PanelButtonBevel)
        )
        {
            // Retrieve and update the dynamic properties tracking
            // the previous state of the widget:
            QWidget *w = const_cast<QWidget *> (widget);
            int oldState = w->property("qt_stylestate").toInt();
            oldRect = w->property("qt_stylerect").toRect();
            newRect = w->rect();
            w->setProperty("qt_stylestate", (int)option->state);
            w->setProperty("qt_stylerect", w->rect());

            bool doTransition = ((state & State_Sunken)     != (oldState & State_Sunken) ||
                                 (state & State_On)         != (oldState & State_On)     ||
                                 (state & State_MouseOver)  != (oldState & State_MouseOver));

            if (oldRect != newRect ||
                (state & State_Enabled) != (oldState & State_Enabled) ||
                (state & State_Active)  != (oldState & State_Active))
                    d->stopAnimation(widget);

            if (const QLineEdit *edit = qobject_cast<const QLineEdit *>(widget))// Do not animate read only line edits
                if (edit->isReadOnly() && element == PE_FrameLineEdit)
                    doTransition = false;

            if (doTransition) {

                // We create separate images for the initial and final transition states and store them in the
                // Transition object.

                QImage startImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                QImage endImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                QStyleOption opt = *option;

                opt.rect.setRect(0, 0, option->rect.width(), option->rect.height());
                opt.state = (QStyle::State)oldState;
                startImage.fill(0);
                QPainter startPainter(&startImage);

                Animation *anim = d->widgetAnimation(widget);
                Transition *t = new Transition;
                t->setWidget(w);

                // If we have a running animation on the widget already, we will use that to paint the initial
                // state of the new transition, this ensures a smooth transition from a current animation such as a
                // pulsating default button into the intended target state.

                if (!anim)
                    drawPrimitive(element, &opt, &startPainter, 0); // Note that the widget pointer is intentionally 0
                else                                               // this ensures that we do not recurse in the animation logic above
                    anim->paint(&startPainter, &opt);

                d->startAnimation(t);
                t->setStartImage(startImage);

                // The end state of the transition is simply the result we would have painted
                // if the style was not animated.

                QPainter endPainter(&endImage);
                endImage.fill(0);
                QStyleOption opt2 = opt;
                opt2.state = option->state;
                drawPrimitive(element, &opt2, &endPainter, 0); // Note that the widget pointer is intentionally 0
                                                              // this ensures that we do not recurse in the animation logic above
                t->setEndImage(endImage);

                int partId, duration;

                if (element == PE_IndicatorRadioButton)
                    partId = BP_RADIOBUTTON;
                else if (element == PE_IndicatorCheckBox)
                    partId = BP_CHECKBOX;
                else
                    partId = BP_PUSHBUTTON;

                int fromState = buttonStateId(oldState, partId);
                int toState = buttonStateId(option->state, partId);

                HTHEME theme = pOpenThemeData(0, L"Button");

                // Retrieve the transition time between the states from the system.
                if (element != PE_FrameLineEdit &&
                    pGetThemeTransitionDuration(theme, partId, fromState, toState, TMT_TRANSITIONDURATIONS, &duration) == S_OK)
                {
                    t->setDuration(duration);
                }
                t->setStartTime(QTime::currentTime());
            }
        }
    } // End of animation part


    QRect rect = option->rect;

    switch (element) {
    case PE_PanelButtonBevel:
    case PE_IndicatorCheckBox:
    case PE_IndicatorRadioButton:
        {
            if (Animation *a = d->widgetAnimation(widget)) {
                a->paint(painter, option);
            } else {
                QWindowsXPStyle::drawPrimitive(element, option, painter, widget);
            }
        }
        break;

    case PE_IndicatorToolBarHandle:
        {
            XPThemeData theme;
            if (option->state & State_Horizontal)
                theme = XPThemeData(widget, painter, QLatin1String("REBAR"), RP_GRIPPER, ETS_NORMAL, option->rect.adjusted(0, 1, -2, -2));
            else
                theme = XPThemeData(widget, painter, QLatin1String("REBAR"), RP_GRIPPERVERT, ETS_NORMAL, option->rect.adjusted(0, 1, -2, -2));
            d->drawBackground(theme);
        }
        break;
    case PE_FrameLineEdit:
        if (Animation *anim = d->widgetAnimation(widget)) {
            anim->paint(painter, option);
        } else {
            bool readOnly = false;
            if (const QLineEdit *edit = qobject_cast<const QLineEdit *>(widget))
                if (edit->isReadOnly())
                    readOnly = true;
            int stateId = ETS_NORMAL;
            if (!(state & State_Enabled))
                stateId = ETS_DISABLED;
            else if (readOnly)
                stateId = ETS_READONLY;
            else if (state & State_MouseOver)
                stateId = ETS_HOT;
            else if (state & State_HasFocus)
                stateId = ETS_SELECTED;

            XPThemeData theme(widget, painter, QLatin1String("EDIT"), EP_EDITBORDER_NOSCROLL, stateId, option->rect);
            d->drawBackground(theme);
        }
        break;

    case PE_IndicatorToolBarSeparator:
        {
            QPen pen = painter->pen();
            int x1 = option->rect.center().x();
            int margin = 3;
            painter->setPen(option->palette.background().color().darker(114));
            painter->drawLine(QPoint(x1, option->rect.top() + margin), QPoint(x1, option->rect.bottom() - margin));
            painter->setPen(pen);
        }
        break;

    case PE_PanelTipLabel: {
        XPThemeData theme(widget, painter, QLatin1String("TOOLTIP"), TTP_STANDARD, TTSS_NORMAL, option->rect);
        d->drawBackground(theme);
        break;
    }

    default:
        QWindowsXPStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}


/*!

 \reimp

 see drawPrimitive for comments on the animation support

 */
void QWindowsVistaStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    QWindowsVistaStylePrivate *d = const_cast<QWindowsVistaStylePrivate*>(d_func());

    if (!QWindowsVistaStylePrivate::useVista()) {
        QWindowsStyle::drawControl(element, option, painter, widget);
        return;
    }

    bool selected = option->state & State_Selected;
    bool pressed = option->state & State_Sunken;
    bool disabled = !(option->state & State_Enabled);

    int state = option->state;
    QString name;

    QRect rect(option->rect);
    State flags = option->state;
    int partId = 0;
    int stateId = 0;

    QRect oldRect;
    QRect newRect;

    if (d->transitionsEnabled() && widget) {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            if ((qobject_cast<const QPushButton*>(widget) && element == CE_PushButtonBevel))
            {
                QWidget *w = const_cast<QWidget *> (widget);
                int oldState = w->property("qt_stylestate").toInt();
                oldRect = w->property("qt_stylerect").toRect();
                newRect = w->rect();
                w->setProperty("qt_stylestate", (int)option->state);
                w->setProperty("qt_stylerect", w->rect());

                bool wasDefault = w->property("qt_isdefault").toBool();
                bool isDefault = button->features & QStyleOptionButton::DefaultButton;
                w->setProperty("qt_isdefault", isDefault);

                bool doTransition = ((state & State_Sunken)     != (oldState & State_Sunken) ||
                                     (state & State_On)         != (oldState & State_On)     ||
                                     (state & State_MouseOver)  != (oldState & State_MouseOver));

                if (oldRect != newRect || (wasDefault && !isDefault))
                {
                    doTransition = false;
                    d->stopAnimation(widget);
                }

                if (doTransition) {
                    QImage startImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                    QImage endImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                    Animation *anim = d->widgetAnimation(widget);

                    QStyleOptionButton opt = *button;
                    opt.state = (QStyle::State)oldState;

                    startImage.fill(0);
                    Transition *t = new Transition;
                    t->setWidget(w);
                    QPainter startPainter(&startImage);

                    if (!anim) {
                        drawControl(element, &opt, &startPainter, 0 /* Intentional */);
                    } else {
                        anim->paint(&startPainter, &opt);
                        d->stopAnimation(widget);
                    }

                    t->setStartImage(startImage);
                    d->startAnimation(t);

                    endImage.fill(0);
                    QPainter endPainter(&endImage);
                    drawControl(element, option, &endPainter, 0 /* Intentional */);
                    t->setEndImage(endImage);
                    int duration = 0;
                    HTHEME theme = pOpenThemeData(0, L"Button");

                    int fromState = buttonStateId(oldState, BP_PUSHBUTTON);
                    int toState = buttonStateId(option->state, BP_PUSHBUTTON);
                    if (pGetThemeTransitionDuration(theme, BP_PUSHBUTTON, fromState, toState, TMT_TRANSITIONDURATIONS, &duration) == S_OK)
                        t->setDuration(duration);
                    else
                        t->setDuration(0);
                    t->setStartTime(QTime::currentTime());
                }
            }
        }
    }
    switch (element) {
    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {

            if (Animation *anim = d->widgetAnimation(widget)) {
                anim->paint(painter, option);
            } else {
                name = QLatin1String("BUTTON");
                partId = BP_PUSHBUTTON;
                bool justFlat = (btn->features & QStyleOptionButton::Flat) && !(flags & (State_On|State_Sunken));
                if (!(flags & State_Enabled) && !(btn->features & QStyleOptionButton::Flat))
                    stateId = PBS_DISABLED;
                else if (justFlat)
                    ;
                else if (flags & (State_Sunken | State_On))
                    stateId = PBS_PRESSED;
                else if (flags & State_MouseOver)
                    stateId = PBS_HOT;
                else if (btn->features & QStyleOptionButton::DefaultButton)
                    stateId = PBS_DEFAULTED;
                else
                    stateId = PBS_NORMAL;

                if (!justFlat) {

                    if (widget && (btn->features & QStyleOptionButton::DefaultButton)      &&
                        !(state & (State_Sunken | State_On)) && !(state & State_MouseOver) &&
                         (state & State_Enabled))
                        {
                        Animation *anim = d->widgetAnimation(widget);
                        if (!anim && widget) {
                            QImage startImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                            startImage.fill(0);
                            QImage alternateImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                            alternateImage.fill(0);

                            Pulse *pulse = new Pulse;
                            pulse->setWidget(const_cast<QWidget*>(widget));

                            QPainter startPainter(&startImage);
                            stateId = PBS_DEFAULTED;
                            XPThemeData theme(widget, &startPainter, name, partId, stateId, rect);
                            d->drawBackground(theme);

                            QPainter alternatePainter(&alternateImage);
                            theme.stateId = PBS_DEFAULTED_ANIMATING;
                            theme.painter = &alternatePainter;
                            d->drawBackground(theme);
                            pulse->setPrimaryImage(startImage);
                            pulse->setAlternateImage(alternateImage);
                            pulse->setStartTime(QTime::currentTime());
                            pulse->setDuration(1600);
                            d->startAnimation(pulse);
                            anim = pulse;
                        }

                        if (anim)
                            anim->paint(painter, option);
                        else {
                            XPThemeData theme(widget, painter, name, partId, stateId, rect);
                            d->drawBackground(theme);
                        }
                    }
                    else {
                        d->stopAnimation(widget);
                        XPThemeData theme(widget, painter, name, partId, stateId, rect);
                        d->drawBackground(theme);
                    }
                }
            }
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbiw = 0, mbih = 0;
                XPThemeData theme(widget, 0, QLatin1String("TOOLBAR"), TP_DROPDOWNBUTTON);
                if (theme.isValid()) {
                    SIZE size;
                    if (pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size) == S_OK) {
                        mbiw = size.cx;
                        mbih = size.cy;
                    }
                }
                QRect ir = subElementRect(SE_PushButtonContents, option, 0);
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QStyle::visualRect(option->direction, option->rect,
                                                QRect(ir.right() - mbiw - 2, (option->rect.height()/2) - (mbih/2),
                                                      mbiw + 1, mbih + 1));
                drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            }
            return;
        }
        break;

    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *bar
                = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            int stateId = MBI_NORMAL;
            if (disabled)
                stateId = MBI_DISABLED;
            bool isIndeterminate = (bar->minimum == 0 && bar->maximum == 0);
            bool vertical = false;
            bool inverted = false;
            if (const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option)) {
                vertical = (pb2->orientation == Qt::Vertical);
                inverted = pb2->invertedAppearance;
            }

            if (const QProgressBar *progressbar = qobject_cast<const QProgressBar *>(widget)) {
                if (progressbar->value() < progressbar->maximum() && (progressbar->value() > 0 || isIndeterminate) && d->transitionsEnabled()) {
                    if (!d->widgetAnimation(progressbar)) {
                        Animation *a = new Animation;
                        a->setWidget(const_cast<QWidget*>(widget));
                        a->setStartTime(QTime::currentTime());
                        d->startAnimation(a);
                    }
                } else {
                    d->stopAnimation(progressbar);
                }
            }

            XPThemeData theme(widget, painter, QLatin1String("PROGRESS"), vertical ? PP_FILLVERT : PP_FILL);
            theme.rect = option->rect;

            if (isIndeterminate) {
                if (Animation *a = d->widgetAnimation(widget)) {
                    QTime current = QTime::currentTime();
                    int timeDiff = a->startTime().msecsTo(current);
                    int loopTime = 2200;
                    float ps = (timeDiff%loopTime)/(float)loopTime;
                    painter->save();
                    painter->setClipRect(theme.rect);
                    QRect barRect;
                    QRect middleRect;

                    if (vertical) {
                        theme.rect = QRect(theme.rect.left(),
                                           theme.rect.top() - theme.rect.height() + 2*ps*theme.rect.height(),
                                           theme.rect.width(), (int)theme.rect.height()/2);
                    } else {
                        theme.rect = QRect(theme.rect.left() - theme.rect.width() + 2*ps*theme.rect.width(),
                                           theme.rect.top(), (int)theme.rect.width()/2, theme.rect.height());
                    }
                    theme.partId = vertical ? PP_FILLVERT : PP_FILL;
                    d->drawBackground(theme);
                    painter->restore();
                }
            }
            else {
                bool reverse = bar->direction == Qt::LeftToRight && inverted || bar->direction == Qt::RightToLeft && !inverted;
                int progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar

                if (vertical) {
                    int maxHeight = option->rect.height();
                    int minHeight = 0;
                    int height = isIndeterminate ? maxHeight: qMax(int((((progress - bar->minimum))
                                                             / double(bar->maximum - bar->minimum)) * maxHeight), minHeight);
                    theme.rect.setHeight(height);
                    if (!inverted)
                        theme.rect.moveTop(rect.height() - theme.rect.height());
                }
                else{
                    int maxWidth = option->rect.width();
                    int minWidth = 0;
                    int width = isIndeterminate ? maxWidth : qMax(int((((progress - bar->minimum))
                                                             / double(bar->maximum - bar->minimum)) * maxWidth), minWidth);
                    theme.rect.setWidth(width);
                    if (reverse) {
                        theme.rect = QStyle::visualRect(bar->direction, option->rect, theme.rect);
                        theme.rect.moveLeft(rect.width() - theme.rect.width());
                    }
                }
                d->drawBackground(theme);

                if (Animation *a = d->widgetAnimation(widget)) {
                    QTime current = QTime::currentTime();
                    int timeDiff = a->startTime().msecsTo(current);
                    theme.partId = vertical ? PP_MOVEOVERLAYVERT : PP_MOVEOVERLAY;

                    int loopTime = 1600;
                    float ps = (timeDiff%loopTime)/(float)loopTime;
                    painter->save();
                    painter->setClipRect(theme.rect);
                    if (vertical) {
                        theme.rect = QRect(theme.rect.left(),
                                           inverted ? theme.rect.top() - theme.rect.height() + 3*ps*theme.rect.height() :
                                                     theme.rect.top() + theme.rect.height() - 3*ps*theme.rect.height(),
                                           theme.rect.width(), theme.rect.height());
                    } else {
                        theme.rect = QRect(reverse ? theme.rect.left() + theme.rect.width() - 3*ps*theme.rect.width():
                                           theme.rect.left() - theme.rect.width() + 3*ps*theme.rect.width(),
                                           theme.rect.top(), theme.rect.width(), theme.rect.height());
                    }
                    d->drawBackground(theme);
                    painter->restore();
                }
            }
        }
        break;
    case CE_MenuBarItem:
        {

        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            if (mbi->menuItemType == QStyleOptionMenuItem::DefaultItem)
                break;

            QPalette::ColorRole textRole = disabled ? QPalette::Text : QPalette::ButtonText;
            QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal);

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            //The rect adjustment is a workaround for the menu not really filling it's background.
            XPThemeData theme(widget, painter, QLatin1String("MENU"), MENU_BARBACKGROUND, 0, option->rect.adjusted(-1, 1 , 2, 1));
            d->drawBackground(theme);

            int stateId = MBI_NORMAL;
            if (disabled)
                stateId = MBI_DISABLED;
            else if (pressed)
                stateId = MBI_PUSHED;
            else if (selected)
                stateId = MBI_HOT;

            XPThemeData theme2(widget, painter, QLatin1String("MENU"), MENU_BARITEM, stateId, option->rect);
            d->drawBackground(theme2);

            if (!pix.isNull())
                drawItemPixmap(painter, mbi->rect, alignment, pix);
            else
                drawItemText(painter, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
        }
    }
    break;
#ifndef QT_NO_MENU
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax(menuitem->maxIconWidth, 28);
            QColor darkLine = option->palette.background().color().darker(108);
            QColor lightLine = option->palette.background().color().lighter(107);
            QRect rect = option->rect;
            QStyleOptionMenuItem mbiCopy = *menuitem;

            //draw vertical menu line
            QPoint p1 = QStyle::visualPos(option->direction, menuitem->rect, QPoint(checkcol, rect.top()));
            QPoint p2 = QStyle::visualPos(option->direction, menuitem->rect, QPoint(checkcol, rect.bottom()));
            QRect gutterRect(p1.x(), p1.y(), 3, p2.y() - p1.y() + 1);
            XPThemeData theme2(widget, painter, QLatin1String("MENU"), MENU_POPUPGUTTER, stateId, gutterRect);
            d->drawBackground(theme2);

            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable
                            ? menuitem->checked : false;
            bool act = menuitem->state & State_Selected;

            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                int yoff = y-2 + h / 2;
                QPoint p1 = QPoint(x + checkcol, yoff);
                QPoint p2 = QPoint(x + w + 6 , yoff);
                int stateId = stateId = MBI_HOT;
                QRect subRect(p1.x(), p1.y(), p2.x() - p1.x(), 6);
                subRect  = QStyle::visualRect(option->direction, option->rect, subRect );
                XPThemeData theme2(widget, painter, QLatin1String("MENU"), MENU_POPUPSEPARATOR, stateId, subRect);
                d->drawBackground(theme2);
                return;
            }

            QRect vCheckRect = visualRect(option->direction, menuitem->rect, QRect(menuitem->rect.x() + 1,
                                          menuitem->rect.y(), checkcol - 7, menuitem->rect.height()));

            if (act) {
                int stateId = stateId = MBI_HOT;
                XPThemeData theme2(widget, painter, QLatin1String("MENU"), MENU_POPUPITEM, stateId, option->rect);
                d->drawBackground(theme2);
            }

            if (checked) {
                XPThemeData theme(widget, painter, QLatin1String("MENU"), MENU_POPUPCHECKBACKGROUND,
                                  menuitem->icon.isNull() ? MBI_HOT : MBI_PUSHED, vCheckRect);
                d->drawBackground(theme);
                theme.partId = MENU_POPUPCHECK;
                bool bullet = menuitem->checkType & QStyleOptionMenuItem::Exclusive;
                if (dis)
                    theme.stateId = bullet ? MC_BULLETDISABLED: MC_CHECKMARKDISABLED;
                else
                    theme.stateId = bullet ? MC_BULLETNORMAL: MC_CHECKMARKNORMAL;
                d->drawBackground(theme);
            }

            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checked)
                    pixmap = menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On);
                else
                    pixmap = menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                painter->setPen(menuitem->palette.text().color());
                painter->drawPixmap(pmr.topLeft(), pixmap);
            }

            painter->setPen(menuitem->palette.buttonText().color());

            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                painter->setPen(discol);
            }

            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = visualRect(option->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {    // draw text
                painter->save();
                int t = s.indexOf(QLatin1Char('\t'));
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(option->direction, menuitem->rect,
                    QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
                    painter->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                painter->setFont(font);
                painter->setPen(discol);
                painter->drawText(vTextRect, text_flags, s.left(t));
                painter->restore();
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = (option->direction == Qt::RightToLeft) ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - windowsArrowHMargin - windowsItemFrame - dim;
                QRect  vSubMenuRect = visualRect(option->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                drawPrimitive(arrow, &newMI, painter, widget);
            }
        }
        break;
#endif // QT_NO_MENU
    case CE_MenuBarEmptyArea:
        {
            int stateId = MBI_NORMAL;
            if (!(state & State_Enabled))
                stateId = MBI_DISABLED;
            XPThemeData theme(widget, painter, QLatin1String("MENU"), MENU_BARBACKGROUND, stateId, option->rect);
            d->drawBackground(theme);
        }
        break;
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(option)) {
            QPalette pal = option->palette;
            pal.setColor(QPalette::Dark, option->palette.background().color().darker(130));
            QStyleOptionToolBar copyOpt = *toolbar;
            copyOpt.palette = pal;
            QWindowsStyle::drawControl(element, &copyOpt, painter, widget);
        }
        break;
    case CE_DockWidgetTitle:
        if (const QDockWidget *dockWidget = qobject_cast<const QDockWidget *>(widget)) {
            QRect rect = option->rect;
            if (dockWidget->isFloating()) {
                QWindowsXPStyle::drawControl(element, option, painter, widget);
                break; //otherwise fall through
            }
            painter->setBrush(option->palette.background().color().darker(110));
            painter->setPen(option->palette.background().color().darker(130));
            painter->drawRect(rect.adjusted(0, 1, -1, -3));

            if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {

            int buttonMargin = 4;
            int mw = pixelMetric(QStyle::PM_DockWidgetTitleMargin, dwOpt, widget);
            int fw = pixelMetric(PM_DockWidgetFrameWidth, dwOpt, widget);
            const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget);
            bool isFloating = dw != 0 && dw->isFloating();

            QRect r = option->rect.adjusted(0, 2, -1, -3);
            QRect titleRect = r;

            if (dwOpt->closable) {
                QPixmap pm = standardPixmap(QStyle::SP_TitleBarCloseButton, dwOpt, widget);
                titleRect.adjust(0, 0, -pm.size().width() - mw - buttonMargin, 0);
            }

            if (dwOpt->floatable) {
                QPixmap pm = standardPixmap(QStyle::SP_TitleBarMaxButton, dwOpt, widget);
                titleRect.adjust(0, 0, -pm.size().width() - mw - buttonMargin, 0);
            }

            if (isFloating) {
                titleRect.adjust(0, -fw, 0, 0);
                if (widget != 0 && widget->windowIcon().cacheKey() != qApp->windowIcon().cacheKey())
                    titleRect.adjust(titleRect.height() + mw, 0, 0, 0);
            } else {
                titleRect.adjust(mw, 0, 0, 0);
                if (!dwOpt->floatable && !dwOpt->closable)
                    titleRect.adjust(0, 0, -mw, 0);
            }
            titleRect = visualRect(dwOpt->direction, r, titleRect);
                 if (!dwOpt->title.isEmpty()) {
                    QString titleText = painter->fontMetrics().elidedText(dwOpt->title, Qt::ElideRight, titleRect.width());
                    const int indent = painter->fontMetrics().descent();
                    drawItemText(painter, rect.adjusted(indent + 1, 1, -indent - 1, -1),
                                Qt::AlignLeft | Qt::AlignVCenter, dwOpt->palette,
                                dwOpt->state & State_Enabled, titleText,
                                QPalette::WindowText);
                }
            }

            break;
        }
    default:
        QWindowsXPStyle::drawControl(element, option, painter, widget);
        break;
    }
}

/*!
  \reimp

  see drawPrimitive for comments on the animation support

 */
void QWindowsVistaStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         QPainter *painter, const QWidget *widget) const
{
    QWindowsVistaStylePrivate *d = const_cast<QWindowsVistaStylePrivate*>(d_func());
    if (!QWindowsVistaStylePrivate::useVista()) {
        QWindowsStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    State state = option->state;
    SubControls sub = option->subControls;
    QRect r = option->rect;

    int partId = 0;
    int stateId = 0;

    State flags = option->state;
    if (widget && widget->testAttribute(Qt::WA_UnderMouse) && widget->isActiveWindow())
        flags |= State_MouseOver;

    if (d->transitionsEnabled() && widget) {
        if ((qobject_cast<const QScrollBar *>(widget) && control == CC_ScrollBar)||
            (qobject_cast<const QAbstractSpinBox*>(widget) && control == CC_SpinBox)    ||
            (qobject_cast<const QComboBox*>(widget) && control == CC_ComboBox))
        {
            QWidget *w = const_cast<QWidget *> (widget);

            int oldState = w->property("qt_stylestate").toInt();
            int oldSubControls = w->property("qt_stylecontrols").toInt();
            QRect oldRect = w->property("qt_stylerect").toRect();
            w->setProperty("qt_stylestate", (int)option->state);
            w->setProperty("qt_stylecontrols", (int)option->activeSubControls);
            w->setProperty("qt_stylerect", w->rect());

            bool doTransition = ((state & State_Sunken)     != (oldState & State_Sunken) ||
                                 (state & State_On)         != (oldState & State_On)     ||
                                 (state & State_MouseOver)  != (oldState & State_MouseOver) ||
                                  oldSubControls            != option->activeSubControls);


            if (oldRect != option->rect) {
                doTransition = false;
                d->stopAnimation(widget);
            }

            if (const QStyleOptionSlider *slider= qstyleoption_cast<const QStyleOptionSlider *>(option)) {
                QRect oldSliderPos = w->property("qt_stylesliderpos").toRect();
                QRect currentPos = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                w->setProperty("qt_stylesliderpos", currentPos);
                if (oldSliderPos != currentPos) {
                    doTransition = false;
                    d->stopAnimation(widget);
                }
            }
            else if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
                QString oldText = w->property("qt_styletext").toString();
                w->setProperty("qt_styletext", combo->currentText);
                doTransition = false;
                if ((state & State_Sunken) != (oldState & State_Sunken) ||
                    (state & State_On) != (oldState & State_On) ||
                    (state & State_MouseOver) != (oldState & State_MouseOver))
                    doTransition = true;
            }

            if (doTransition) {
                QImage startImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                QImage endImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
                Animation *anim = d->widgetAnimation(widget);
                Transition *t = new Transition;
                t->setWidget(w);
                if (!anim) {
                    if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
                        //Combo boxes are special cased to avoid cleartype issues
                        startImage.fill(0);
                        QPainter startPainter(&startImage);
                        QStyleOptionComboBox startCombo = *combo;
                        startCombo.state = (QStyle::State)oldState;
                        startCombo.activeSubControls = (QStyle::SubControl)oldSubControls;
                        drawComplexControl(control, &startCombo, &startPainter, 0 /* Intentional */);
                        t->setStartImage(startImage);
                    } else {
                        t->setStartImage(QPixmap::grabWindow(widget->winId(), 0, 0 , option->rect.width(), option->rect.height()).toImage());
                    }
                } else {
                    startImage.fill(0);
                    QPainter startPainter(&startImage);
                    anim->paint(&startPainter, option);
                    t->setStartImage(startImage);
                }
                d->stopAnimation(w);
                d->startAnimation(t);
                endImage.fill(0);
                QPainter endPainter(&endImage);
                drawComplexControl(control, option, &endPainter, 0 /* Intentional */);
                t->setEndImage(endImage);
                t->setStartTime(QTime::currentTime());

                if (option->state & State_MouseOver || option->state & State_Sunken)
                    t->setDuration(150);
                else
                    t->setDuration(500);
            }

            if (Animation *anim = d->widgetAnimation(widget)) {
                anim->paint(painter, option);
                return;
            }

        }
    }

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            if (cmb->editable) {
                if (sub & SC_ComboBoxEditField) {
                    partId = EP_EDITBORDER_NOSCROLL;
                    if (!(flags & State_Enabled))
                        stateId = ETS_DISABLED;
                    else if (flags & State_MouseOver)
                        stateId = ETS_HOT;
                    else if (flags & State_HasFocus)
                        stateId = ETS_FOCUSED;
                    else
                        stateId = ETS_NORMAL;

                    XPThemeData theme(widget, painter, QLatin1String("EDIT"), partId, stateId, r);

                    d->drawBackground(theme);
                }
                if (sub & SC_ComboBoxArrow) {
                    QRect subRect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
                    XPThemeData theme(widget, painter, QLatin1String("COMBOBOX"));
                    theme.rect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
                    partId = option->direction == Qt::RightToLeft ? CP_DROPDOWNBUTTONLEFT : CP_DROPDOWNBUTTONRIGHT;

                    if (!(cmb->state & State_Enabled))
                        stateId = CBXS_DISABLED;
                    else if (cmb->state & State_Sunken || cmb->state & State_On)
                        stateId = CBXS_PRESSED;
                    else if (cmb->state & State_MouseOver && option->activeSubControls & SC_ComboBoxArrow)
                        stateId = CBXS_HOT;
                    else
                        stateId = CBXS_NORMAL;

                    theme.partId = partId;
                    theme.stateId = stateId;
                    d->drawBackground(theme);
                }

            } else {
                if (sub & SC_ComboBoxFrame) {
                    QStyleOptionButton btn;
                    btn.palette = option->palette;
                    btn.rect = option->rect.adjusted(-1, -1, 1, 1);
                    btn.state = option->state;
                    btn.features = QStyleOptionButton::HasMenu;
                    drawControl(QStyle::CE_PushButton, &btn, painter, widget);
                }
            }
       }
       break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            XPThemeData theme(widget, painter, QLatin1String("SCROLLBAR"));

            bool maxedOut = (scrollbar->maximum == scrollbar->minimum);
            if (maxedOut)
                flags &= ~State_Enabled;

            bool isHorz = flags & State_Horizontal;
            bool isRTL  = option->direction == Qt::RightToLeft;
            if (sub & SC_ScrollBarAddLine) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget);
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTDISABLED : ABS_RIGHTDISABLED) : ABS_DOWNDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTPRESSED : ABS_RIGHTPRESSED) : ABS_DOWNPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTHOT : ABS_RIGHTHOT) : ABS_DOWNHOT);
                else if (scrollbar->state & State_MouseOver)
                    stateId = (isHorz ? (isRTL ? ABS_LEFTHOVER : ABS_RIGHTHOVER) : ABS_DOWNHOVER);
                else
                    stateId = (isHorz ? (isRTL ? ABS_LEFTNORMAL : ABS_RIGHTNORMAL) : ABS_DOWNNORMAL);
                theme.partId = partId;
                theme.stateId = stateId;
                d->drawBackground(theme);
            }
            if (sub & SC_ScrollBarSubLine) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget);
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTDISABLED : ABS_LEFTDISABLED) : ABS_UPDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTPRESSED : ABS_LEFTPRESSED) : ABS_UPPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTHOT : ABS_LEFTHOT) : ABS_UPHOT);
                else if (scrollbar->state & State_MouseOver)
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTHOVER : ABS_LEFTHOVER) : ABS_UPHOVER);
                else
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTNORMAL : ABS_LEFTNORMAL) : ABS_UPNORMAL);
                theme.partId = partId;
                theme.stateId = stateId;
                d->drawBackground(theme);
            }
            if (maxedOut) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                theme.rect = theme.rect.united(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget));
                theme.rect = theme.rect.united(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget));
                partId = scrollbar->direction == Qt::Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                stateId = SCRBS_DISABLED;
                theme.partId = partId;
                theme.stateId = stateId;
                d->drawBackground(theme);
            } else {
                if (sub & SC_ScrollBarSubPage) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget);
                    partId = flags & State_Horizontal ? SBP_UPPERTRACKHORZ : SBP_UPPERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    d->drawBackground(theme);
                }
                if (sub & SC_ScrollBarAddPage) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget);
                    partId = flags & State_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    d->drawBackground(theme);
                }
                if (sub & SC_ScrollBarSlider) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else if (option->state & State_MouseOver)
                        stateId = SCRBS_HOVER;
                    else
                        stateId = SCRBS_NORMAL;

                    // Draw handle
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                    theme.partId = flags & State_Horizontal ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT;
                    theme.stateId = stateId;
                    d->drawBackground(theme);

                    // Calculate rect of gripper
                    const int swidth = theme.rect.width();
                    const int sheight = theme.rect.height();

                    MARGINS contentsMargin;
                    RECT rect = theme.toRECT(theme.rect);
                    pGetThemeMargins(theme.handle(), 0, theme.partId, theme.stateId, TMT_SIZINGMARGINS, &rect, &contentsMargin);

                    SIZE size;
                    theme.partId = flags & State_Horizontal ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT;
                    pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                    int gw = size.cx, gh = size.cy;


                    QRect gripperBounds;
                    if (flags & State_Horizontal && ((swidth - contentsMargin.cxLeftWidth - contentsMargin.cxRightWidth) > gw)) {
                        gripperBounds.setLeft(theme.rect.left() + swidth/2 - gw/2);
                        gripperBounds.setTop(theme.rect.top() + sheight/2 - gh/2);
                        gripperBounds.setWidth(gw);
                        gripperBounds.setHeight(gh);
                    } else if ((sheight - contentsMargin.cyTopHeight - contentsMargin.cyBottomHeight) > gh) {
                        gripperBounds.setLeft(theme.rect.left() + swidth/2 - gw/2);
                        gripperBounds.setTop(theme.rect.top() + sheight/2 - gh/2);
                        gripperBounds.setWidth(gw);
                        gripperBounds.setHeight(gh);
                    }

                    // Draw gripper if there is enough space
                    if (!gripperBounds.isEmpty() && flags & State_Enabled) {
                        painter->save();
                        XPThemeData grippBackground = theme;
                        grippBackground.partId = flags & State_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                        theme.rect = gripperBounds;
                        painter->setClipRegion(d->region(theme));// Only change inside the region of the gripper
                        d->drawBackground(grippBackground);// The gutter is the grippers background
                        d->drawBackground(theme);          // Transparent gripper ontop of background
                        painter->restore();
                    }
                }
            }
        }
        break;


    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            XPThemeData theme(widget, painter, QLatin1String("SPIN"));
            if (sb->frame && (sub & SC_SpinBoxFrame)) {
                partId = EP_EDITBORDER_NOSCROLL;
                if (!(flags & State_Enabled))
                    stateId = ETS_DISABLED;
                else if (flags & State_MouseOver)
                    stateId = ETS_HOT;
                else if (flags & State_HasFocus)
                    stateId = ETS_SELECTED;
                else
                    stateId = ETS_NORMAL;

                XPThemeData ftheme(widget, painter, QLatin1String("EDIT"), partId, stateId, r);
                ftheme.noContent = true;
                d->drawBackground(ftheme);
            }
            if (sub & SC_SpinBoxUp) {
                theme.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget).adjusted(0, 0, 0, 1);
                partId = SPNP_UP;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) || !(flags & State_Enabled))
                    stateId = UPS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken))
                    stateId = UPS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_MouseOver))
                    stateId = UPS_HOT;
                else
                    stateId = UPS_NORMAL;
                theme.partId = partId;
                theme.stateId = stateId;
                d->drawBackground(theme);
            }
            if (sub & SC_SpinBoxDown) {
                theme.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);
                partId = SPNP_DOWN;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) || !(flags & State_Enabled))
                    stateId = DNS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken))
                    stateId = DNS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_MouseOver))
                    stateId = DNS_HOT;
                else
                    stateId = DNS_NORMAL;
                theme.partId = partId;
                theme.stateId = stateId;
                d->drawBackground(theme);
            }
        }
        break;
    default:
        QWindowsXPStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

/*!
 \reimp
 */
QSize QWindowsVistaStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                        const QSize &size, const QWidget *widget) const
{
    if (!QWindowsVistaStylePrivate::useVista())
        return QWindowsStyle::sizeFromContents(type, option, size, widget);

    QSize sz(size);

    QSize newSize = QWindowsXPStyle::sizeFromContents(type, option, size, widget);
    switch (type) {
    case CT_LineEdit:
    case CT_ComboBox:
        {
            HTHEME theme = pOpenThemeData(0, L"Button");
            MARGINS borderSize;
            if (theme) {
                int result = pGetThemeMargins(theme,
                                              NULL,
                                              BP_PUSHBUTTON,
                                              PBS_NORMAL,
                                              TMT_CONTENTMARGINS,
                                              NULL,
                                              &borderSize);
                if (result == S_OK) {
                    sz += QSize(borderSize.cxLeftWidth + borderSize.cxRightWidth - 2,
                                borderSize.cyBottomHeight + borderSize.cyTopHeight - 2);
                }
                sz += QSize(23, 0); //arrow button
            }
        }
        return sz;
    case CT_MenuItem:
        return QWindowsStyle::sizeFromContents(type, option, size, widget);
#ifndef QT_NO_MENUBAR
    case CT_MenuBarItem:
        if (!sz.isEmpty())
            sz += QSize(windowsItemHMargin * 5 + 1, 5);
            return sz;
        break;
#endif
    case CT_SpinBox:
        {
            //Spinbox adds frame twice
            sz = QWindowsStyle::sizeFromContents(type, option, size, widget);
            int border = pixelMetric(PM_SpinBoxFrameWidth, option, widget);
            sz -= QSize(2*border, 2*border);
        }
        return sz;
    default:
        break;
    }
    return newSize;
}

/*!
 \reimp
 */
QRect QWindowsVistaStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
   if (!QWindowsVistaStylePrivate::useVista())
        return QWindowsStyle::subElementRect(element, option, widget);

   QRect rect = QWindowsXPStyle::subElementRect(element, option, widget);
    switch (element) {

    //### backport this pushbutton content fix to XP
    //the original code required a widget to function.
    case SE_PushButtonContents:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            MARGINS borderSize;
            HTHEME theme = pOpenThemeData(widget ? QWindowsVistaStylePrivate::winId(widget) : 0, L"Button");
            if (theme) {
                int stateId;
                if (!(option->state & State_Enabled))
                    stateId = PBS_DISABLED;
                else if (option->state & State_Sunken)
                    stateId = PBS_PRESSED;
                else if (option->state & State_MouseOver)
                    stateId = PBS_HOT;
                else if (btn->features & QStyleOptionButton::DefaultButton)
                    stateId = PBS_DEFAULTED;
                else
                    stateId = PBS_NORMAL;

                int border = pixelMetric(PM_DefaultFrameWidth, btn, widget);
                rect = option->rect.adjusted(border, border, -border, -border);

                int result = pGetThemeMargins(theme,
                                              NULL,
                                              BP_PUSHBUTTON,
                                              stateId,
                                              TMT_CONTENTMARGINS,
                                              NULL,
                                              &borderSize);

                if (result == S_OK) {
                    rect.adjust(borderSize.cxLeftWidth, borderSize.cyTopHeight,
                                -borderSize.cxRightWidth, -borderSize.cyBottomHeight);
                    rect = visualRect(option->direction, option->rect, rect);
                }
            }
        }
        break;

    default:
        break;
    }
    return rect;
}


/*
  This function is used by subControlRect to check if a button
  should be drawn for the given subControl given a set of window flags.
*/
static bool buttonVisible(const QStyle::SubControl sc, const QStyleOptionTitleBar *tb){

    bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
    bool isMaximized = tb->titleBarState & Qt::WindowMaximized;
    const uint flags = tb->titleBarFlags;
    bool retVal = false;
    switch (sc) {
    case QStyle::SC_TitleBarContextHelpButton:
        if (flags & Qt::WindowContextHelpButtonHint)
            retVal = true;
        break;
    case QStyle::SC_TitleBarMinButton:
        if (!isMinimized && (flags & Qt::WindowMinimizeButtonHint))
            retVal = true;
        break;
    case QStyle::SC_TitleBarNormalButton:
        if (isMinimized && (flags & Qt::WindowMinimizeButtonHint))
            retVal = true;
        else if (isMaximized && (flags & Qt::WindowMaximizeButtonHint))
            retVal = true;
        break;
    case QStyle::SC_TitleBarMaxButton:
        if (!isMaximized && (flags & Qt::WindowMaximizeButtonHint))
            retVal = true;
        break;
    case QStyle::SC_TitleBarShadeButton:
        if (!isMinimized &&  flags & Qt::WindowShadeButtonHint)
            retVal = true;
        break;
    case QStyle::SC_TitleBarUnshadeButton:
        if (isMinimized && flags & Qt::WindowShadeButtonHint)
            retVal = true;
        break;
    case QStyle::SC_TitleBarCloseButton:
        if (flags & Qt::WindowSystemMenuHint)
            retVal = true;
        break;
    case QStyle::SC_TitleBarSysMenu:
        if (flags & Qt::WindowSystemMenuHint)
            retVal = true;
        break;
    default :
        retVal = true;
    }
    return retVal;
}


/*! \reimp */
int QWindowsVistaStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                             QStyleHintReturn *returnData) const
{
    QWindowsVistaStylePrivate *d = const_cast<QWindowsVistaStylePrivate*>(d_func());
    int ret = 0;
    switch (hint) {
    case SH_ToolTip_Mask:
        ret = true;
        if (QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData)) {
            XPThemeData themeData(widget, 0, QLatin1String("TOOLTIP"), TTP_STANDARD, TTSS_NORMAL, option->rect);
            mask->region = d->region(themeData);
        }
        break;
    default:
        ret = QWindowsXPStyle::styleHint(hint, option, widget, returnData);
        break;
    }
    return ret;
}


/*!
 \reimp
 */
QRect QWindowsVistaStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                  SubControl subControl, const QWidget *widget) const
{
   if (!QWindowsVistaStylePrivate::useVista())
        return QWindowsStyle::subControlRect(control, option, subControl, widget);

    QRect rect = QWindowsXPStyle::subControlRect(control, option, subControl, widget);
    switch (control) {
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int x = cb->rect.x(),
                y = cb->rect.y(),
                wi = cb->rect.width(),
                he = cb->rect.height();
            int xpos = x;
            int margin = cb->frame ? 3 : 0;
            int bmarg = cb->frame ? 2 : 0;
            xpos += wi - bmarg - 16;

            switch (subControl) {
            case SC_ComboBoxFrame:
                rect = cb->rect;
                break;
            case SC_ComboBoxArrow:
                rect.setRect(cb->editable ? xpos : 0, y , wi - xpos, he);
                break;
            case SC_ComboBoxEditField:
                rect.setRect(x + margin, y + margin, wi - 2 * margin - 16, he - 2 * margin);
                break;
            case SC_ComboBoxListBoxPopup:
                rect = cb->rect;
                break;
            default:
                break;
            }
            rect = visualRect(cb->direction, cb->rect, rect);
            return rect;
        }
#endif // QT_NO_COMBOBOX
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            if (!buttonVisible(subControl, tb))
                return rect;
            const bool isToolTitle = false;
            const int height = tb->rect.height();
            const int width = tb->rect.width();
            int buttonHeight = GetSystemMetrics(SM_CYSIZE) - 4;
            int buttonWidth = GetSystemMetrics(SM_CXSIZE) - 4;

            int controlTop = option->rect.bottom() - buttonHeight - 3;
            const int frameWidth = pixelMetric(PM_MDIFrameWidth, option, widget);
            const bool sysmenuHint  = (tb->titleBarFlags & Qt::WindowSystemMenuHint) != 0;
            const bool minimizeHint = (tb->titleBarFlags & Qt::WindowMinimizeButtonHint) != 0;
            const bool maximizeHint = (tb->titleBarFlags & Qt::WindowMaximizeButtonHint) != 0;
            const bool contextHint = (tb->titleBarFlags & Qt::WindowContextHelpButtonHint) != 0;
            const bool shadeHint = (tb->titleBarFlags & Qt::WindowShadeButtonHint) != 0;

            switch (subControl) {
            case SC_TitleBarLabel:
                rect = QRect(frameWidth, 0, width - (buttonWidth + frameWidth + 10), height);
                if (isToolTitle) {
                    if (sysmenuHint) {
                        rect.adjust(0, 0, -buttonWidth - 3, 0);
                    }
                    if (minimizeHint || maximizeHint)
                        rect.adjust(0, 0, -buttonWidth - 2, 0);
                } else {
                    if (sysmenuHint) {
                        const int leftOffset = height - 8;
                        rect.adjust(leftOffset, 0, 0, 4);
                    }
                    if (minimizeHint)
                        rect.adjust(0, 0, -buttonWidth - 2, 0);
                    if (maximizeHint)
                        rect.adjust(0, 0, -buttonWidth - 2, 0);
                    if (contextHint)
                        rect.adjust(0, 0, -buttonWidth - 2, 0);
                    if (shadeHint)
                        rect.adjust(0, 0, -buttonWidth - 2, 0);
                }
                rect.translate(0, 2);
                rect = visualRect(option->direction, option->rect, rect);
                break;

            case SC_TitleBarCloseButton:
                rect = QRect(width - (buttonWidth + 2) - controlTop + 4, controlTop,
                             buttonWidth, buttonHeight);
                rect = visualRect(option->direction, option->rect, rect);
                break;

            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                rect = QRect(width - ((buttonWidth + 2) * 2) - controlTop + 4, controlTop,
                             buttonWidth, buttonHeight);
                rect = visualRect(option->direction, option->rect, rect);
                break;

            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton:
                {
                    int offset = buttonWidth + 2;
                    if (!maximizeHint)
                        offset *= 2;
                    else
                        offset *= 3;
                    rect = QRect(width - offset - controlTop + 4, controlTop,
                                 buttonWidth, buttonHeight);
                    rect = visualRect(option->direction, option->rect, rect);
                }
                break;

            case SC_TitleBarSysMenu:
                {
                    const int controlTop = 6;
                    const int controlHeight = height - controlTop - 3;
                    QSize iconSize = tb->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).size();
                    if (tb->icon.isNull())
                        iconSize = QSize(controlHeight, controlHeight);
                    int hPad = (controlHeight - iconSize.height())/2;
                    int vPad = (controlHeight - iconSize.width())/2;
		            rect = QRect(frameWidth + hPad, controlTop + vPad, iconSize.width(), iconSize.height());
                    rect.translate(0, 3);
                    rect = visualRect(option->direction, option->rect, rect);
                }
                break;
            }
        }
        break;
    default:
        break;
    }
    return rect;
}

/*!
 \reimp
 */
QStyle::SubControl QWindowsVistaStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                          const QPoint &pos, const QWidget *widget) const
{
    if (!QWindowsVistaStylePrivate::useVista()) {
        return QWindowsStyle::hitTestComplexControl(control, option, pos, widget);
    }
    return QWindowsXPStyle::hitTestComplexControl(control, option, pos, widget);
}

/*!
 \reimp
 */
int QWindowsVistaStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (!QWindowsVistaStylePrivate::useVista()) {
        return QWindowsStyle::pixelMetric(metric, option, widget);
    }
    switch (metric) {

    case PM_DockWidgetTitleBarButtonMargin:
        return 5;
    case PM_ScrollBarSliderMin:
        return 18;
    default:
        break;
    }
    return QWindowsXPStyle::pixelMetric(metric, option, widget);
}

/*!
 \reimp
 */
QPalette QWindowsVistaStyle::standardPalette()
{
    return QWindowsXPStyle::standardPalette();
}

/*!
 \reimp
 */
void QWindowsVistaStyle::polish(QApplication *app)
{
    QWindowsXPStyle::polish(app);
}

/*!
 \reimp
 */
void QWindowsVistaStyle::polish(QWidget *widget)
{
    QWindowsXPStyle::polish(widget);
    if (qobject_cast<QLineEdit*>(widget))
        widget->setAttribute(Qt::WA_Hover);
    else if (qobject_cast<QGroupBox*>(widget))
        widget->setAttribute(Qt::WA_Hover);
}

/*!
 \reimp
 */
void QWindowsVistaStyle::unpolish(QWidget *widget)
{
    QWindowsXPStyle::unpolish(widget);

    QWindowsVistaStylePrivate *d = const_cast<QWindowsVistaStylePrivate*>(d_func());
    d->stopAnimation(widget);

    if (qobject_cast<QLineEdit*>(widget))
        widget->setAttribute(Qt::WA_Hover, false);
    else if (qobject_cast<QGroupBox*>(widget))
        widget->setAttribute(Qt::WA_Hover, false);
}


/*!
 \reimp
 */
void QWindowsVistaStyle::unpolish(QApplication *app)
{
    QWindowsXPStyle::unpolish(app);
}

/*!
 \reimp
 */
void QWindowsVistaStyle::polish(QPalette pal)
{
    QWindowsXPStyle::polish(pal);
}

/*!
 \reimp
 */
QPixmap QWindowsVistaStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                      const QWidget *widget) const
{
    if (!QWindowsVistaStylePrivate::useVista()) {
        return QWindowsStyle::standardPixmap(standardPixmap, option, widget);
    }
    return QWindowsXPStyle::standardPixmap(standardPixmap, option, widget);
}

void QWindowsVistaStylePrivate::timerEvent(QTimerEvent *timer)
{
    if (timer->timerId() == timerId) {
        for (int i = animations.size() - 1 ; i >= 0 ; --i) {

            if (animations[i]->widget())
                animations[i]->widget()->update();

            if (!animations[i]->widget() ||
                !animations[i]->widget()->isEnabled() ||
                !animations[i]->widget()->isVisible() ||
                animations[i]->widget()->window()->isMinimized() ||
                !animations[i]->running() ||
                !transitionsEnabled() ||
                !QWindowsVistaStylePrivate::useVista())
            {
                Animation *a = animations.takeAt(i);
                delete a;
            }
        }
        if (animations.size() == 0 && timerId >= 0) {
            killTimer(timerId);
            timerId = -1;
        }
    }
}

void QWindowsVistaStylePrivate::stopAnimation(const QWidget *w)
{
    for (int i = animations.size() - 1 ; i >= 0 ; --i) {
        if (animations[i]->widget() == w) {
            Animation *a = animations.takeAt(i);
            delete a;
            break;
        }
    }
}

void QWindowsVistaStylePrivate::startAnimation(Animation *t)
{
    for (int i = animations.size() - 1 ; i >= 0 ; --i) {
        if (animations[i]->widget() == t->widget()) {
            Animation *a = animations.takeAt(i);
            a->setRunning(false);
            break;
        }
    }
    animations.append(t);
    if (animations.size() > 0 && timerId == -1) {
        timerId = startTimer(60);
    }
}

bool QWindowsVistaStylePrivate::transitionsEnabled() const
{
    BOOL animEnabled = false;
    if (QT_WA_INLINE(SystemParametersInfo(SPI_GETCLIENTAREAANIMATION, 0, &animEnabled, 0),
                     SystemParametersInfoA(SPI_GETCLIENTAREAANIMATION, 0, &animEnabled, 0)
    ))
    {
        if (animEnabled)
            return true;
    }
    return false;
}


Animation * QWindowsVistaStylePrivate::widgetAnimation(const QWidget *widget) const
{
    if (!widget)
        return 0;
    foreach (Animation *a, animations) {
        if (a->widget() == widget)
            return a;
    }
    return 0;
}


/*! \internal
    Returns true if all the necessary theme engine symbols were
    resolved.
*/
bool QWindowsVistaStylePrivate::resolveSymbols()
{
    static bool tried = false;
    if (!tried) {
        tried = true;
        QLibrary themeLib(QLatin1String("uxtheme"));
        pGetThemePartSize       = (PtrGetThemePartSize      )themeLib.resolve("GetThemePartSize");
        pOpenThemeData          = (PtrOpenThemeData         )themeLib.resolve("OpenThemeData");
        pCloseThemeData         = (PtrCloseThemeData        )themeLib.resolve("CloseThemeData");
        pDrawThemeBackground    = (PtrDrawThemeBackground   )themeLib.resolve("DrawThemeBackground");
        pDrawThemeBackgroundEx  = (PtrDrawThemeBackgroundEx )themeLib.resolve("DrawThemeBackgroundEx");
        pGetCurrentThemeName    = (PtrGetCurrentThemeName   )themeLib.resolve("GetCurrentThemeName");
        pGetThemeBool           = (PtrGetThemeBool          )themeLib.resolve("GetThemeBool");
        pGetThemeColor          = (PtrGetThemeColor         )themeLib.resolve("GetThemeColor");
        pGetThemeEnumValue      = (PtrGetThemeEnumValue     )themeLib.resolve("GetThemeEnumValue");
        pGetThemeFilename       = (PtrGetThemeFilename      )themeLib.resolve("GetThemeFilename");
        pGetThemeFont           = (PtrGetThemeFont          )themeLib.resolve("GetThemeFont");
        pGetThemeInt            = (PtrGetThemeInt           )themeLib.resolve("GetThemeInt");
        pGetThemeIntList        = (PtrGetThemeIntList       )themeLib.resolve("GetThemeIntList");
        pGetThemeMargins        = (PtrGetThemeMargins       )themeLib.resolve("GetThemeMargins");
        pGetThemeMetric         = (PtrGetThemeMetric        )themeLib.resolve("GetThemeMetric");
        pGetThemePartSize       = (PtrGetThemePartSize      )themeLib.resolve("GetThemePartSize");
        pGetThemePosition       = (PtrGetThemePosition      )themeLib.resolve("GetThemePosition");
        pGetThemeRect           = (PtrGetThemeRect          )themeLib.resolve("GetThemeRect");
        pGetThemeString         = (PtrGetThemeString        )themeLib.resolve("GetThemeString");
        pIsAppThemed            = (PtrIsAppThemed           )themeLib.resolve("IsAppThemed");
        pGetThemeTransitionDuration = (PtrGetThemeTransitionDuration)themeLib.resolve("GetThemeTransitionDuration");
    }
    return pGetThemeTransitionDuration != 0;
}

#endif //QT_NO_WINDOWSVISTA
