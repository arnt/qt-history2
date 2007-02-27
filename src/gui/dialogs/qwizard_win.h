/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#pragma once
#include <windows.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <qobject.h>
#include <qwidget.h>
#include <qabstractbutton.h>
#include <vssym32.h>
#include <QtGui/private/qwidget_p.h>

class QVistaBackButton : public QAbstractButton
{
public:
    QVistaBackButton(QWidget *widget);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

class QWizard;

class QVistaHelper : public QObject
{
    Q_OBJECT
public:
    QVistaHelper(QWizard *wizard);
    ~QVistaHelper();
    enum TitleBarChangeType { NormalTitleBar, ExtendedTitleBar };
    bool setDWMTitleBar(TitleBarChangeType type);
    void mouseEvent(QEvent *event);
    bool handleWinEvent(MSG *message, long *result);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    QVistaBackButton *backButton() const { return backButton_; }
    void setWindowPosHack();
    static bool isCompositionEnabled();
    static bool isVista();

    static int titleBarSize() { return frameSize() + captionSize(); }

    static int topPadding() { return 8; } // standard Aero (?)
    //static int topPadding() { return 0; } // tighter

    static int topOffset() { return titleBarSize() + 13; } // standard Aero
    //static int topOffset() { return 2 * topPadding(); } // nicer!(?)
    //static int topOffset() { return titleBarSize() * 4; } // extreme

private:
    static HFONT getCaptionFont(HTHEME hTheme);
    static bool drawTitleText(const QString &text, const QRect &rect, HDC hdc);
    static bool drawBlackRect(const QRect &rect, HDC hdc);

    static int frameSize() { return GetSystemMetrics(SM_CYSIZEFRAME); }
    static int captionSize() { return GetSystemMetrics(SM_CYCAPTION); }

    static int backButtonSize() { return 31; } // ### should be queried from back button itself
    static int iconSize() { return 16; } // Standard Aero
    static int padding() { return 7; } // Standard Aero
    static int leftMargin() { return backButtonSize() + padding(); }
    static int titleOffset() { return leftMargin() + iconSize() + padding(); }

    bool resolveSymbols();
    void drawTitleBar(QPainter *painter);
    void setMouseCursor(QPoint pos);
    void collapseTopFrameStrut();
    bool winEvent(MSG *message, long *result);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

    static bool is_vista;
    enum Changes { resizeTop, movePosition, noChange } change;
    QPoint pressedPos;
    bool pressed;
    QRect rtTop;
    QRect rtTitle;
    QWizard *wizard;
    QVistaBackButton *backButton_;
};
