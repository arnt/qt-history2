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

#include "qcompactstyle.h"

#if !defined(QT_NO_STYLE_COMPACT) || defined(QT_PLUGIN)

#include "qfontmetrics.h"
#include "qpalette.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qmenudata.h"
#include "q3popupmenu.h"

QCompactStyle::QCompactStyle()
{
}

/*! \reimp */
int QCompactStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                               const QWidget *widget) const
{
    int ret;
    switch (metric) {
    case PM_ButtonMargin:
        ret = 2;
        break;
        // tws - I added this in to stop this "Windows Scroll behaivor."  Remove it
        // if you don't want it.
    case PM_MaximumDragDistance:
        ret = -1;
        break;
    default:
        ret = QWindowsStyle::pixelMetric(metric, option, widget);
        break;
    }
    return ret;
}

static const int motifItemFrame                = 0;        // menu item frame width
static const int motifSepHeight                = 2;        // separator item height
static const int motifItemHMargin        = 1;        // menu item hor text margin
static const int motifItemVMargin        = 2;        // menu item ver text margin
static const int motifArrowHMargin        = 0;        // arrow horizontal margin
static const int motifTabSpacing        = 4;        // space between text and tab
static const int motifCheckMarkHMargin        = 1;        // horiz. margins of check mark
static const int windowsRightBorder        = 8;    // right border on windows
static const int windowsCheckMarkWidth  = 2;    // checkmarks width on windows

static int extraPopupMenuItemWidth(bool checkable, int maxpmw, Q3MenuItem* mi, const QFontMetrics& /*fm*/)
{
    int w = 2*motifItemHMargin + 2*motifItemFrame; // a little bit of border can never harm

    if (mi->isSeparator())
        return 10; // arbitrary
    else if (mi->pixmap())
        w += mi->pixmap()->width();        // pixmap only

    if (!mi->text().isNull()) {
        if (mi->text().find('\t') >= 0)        // string contains tab
            w += motifTabSpacing;
    }

    if (maxpmw) { // we have icons
        w += maxpmw;
        w += 6; // add a little extra border around the icon
    }

    if (checkable && maxpmw < windowsCheckMarkWidth) {
        w += windowsCheckMarkWidth - maxpmw; // space for the checkmarks
    }

    if (maxpmw > 0 || checkable) // we have a check-column (icons or checkmarks)
        w += motifCheckMarkHMargin; // add space to separate the columns

    w += windowsRightBorder; // windows has a strange wide border on the right side

    return w;
}

static int popupMenuItemHeight(bool /*checkable*/, Q3MenuItem* mi, const QFontMetrics& fm)
{
    int h = 0;
    if (mi->isSeparator())                        // separator height
        h = motifSepHeight;
    else if (mi->pixmap())                // pixmap height
        h = mi->pixmap()->height() + 2*motifItemFrame;
    else                                        // text height
        h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame - 1;

    if (!mi->isSeparator() && mi->iconSet() != 0) {
        h = qMax(h, mi->iconSet()->pixmap(Qt::SmallIconSize, QIcon::Normal).height() + 2*motifItemFrame);
    }
    if (mi->custom())
        h = qMax(h, mi->custom()->sizeHint().height() + 2*motifItemVMargin + 2*motifItemFrame) - 1;
    return h;
}

void drawPopupMenuItem(QPainter* p, bool checkable,
                        int maxpmw, int tab, Q3MenuItem* mi,
                        const QPalette& pal, bool act,
                        bool enabled,
                        int x, int y, int w, int h)
{

}

/*! \overload */
void QCompactStyle::drawControl(ControlElement element, QPainter *p, const QWidget *widget,
                                 const QRect &r, const QPalette &pal,
                                 SFlags flags, const Q3StyleOption& opt)
{
    switch (element) {
#ifdef QT3_SUPPORT
    case CE_Q3PopupMenuItem:
        {
            if (! widget || opt.isDefault())
                break;

            const Q3PopupMenu *popupmenu = (const Q3PopupMenu *) widget;
            Q3MenuItem *mi = opt.menuItem();
            if (!mi)
                break;

            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = !(flags & Style_Enabled);
            bool checkable = popupmenu->isCheckable();
            bool act = flags & Style_Active;
            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, 8); // space for the checkmarks

            int checkcol          =     maxpmw;

            if (mi && mi->isSeparator()) {                        // draw separator
                p->setPen(pal.dark());
                p->drawLine(x, y, x+w, y);
                p->setPen(pal.light());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            QBrush fill = act? pal.brush(QPalette::Highlight) :
                                    pal.brush(QPalette::Button);
            p->fillRect(x, y, w, h, fill);

            if (!mi)
                return;

            if (mi->isChecked()) {
                if (act && !dis) {
                    qDrawShadePanel(p, x, y, checkcol, h,
                                     pal, true, 1, &pal.brush(QPalette::Button));
                } else {
                    qDrawShadePanel(p, x, y, checkcol, h,
                                     pal, true, 1, &pal.brush(QPalette::Midlight));
                }
            } else if (!act) {
                p->fillRect(x, y, checkcol , h,
                            pal.brush(QPalette::Button));
            }

            if (mi->iconSet()) {                // draw icon
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if (act && !dis) {
                    if (!mi->isChecked())
                        qDrawShadePanel(p, x, y, checkcol, h, pal, false,  1, &pal.brush(QPalette::Button));
                }
                QRect cr(x, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

                QBrush fill = act? pal.brush(QPalette::Highlight) :
                                      pal.brush(QPalette::Button);
                p->fillRect(x+checkcol + 1, y, w - checkcol - 1, h, fill);
            } else  if (checkable) {        // just "checking"...
                int mw = checkcol + motifItemFrame;
                int mh = h - 2*motifItemFrame;
                if (mi->isChecked()) {

                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    drawPrimitive(PE_CheckMark, p, QRect(x + motifItemFrame + 2, y + motifItemFrame,
                                    mw, mh), pal, cflags, opt);
                }
            }

            p->setPen(act ? pal.highlightedText() : pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            int xm = motifItemFrame + checkcol + motifItemHMargin;

            if (mi->custom()) {
                int m = motifItemVMargin;
                p->save();
                if (dis && !act) {
                    p->setPen(pal.light());
                    mi->custom()->paint(p, pal, act, !dis,
                                         x+xm+1, y+m+1, w-xm-tab+1, h-2*m);
                    p->setPen(discol);
                }
                mi->custom()->paint(p, pal, act, !dis,
                                     x+xm, y+m, w-xm-tab+1, h-2*m);
                p->restore();
            }
            QString s = mi->text();
            if (!s.isNull()) {                        // draw text
                int t = s.find('\t');
                int m = motifItemVMargin;
                const int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (t >= 0) {                                // draw tab text
                    if (dis && !act) {
                        p->setPen(pal.light());
                        p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame+1,
                                     y+m+1, tab, h-2*m, text_flags, s.mid(t+1));
                        p->setPen(discol);
                    }
                    p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
                                 y+m, tab, h-2*m, text_flags, s.mid(t+1));
                    s = s.left(t);
                }
                if (dis && !act) {
                    p->setPen(pal.light());
                    p->drawText(x+xm+1, y+m+1, w-xm+1, h-2*m, text_flags, s, t);
                    p->setPen(discol);
                }
                p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
            } else if (mi->pixmap()) {                        // draw pixmap
                QPixmap *pixmap = mi->pixmap();
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::OpaqueMode);
                p->drawPixmap(x+xm, y+motifItemFrame, *pixmap);
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::TransparentMode);
            }
            if (mi->popup()) {                        // draw sub menu arrow
                int dim = (h-2*motifItemFrame) / 2;
                if (act) {
                    if (!dis)
                        discol = white;
                    QPalette pal2(discol, pal.highlight(), white, white,
                                    dis ? discol : white, discol, white);
                    drawPrimitive(PE_ArrowRight, p, QRect(x+w - motifArrowHMargin - motifItemFrame - dim, y + h / 2 - dim / 2, dim, dim),
                                  pal2, Style_Enabled);
                } else {
                    drawPrimitive(PE_ArrowRight, p, QRect(x+w - motifArrowHMargin - motifItemFrame - dim, y + h / 2 - dim / 2, dim, dim),
                                  pal, !dis ? Style_Enabled : Style_Default);
                }
            }
        }
        break;
#endif

    default:
        QWindowsStyle::drawControl(element, p, widget, r, pal, flags, opt);
        break;
    }
}

#endif
