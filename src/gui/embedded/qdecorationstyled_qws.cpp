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

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include "qdecorationstyled_qws.h"
#include "qstyle.h"
#include "qstyleoption.h"

#if !defined(QT_NO_QWS_DECORATION_STYLED) || defined(QT_PLUGIN)

QDecorationStyled::QDecorationStyled()
    : QDecorationDefault()
{
}

QDecorationStyled::~QDecorationStyled()
{
}

int QDecorationStyled::titleBarHeight(const QWidget *widget)
{
    QStyleOptionTitleBar opt;
    opt.subControls = QStyle::SC_TitleBarLabel
                      | QStyle::SC_TitleBarSysMenu
                      | QStyle::SC_TitleBarMinButton
                      | QStyle::SC_TitleBarMaxButton
                      | QStyle::SC_TitleBarCloseButton;
    opt.titleBarFlags = widget->windowFlags();
    opt.direction = QApplication::layoutDirection();
    opt.text = widget->windowTitle();
    opt.icon = widget->windowIcon();
    opt.rect = widget->rect();

    QStyle *style = QApplication::style();
    if (!style)
        return 18;

    return style->pixelMetric(QStyle::PM_TitleBarHeight, &opt, widget);
}

bool QDecorationStyled::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                            DecorationState state)
{
    if (decorationRegion == None)
        return false;

    bool isActive = (widget == qApp->activeWindow());
    QPalette pal = widget->palette();
    //ideally, the difference between Active and Inactive should be enough, so we shouldn't need to test this
    if (!isActive) {
        //pal.setCurrentColorGroup(QPalette::Disabled); //Can't do this either, because of palette limitations
        //copied from Q3TitleBar:
         pal.setColor(QPalette::Inactive, QPalette::Highlight,
                      pal.color(QPalette::Inactive, QPalette::Dark));
        pal.setColor(QPalette::Inactive, QPalette::Base,
                      pal.color(QPalette::Inactive, QPalette::Dark));
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                      pal.color(QPalette::Inactive, QPalette::Background));
    }

    QRegion oldClipRegion = painter->clipRegion();

    Qt::WindowFlags flags = widget->windowFlags();
    bool hasBorder = !widget->isMaximized();
    bool hasTitle = flags & Qt::WindowTitleHint;
    bool hasSysMenu = flags & Qt::WindowSystemMenuHint;
    bool hasContextHelp = flags & Qt::WindowContextHelpButtonHint;
    bool hasMinimize = flags & Qt::WindowMinimizeButtonHint;
    bool hasMaximize = flags & Qt::WindowMaximizeButtonHint;
    int  titleHeight = titleBarHeight(widget);

    bool paintAll = (DecorationRegion(decorationRegion) == All);
    bool handled = false;

    QStyle *style = QApplication::style();

    if ((paintAll || decorationRegion & Borders) && state == Normal && hasBorder) {
        painter->save();
        if (hasTitle) { // reduce flicker
            QRect rect(widget->rect());
            QRect r(rect.left(), rect.top() - titleHeight,
                    rect.width(), titleHeight);
            painter->setClipRegion(oldClipRegion - r);
        }
        QRect br = QDecoration::region(widget).boundingRect();

        QStyleOptionFrame opt;
        opt.palette = pal;
        opt.rect = br;
        opt.lineWidth = 2;
        if (isActive)
            opt.state |= QStyle::State_Active;
        painter->fillRect(br, pal.background());
        style->drawPrimitive(QStyle::PE_FrameWindow, &opt, painter, widget);
        painter->restore();

        decorationRegion &= (~Borders);
        handled |= true;
    }

    if (hasTitle) {
        painter->save();

        QStyleOptionTitleBar opt;
        opt.subControls = (decorationRegion & Title
                              ? QStyle::SC_TitleBarLabel : QStyle::SubControl(0))
                          | (decorationRegion & Menu
                              ? QStyle::SC_TitleBarSysMenu : QStyle::SubControl(0))
                          | (decorationRegion & Help
                              ? QStyle::SC_TitleBarContextHelpButton : QStyle::SubControl(0))
                          | (decorationRegion & Minimize
                              ? QStyle::SC_TitleBarMinButton : QStyle::SubControl(0))
                          | (decorationRegion & Maximize
                              ? QStyle::SC_TitleBarMaxButton : QStyle::SubControl(0))
                          | (decorationRegion & Close
                              ? QStyle::SC_TitleBarCloseButton : QStyle::SubControl(0));
        opt.titleBarFlags = widget->windowFlags();
        if (isActive)
            opt.titleBarState |= QStyle::State_Active;
        opt.text = widget->windowTitle();
        opt.palette = pal;

        opt.rect = QRect(widget->rect().x(), -titleHeight, widget->rect().width(), titleHeight);
        // If we're not painting all, then lets clip to only those who are not painted
        if (!paintAll) {
            const QRect widgetRect = widget->rect();
            QRegion newClip = opt.rect;
            if (!(decorationRegion & Menu) && hasSysMenu)
                newClip -= region(widget, widgetRect, Menu);
            if (!(decorationRegion & Title) && hasTitle)
                newClip -= region(widget, widgetRect, Title);
            if (!(decorationRegion & Help) && hasContextHelp)
                newClip -= region(widget, widgetRect, Help);
            if (!(decorationRegion & Minimize) && hasMinimize)
                newClip -= region(widget, widgetRect, Minimize);
            if (!(decorationRegion & Maximize) && hasMaximize)
                newClip -= region(widget, widgetRect, Maximize);
            if (!(decorationRegion & Close))
                newClip -= region(widget, widgetRect, Close);
            painter->setClipRegion(newClip);
        }

        if (state == Pressed)
            opt.activeSubControls = opt.subControls;

        painter->setFont(widget->font());
        style->drawComplexControl(QStyle::CC_TitleBar, &opt, painter, widget);
        painter->restore();

        decorationRegion &= ~(Title | Menu | Help | Minimize | Maximize | Close);
        handled |= true;
    }

    return handled;
}

QRegion QDecorationStyled::region(const QWidget *widget, const QRect &rect, int decorationRegion)
{
    int titleHeight = titleBarHeight(widget);
    QRect inside(rect.x(), rect.top() - titleHeight, rect.width(), titleHeight);


    Qt::WindowFlags flags = widget->windowFlags();
    bool hasSysMenu = flags & Qt::WindowSystemMenuHint;
    bool hasContextHelp = flags & Qt::WindowContextHelpButtonHint;
    bool hasMinimize = flags & Qt::WindowMinimizeButtonHint;
    bool hasMaximize = flags & Qt::WindowMaximizeButtonHint;

    QStyleOptionTitleBar opt;
    opt.subControls = QStyle::SC_TitleBarLabel
                      | QStyle::SC_TitleBarSysMenu
                      | QStyle::SC_TitleBarMinButton
                      | QStyle::SC_TitleBarMaxButton
                      | QStyle::SC_TitleBarCloseButton;
    opt.titleBarFlags = widget->windowFlags();
    opt.direction = QApplication::layoutDirection();
    opt.text = widget->windowTitle();
    opt.icon = widget->windowIcon();
    opt.rect = inside;

    QStyle *style = QApplication::style();

    QRegion region;
    switch (decorationRegion) {
        case Title:
            region =
                QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarLabel, widget));
            break;
        case Menu:
            if (hasSysMenu)
            region =
                QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarSysMenu, widget));
            break;
        case Help:
            if (hasContextHelp)
            region =
                QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarContextHelpButton, widget));
            break;
        case Minimize:
            if (hasMinimize)
            region =
                QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarMinButton, widget));
            break;
        case Normalize:
        case Maximize:
            if (hasMaximize)
            region =
                QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarMaxButton, widget));
            break;
       case Close:
            region
                = QStyle::visualRect(opt.direction, opt.rect, style->subControlRect(
                    QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarCloseButton, widget));
            break;

        default:
            region = QDecorationDefault::region(widget, rect, decorationRegion);
    }
    return region;
}

#endif // QT_NO_QWS_DECORATION_STYLED
