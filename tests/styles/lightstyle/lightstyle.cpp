#include "lightstyle.h"

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qframe.h"
#include "qpushbutton.h"
#include "qdrawutil.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qguardedptr.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qimage.h"
#include "qcombobox.h"
#include "qslider.h"


static QPixmap blend(const QSize &size, const QColor &color)
{
    QImage image(size, 32);
    image.setAlphaBuffer(TRUE);
    int i;
    uint *p, pixel;

    i = image.numBytes() / 4;
    // i = image.bytesPerLine() / 4;
    p = (uint *) image.scanLine(0);
    pixel = qRgba(color.red(), color.green(), color.blue(), 0xdd);
    while (i > 4) {
	p[0] = pixel;
	p[1] = pixel;
	p[2] = pixel;
	p[3] = pixel;
	i-=4;
	p+=4;
    }
    if (i > 0) {
	p[0] = pixel;
	if (i > 1) {
	    p[1] = pixel;
	    if (i > 2) {
		p[2] = pixel;
	    }
	}
    }

    // the interlacing stuff is below

    // QColor color2 = color.dark(115);
    // i = image.bytesPerLine() / 4;
    // p = (uint *) image.scanLine(1);
    // pixel = qRgba(color2.red(), color2.green(), color2.blue(), 0xdd);
    // while (i > 4) {
    //     p[0] = pixel;
    //     p[1] = pixel;
    //     p[2] = pixel;
    //     p[3] = pixel;
    //     i-=4;
    //     p+=4;
    // }
    // if (i > 0) {
    //     p[0] = pixel;
    //     if (i > 1) {
    //         p[1] = pixel;
    // 	   if (i > 2) {
    // 	       p[2] = pixel;
    // 	   }
    //     }
    // }

    // for (i = 2; i < image.height() - 1; i += 2)
    // memcpy(image.scanLine(i), image.scanLine(0),
    // image.bytesPerLine() * 2);
    // if (image.height() % 2)
    // memcpy(image.scanLine(image.height() - 1), image.scanLine(0),
    // image.bytesPerLine());

    return image;
}


class LightStyleFilter : public QObject
{
public:
    QPixmap menupixmap, selpixmap;

    LightStyleFilter()
	: QObject(0)
    {
    }

    void constrainPixmap(const QSize &constraint, const QColorGroup &cg)
    {
	if (menupixmap.width() < constraint.width() ||
	    menupixmap.height() < constraint.height())
	    menupixmap = blend(QSize(QMAX(selpixmap.width(), constraint.width()),
				     QMAX(selpixmap.height(), constraint.height())),
			       cg.button());

	if (selpixmap.width() < constraint.width() ||
	    selpixmap.height() < constraint.height())
	    selpixmap = blend(QSize(QMAX(selpixmap.width(), constraint.width()),
				    QMAX(selpixmap.height(), constraint.height())),
			      cg.highlight());
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
	QPopupMenu *popup = (QPopupMenu *) object;
	if (event->type() == QEvent::Show) {
	    QPixmap pix =
		QPixmap::grabWindow( QApplication::desktop()->winId(),
				     popup->x(), popup->y(),
				     popup->width(), popup->height()),
		pix2(pix.size());

	    constrainPixmap(pix.size(), popup->colorGroup());

	    QPainter p;

	    p.begin(&pix2);
	    p.drawPixmap(0, 0, pix);
	    p.drawPixmap(0, 0, selpixmap);
	    p.end();

	    p.begin(&pix);
	    p.drawPixmap(0, 0, menupixmap);
	    p.end();

	    QPalette pal(popup->palette());
	    pal.setBrush(QPalette::Active, QColorGroup::Button,
			 QBrush(pal.active().button(), pix));
	    pal.setBrush(QPalette::Inactive, QColorGroup::Button,
			 QBrush(pal.active().button(), pix));
	    pal.setBrush(QPalette::Disabled, QColorGroup::Button,
			 QBrush(pal.active().button(), pix));
	    pal.setBrush(QPalette::Active, QColorGroup::Highlight,
			 QBrush(pal.active().button(), pix2));
	    pal.setBrush(QPalette::Inactive, QColorGroup::Highlight,
			 QBrush(pal.active().button(), pix2));
	    pal.setBrush(QPalette::Disabled, QColorGroup::Highlight,
			 QBrush(pal.active().button(), pix2));
	    popup->setPalette(pal);
	} else if (event->type() == QEvent::Hide) {
	    popup->move(-popup->x(), -popup->y());
	    popup->setBackgroundPixmap(QPixmap());
	    int count = 0;
	    QApplication::syncX();
	    while (qApp->hasPendingEvents() && count++ < 5)
		qApp->processEvents();
	}

	return FALSE;
    }
};


class LightStylePrivate
{
public:
    LightStylePrivate()
	: ref(1)
    {
	filter = new LightStyleFilter;
    }

    ~LightStylePrivate()
    {
	delete filter;
	filter = 0;
    }

    QPalette oldPalette;
    LightStyleFilter *filter;
    int ref;
};


static LightStylePrivate *singleton = 0;


LightStyle::LightStyle()
    : QWindowsStyle()
{
    if (! singleton) {
	singleton = new LightStylePrivate;

	QPalette pal = QApplication::palette();
	singleton->oldPalette = pal;

	QColor bg = pal.color(QPalette::Active, QColorGroup::Background);
	QColor prelight;

	if ( (bg.red() + bg.green() + bg.blue()) / 3 > 128)
	    prelight = pal.color(QPalette::Active,
				 QColorGroup::Background).light(110);
	else
	    prelight = pal.color(QPalette::Active,
				 QColorGroup::Background).light(120);

	QColorGroup active2(pal.color(QPalette::Active,
 				      QColorGroup::Foreground),      // foreground
			    prelight,                                // button
			    prelight.light(),                        // light
			    prelight.dark(),                         // dark
			    prelight.dark(120),                      // mid
			    pal.color(QPalette::Active,
				      QColorGroup::Text),            // text
			    pal.color(QPalette::Active,
				      QColorGroup::BrightText),      // bright text
			    pal.color(QPalette::Active,
				      QColorGroup::Base),            // base
			    bg);                                     // background
	active2.setColor(QColorGroup::Highlight,
			 pal.color(QPalette::Active, QColorGroup::Highlight));
    } else
	singleton->ref++;
}


LightStyle::~LightStyle()
{
    if (singleton && --singleton->ref <= 0) {
	delete singleton;
	singleton = 0;
    }
}


void LightStyle::unPolish(QWidget *widget)
{
    if (widget->inherits("QPopupMenu")) {
	widget->removeEventFilter(singleton->filter);
	widget->unsetPalette();
    }

    QWindowsStyle::unPolish(widget);
}


void LightStyle::polish(QApplication *app)
{
    QPalette pal = app->palette();

    QColorGroup active(pal.color(QPalette::Active,
				 QColorGroup::Foreground),           // foreground
		       pal.color(QPalette::Active,
				 QColorGroup::Button),               // button
		       pal.color(QPalette::Active,
				 QColorGroup::Background).light(),   // light
		       pal.color(QPalette::Active,
				 QColorGroup::Background).dark(175), // dark
		       pal.color(QPalette::Active,
				 QColorGroup::Background).dark(110), // mid
		       pal.color(QPalette::Active,
				 QColorGroup::Text),                 // text
		       pal.color(QPalette::Active,
				 QColorGroup::BrightText),           // bright text
		       pal.color(QPalette::Active,
				 QColorGroup::Base),                 // base
		       pal.color(QPalette::Active,
				 QColorGroup::Background)),          // background


	disabled(pal.color(QPalette::Disabled,
			   QColorGroup::Foreground),                 // foreground
		 pal.color(QPalette::Disabled,
			   QColorGroup::Button),                     // button
		 pal.color(QPalette::Disabled,
			   QColorGroup::Background).light(),         // light
		 pal.color(QPalette::Disabled,
			   QColorGroup::Background).dark(),          // dark
		 pal.color(QPalette::Disabled,
			   QColorGroup::Background).dark(110),       // mid
		 pal.color(QPalette::Disabled,
			   QColorGroup::Text),                       // text
		 pal.color(QPalette::Disabled,
			   QColorGroup::BrightText),                 // bright text
		 pal.color(QPalette::Disabled,
			   QColorGroup::Base),                       // base
		 pal.color(QPalette::Disabled,
			   QColorGroup::Background));                // background

    active.setColor(QColorGroup::Highlight,
		    pal.color(QPalette::Active, QColorGroup::Highlight));
    disabled.setColor(QColorGroup::Highlight,
		      pal.color(QPalette::Disabled, QColorGroup::Highlight));

    active.setColor(QColorGroup::HighlightedText,
		    pal.color(QPalette::Active, QColorGroup::HighlightedText));
    disabled.setColor(QColorGroup::HighlightedText,
		      pal.color(QPalette::Disabled, QColorGroup::HighlightedText));

    pal.setActive(active);
    pal.setInactive(active);
    pal.setDisabled(disabled);

    singleton->oldPalette = pal;

    QColor bg = pal.color(QPalette::Active, QColorGroup::Background);
    QColor prelight;

    if ( (bg.red() + bg.green() + bg.blue()) / 3 > 128)
	prelight = pal.color(QPalette::Active,
			     QColorGroup::Background).light(110);
    else
	prelight = pal.color(QPalette::Active,
			     QColorGroup::Background).light(120);

    QColorGroup active2(pal.color(QPalette::Active,
				  QColorGroup::Foreground),      // foreground
			prelight,                                // button
			prelight.light(),                        // light
			prelight.dark(),                         // dark
			prelight.dark(120),                      // mid
			pal.color(QPalette::Active,
				  QColorGroup::Text),            // text
			pal.color(QPalette::Active,
				  QColorGroup::BrightText),      // bright text
			pal.color(QPalette::Active,
				  QColorGroup::Base),            // base
			bg);                                     // background
    active2.setColor(QColorGroup::Highlight,
		     pal.color(QPalette::Active, QColorGroup::Highlight));

    app->setPalette(pal);
}


void LightStyle::unPolish(QApplication *app)
{
    app->setPalette(singleton->oldPalette);
}


void LightStyle::polishPopupMenu(QPopupMenu *menu)
{
    menu->installEventFilter(singleton->filter);
    menu->setMouseTracking(TRUE);
    if (! menu->testWState(WState_Polished))
	menu->setCheckable(TRUE);
}


static void drawLightBevel(QPainter *p, const QRect &r, const QColorGroup &cg,
			   QStyle::SFlags flags, bool usebase = FALSE,
			   const QBrush *fill = 0)
{
    QRect br = r;
    QBrush thefill;
    bool sunken =
	(flags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Sunken));

    if (fill) {
	thefill = *fill;
    } else if (flags & QStyle::Style_Enabled) {
	if (usebase)
	    thefill = cg.brush(QColorGroup::Base);
	else if (sunken)
	    thefill = cg.brush(QColorGroup::Midlight);
	else
	    thefill = cg.brush(QColorGroup::Button);
    } else
	thefill = cg.brush(QColorGroup::Background);

    p->setPen(cg.dark());
    p->drawRect(r);

    if (flags & (QStyle::Style_Down | QStyle::Style_On |
		 QStyle::Style_Sunken | QStyle::Style_Raised)) {
	bool sunken =
	    (flags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Sunken));
	// button bevel
	if (sunken)
	    p->setPen(cg.mid());
	else
	    p->setPen(cg.light());

	p->drawLine(r.x() + 1, r.y() + 2,
		    r.x() + 1, r.y() + r.height() - 3); // left
	p->drawLine(r.x() + 1, r.y() + 1,
		    r.x() + r.width() - 2, r.y() + 1); // top

	if (sunken)
	    p->setPen(cg.light());
	else
	    p->setPen(cg.mid());

	p->drawLine(r.x() + r.width() - 2, r.y() + 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
	p->drawLine(r.x() + 1, r.y() + r.height() - 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom

	br.addCoords(2, 2, -2, -2);
    } else
	br.addCoords(1, 1, -1, -1);

    // fill
    p->fillRect(br, thefill);
}


void LightStyle::drawPrimitive( PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				void **data ) const
{
    switch (pe) {
    case PE_HeaderSection:
	// make sure sunken headers (not down/pressed) are drawn raised
	flags = ((flags | Style_Sunken) ^ Style_Sunken) | Style_Raised;
	// fall through intended

    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
	drawLightBevel(p, r, cg, flags);
	break;

    case PE_ButtonDefault:
	p->setPen(cg.dark());
	p->setBrush(cg.light());
	p->drawRect(r);
	break;

    case PE_Indicator:
	if (flags & Style_Down)
	    drawLightBevel(p, r, cg, flags | Style_Sunken, false,
			   &cg.brush(QColorGroup::Mid));
	else
	    drawLightBevel(p, r, cg, flags | Style_Sunken, true);

	p->setPen(cg.foreground());
	if (flags & Style_NoChange) {
	    p->drawLine(r.x() + 3, r.y() + r.height() / 2,
			r.x() + r.width() - 4, r.y() + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() + 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() + 1 + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() - 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() - 1 + r.height() / 2);
	} else if (flags & Style_On) {
	    p->drawLine(r.x() + 4, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 5);
	    p->drawLine(r.x() + 3, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + 4,
			r.x() + r.width() - 5, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 5,
			r.x() + r.width() - 5, r.y() + 3);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 3);
	    p->drawLine(r.x() + 4, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 4);
	}

	break;

    case PE_ExclusiveIndicator:
	{
	    QRect br = r, // bevel rect
		  cr = r, // contents rect
		  ir = r; // indicator rect
	    br.addCoords(1, 1, -1, -1);
	    cr.addCoords(2, 2, -2, -2);
	    ir.addCoords(3, 3, -3, -3);

	    p->fillRect(r, cg.brush(QColorGroup::Background));

	    p->setPen(cg.dark());
	    p->drawArc(r, 0, 16*360);
	    p->setPen(cg.mid());
	    p->drawArc(br, 45*16, 180*16);
	    p->setPen(cg.light());
	    p->drawArc(br, 235*16, 180*16);

	    p->setPen(flags & Style_Down ? cg.mid() :
		      (flags & Style_Enabled ? cg.base() : cg.background()));
	    p->setBrush(flags & Style_Down ? cg.mid() :
			(flags & Style_Enabled ? cg.base() : cg.background()));
	    p->drawEllipse(cr);

	    if (flags & Style_On) {
		p->setBrush(cg.foreground());
		p->drawEllipse(ir);
	    }

	    break;
	}

    case PE_DockWindowHandle:
	flags |= Style_Raised;
	if (flags & Style_Horizontal) {
	    drawLightBevel(p, QRect(r.x() + 2, r.y() + 2, 8, r.height() - 4), cg, flags);

	    p->setPen(cg.dark());
	    p->drawLine(r.left() + 5, r.top() + 5, r.left() + 6, r.top() + 5);
	    p->drawLine(r.left() + 5, r.bottom() - 5, r.left() + 6, r.bottom() - 5);
	} else {
	    drawLightBevel(p, QRect(r.x() + 2, r.y() + 2, r.width() - 4, 8), cg, flags);

	    p->setPen(cg.dark());
	    p->drawLine(r.left() + 5, r.top() + 5, r.left() + 5, r.top() + 6);
	    p->drawLine(r.right() - 5, r.top() + 5, r.right() - 5, r.top() + 6);
	}
	break;

    case PE_Panel:
    case PE_PanelPopup:
	{
	    int lw = pixelMetric(PM_DefaultFrameWidth);
	    if (data)
		lw = *((int *) data[0]);

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags, false,
			       &cg.brush(QColorGroup::Background));
	    else
		QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_PanelDockWindow:
	{
	    int lw = pixelMetric(PM_DockWindowFrameWidth);
	    if (data)
		lw = *((int *) data[0]);

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags, false,
			       &cg.brush(QColorGroup::Button));
	    else
		QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break; }

    case PE_PanelMenuBar:
	{
	    int lw = pixelMetric(PM_MenuBarFrameWidth);
	    if (data)
		lw = *((int *) data[0]);

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags, false,
			       &cg.brush(QColorGroup::Button));
	    else
		QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_ScrollBarSubLine:
	{
	    QRect fr = r, ar = r;;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowLeft;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowUp;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Background));
	    drawPrimitive(pe, p, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarAddLine:
	{
	    QRect fr = r, ar = r;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowRight;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowDown;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Background));
	    drawPrimitive(pe, p, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
	{
	    QRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		fr.addCoords(0, 2, 0, 0);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, 0, 0);
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Mid));
	    break;
	}

    case PE_ScrollBarSlider:
	{
	    QRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		fr.addCoords(0, 2, 0, -1);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, -1, 0);
	    }

	    drawLightBevel(p, fr, cg,
			   ((flags | Style_Down) ^ Style_Down) |
			   ((flags & Style_Enabled) ? Style_Raised : Style_Default));
	    break;
	}

    default:
	QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, data);
	break;
    }
}


void LightStyle::drawControl( ControlElement control,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      SFlags flags,
			      void **data ) const
{
    switch (control) {
    case CE_TabBarTab:
	{
	    QRect tr(r);
	    QRect fr(r);

	    tr.addCoords(0, 0,  0, -1);
	    fr.addCoords(2, 2, -2, -2);

	    if (! (flags & Style_Selected)) {
		tr.addCoords(0, 1, 0, 0);
		fr.addCoords(0, 1, 0, 0);

		p->setPen(cg.dark());
		p->drawRect(tr);

		if (tr.left() == 0)
		    p->drawPoint(tr.left(), tr.bottom() + 1);

		p->setPen(cg.light());
		p->drawLine(tr.left() + 1, tr.bottom() - 1,
			    tr.left() + 1, tr.top() + 2);
		p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.right() - 1, tr.top() + 1);
		if (tr.left() == 0)
		    p->drawLine(tr.left() + 1, tr.bottom() + 1,
				tr.right(), tr.bottom() + 1);
		else
		    p->drawLine(tr.left(), tr.bottom() + 1,
				tr.right(), tr.bottom() + 1);

		p->setPen(cg.mid());
		p->drawLine(tr.right() - 1, tr.top() + 2,
			    tr.right() - 1, tr.bottom() - 1);
	    } else {
		p->setPen(cg.dark());
		if (tr.left() == 0)
		    p->drawLine(tr.left(), tr.bottom() + 1,
				tr.left(), tr.top() + 1);
		else
		    p->drawLine(tr.left(), tr.bottom(),
				tr.left(), tr.top() + 1);
		p->drawLine(tr.left(), tr.top(),
			    tr.right(), tr.top());
		p->drawLine(tr.right(), tr.top() + 1,
			    tr.right(), tr.bottom());

		p->setPen(cg.light());
		if (tr.left() == 0)
		    p->drawLine(tr.left() + 1, tr.bottom() + 2,
				tr.left() + 1, tr.top() + 2);
		else {
		    p->drawLine(tr.left() + 1, tr.bottom(),
				tr.left() + 1, tr.top() + 2);
		    p->drawPoint(tr.left(), tr.bottom() + 1);
		}
		p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.right() - 1, tr.top() + 1);
		p->drawPoint(tr.right(), tr.bottom() + 1);

		p->setPen(cg.mid());
		p->drawLine(tr.right() - 1, tr.top() + 2,
			    tr.right() - 1, tr.bottom());
	    }

	    p->fillRect(fr, ((flags & Style_Selected) ?
			     cg.background() : cg.mid()));
	    break;
	}

    case CE_PopupMenuItem:
	{
	    if (! widget || !data)
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = (QMenuItem *) data[0];
	    int tab = *((int *) data[1]);
	    int maxpmw = *((int *) data[2]);

	    if ( mi && mi->isSeparator() ) {
		// draw separator
		p->setPen(cg.dark());
		p->drawLine(r.left(), r.top(), r.right(), r.top());
		p->setPen(cg.light());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		break;
	    }

	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, TRUE, 1,
				&cg.brush(QColorGroup::Highlight));
	    else
		p->fillRect(r, cg.brush(QColorGroup::Button));

	    if ( !mi )
		break;

	    maxpmw = QMAX(maxpmw, 20);

	    QRect cr, ir, tr, sr;
	    // check column
	    cr.setRect(r.left(), r.top(), maxpmw, r.height());
	    // submenu indicator column
	    sr.setCoords(r.right() - maxpmw, r.top(), r.right(), r.bottom());
	    // tab/accelerator column
	    tr.setCoords(sr.left() - tab - 4, r.top(), sr.left(), r.bottom());
	    // item column
	    ir.setCoords(cr.right() + 4, r.top(), tr.right() - 4, r.bottom());

	    if (mi->isChecked() &&
		! (flags & Style_Active) &
		(flags & Style_Enabled))
		qDrawShadePanel(p, cr, cg, TRUE, 1, &cg.brush(QColorGroup::Midlight));

	    if (mi->iconSet()) {
		QIconSet::Mode mode =
		    (flags & Style_Enabled) ? QIconSet::Normal : QIconSet::Disabled;
		if ((flags & Style_Active) && (flags & Style_Enabled))
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if (popupmenu->isCheckable() && mi->isChecked())
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode );
		QRect pmr(QPoint(0, 0), pixmap.size());
		pmr.moveCenter(cr.center());
		p->setPen(cg.text());
		p->drawPixmap(pmr.topLeft(), pixmap);
	    } else if (popupmenu->isCheckable() && mi->isChecked())
		drawPrimitive(PE_CheckMark, p, cr, cg,
			      (flags & Style_Enabled) | Style_On);

	    QColor textcolor;
	    QColor embosscolor;
	    if (flags & Style_Active) {
		if (! (flags & Style_Enabled))
		    textcolor = cg.highlight().dark(110);
		else
		    textcolor = cg.highlightedText();
		embosscolor = cg.highlight().light(110);
	    } else if (! (flags & Style_Enabled)) {
		textcolor = cg.text();
		embosscolor = cg.light();
	    } else
		textcolor = embosscolor = cg.buttonText();
	    p->setPen(textcolor);

	    if (mi->custom()) {
		p->save();
		if (! (flags & Style_Enabled)) {
		    p->setPen(cg.light());
		    mi->custom()->paint(p, cg, flags & Style_Active,
					flags & Style_Enabled,
					ir.x() + 1, ir.y() + 1,
					ir.width() - 1, ir.height() - 1);
		    p->setPen(textcolor);
		}
		mi->custom()->paint(p, cg, flags & Style_Active,
				    flags & Style_Enabled,
				    ir.x(), ir.y(),
				    ir.width(), ir.height());
		p->restore();
	    }

	    QString text = mi->text();
	    if (! text.isNull()) {
		int t = text.find('\t');

		// draw accelerator/tab-text
		if (t >= 0) {
		    if (! (flags & Style_Enabled)) {
			p->setPen(embosscolor);
			tr.moveBy(1, 1);
			p->drawText(tr, (AlignVCenter | AlignRight | ShowPrefix |
					 DontClip | SingleLine), text.mid(t + 1));
			tr.moveBy(-1, -1);
			p->setPen(textcolor);
		    }

		    p->drawText(tr, (AlignVCenter | AlignRight | ShowPrefix |
				     DontClip | SingleLine), text.mid(t + 1));
		}

		if (! (flags & Style_Enabled)) {
		    p->setPen(embosscolor);
		    ir.moveBy(1, 1);
		    p->drawText(ir, AlignVCenter | ShowPrefix | DontClip | SingleLine,
				text, t);
		    ir.moveBy(-1, -1);
		    p->setPen(textcolor);
		}

		p->drawText(ir, AlignVCenter | ShowPrefix | DontClip | SingleLine,
			    text, t);
	    } else if (mi->pixmap()) {
		QPixmap pixmap = *mi->pixmap();
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(OpaqueMode);
		p->drawPixmap(ir.x(), (ir.height() - pixmap.height()) / 2, pixmap);
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(TransparentMode);
	    }

	    if (mi->popup())
		drawPrimitive(PE_ArrowRight, p, sr, cg, flags);
	    break;
	}

    case CE_MenuBarItem:
	{
	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Highlight));
	    else
		p->fillRect(r, cg.brush(QColorGroup::Button));

	    if (! data)
		break;

	    QMenuItem *mi = (QMenuItem *) data[0];
	    drawItem( p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
		      flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		      ((flags & Style_Active) ? &
		       cg.highlightedText() : &cg.buttonText()));
	    break;
	}

    case CE_ProgressBarGroove:
	drawLightBevel(p, r, cg, Style_Sunken, false,
		       &cg.brush(QColorGroup::Background));
	break;

    default:
	QWindowsStyle::drawControl(control, p, widget, r, cg, flags, data);
	break;
    }
}


void LightStyle::drawComplexControl( ComplexControl control,
				     QPainter* p,
				     const QWidget* widget,
				     const QRect& r,
				     const QColorGroup& cg,
				     SFlags flags,
				     SCFlags controls,
				     SCFlags active,
				     void **data ) const
{
    switch (control) {
    case CC_ComboBox:
	{
	    const QComboBox *combobox = (const QComboBox *) widget;
	    QRect frame, arrow, field;
	    frame =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxFrame, data),
				   widget);
	    arrow =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxArrow, data),
				   widget);
	    field =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxEditField, data),
				   widget);

	    if ((controls & SC_ComboBoxFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Raised);

	    if ((controls & SC_ComboBoxArrow) && arrow.isValid()) {
		if (active == SC_ComboBoxArrow)
		    p->fillRect(arrow, cg.brush(QColorGroup::Mid));
		arrow.addCoords(4, 2, -2, -2);
		drawPrimitive(PE_ArrowDown, p, arrow, cg, flags);
	    }

	    if ((controls & SC_ComboBoxEditField) && field.isValid()) {
		p->setPen(cg.dark());
		if (combobox->editable()) {
		    field.addCoords(-1, -1, 1, 1);
		    p->drawRect(field);
		} else
		    p->drawLine(field.right() + 1, field.top(),
				field.right() + 1, field.bottom());

		if (flags & Style_HasFocus) {
		    if (! combobox->editable()) {
			p->fillRect( field, cg.brush( QColorGroup::Highlight ) );
			QRect fr =
			    QStyle::visualRect( subRect( SR_ComboBoxFocusRect, widget ),
						widget );
			void *pdata[1];
			pdata[0] = (void *) &cg.highlight();
			drawPrimitive( PE_FocusRect, p, fr, cg,
				       flags | Style_FocusAtBorder, pdata);
		    }

		    p->setPen(cg.highlightedText());
		    p->setBackgroundColor(cg.highlight());
		} else {
		    p->setPen(cg.buttonText());
		    p->setBackgroundColor(cg.button());
		}
	    }

	    break;
	}

    case CC_SpinWidget:
	{
	    const QSpinWidget *spinwidget = (const QSpinWidget *) widget;
	    QRect frame, up, down;

	    frame = querySubControlMetrics(CC_SpinWidget, widget,
					   SC_SpinWidgetFrame, data);
	    up = querySubControlMetrics(CC_SpinWidget, widget,
					SC_SpinWidgetUp, data);
	    down = querySubControlMetrics(CC_SpinWidget, widget,
					  SC_SpinWidgetDown, data);

	    if ((controls & SC_SpinWidgetFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Sunken, true);

	    if ((controls & SC_SpinWidgetUp) && up.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetUp;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen(cg.dark());
		p->drawLine(up.topLeft(), up.bottomLeft());

		up.addCoords(1, 0, 0, 0);
		p->fillRect(up, cg.brush(QColorGroup::Button));
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(up.left(), up.top(),
			    up.right() - 1, up.top());
		p->drawLine(up.left(), up.top() + 1,
			    up.left(), up.bottom() - 1);
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(up.right(), up.top(),
			    up.right(), up.bottom());
		p->drawLine(up.left(), up.bottom(),
			    up.right() - 1, up.bottom());

		up.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, up, cg, flags |
			      ((active == SC_SpinWidgetUp) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    if ((controls & SC_SpinWidgetDown) && down.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetDown;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen(cg.dark());
		p->drawLine(down.topLeft(), down.bottomLeft());

		down.addCoords(1, 0, 0, 0);
		p->fillRect(down, cg.brush(QColorGroup::Button));
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(down.left(), down.top(),
			    down.right() - 1, down.top());
		p->drawLine(down.left(), down.top() + 1,
			    down.left(), down.bottom() - 1);
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(down.right(), down.top(),
			    down.right(), down.bottom());
		p->drawLine(down.left(), down.bottom(),
			    down.right() - 1, down.bottom());

		down.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, down, cg, flags |
			      ((active == SC_SpinWidgetDown) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, subline2, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, data);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, data);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, data);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, data);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  data);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   data);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    data);

	    subline2 = addline;
	    if (scrollbar->orientation() == Qt::Horizontal)
		subline2.moveBy(-addline.width(), 0);
	    else
		subline2.moveBy(0, -addline.height());

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
		drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));

		if (subline2.isValid())
		    drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg,
				  ((maxedOut) ? Style_Default : Style_Enabled) |
				  ((active == SC_ScrollBarSubLine) ?
				   Style_Down : Style_Default) |
				  ((scrollbar->orientation() == Qt::Horizontal) ?
				   Style_Horizontal : Style_Vertical));
	    }
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarFirst) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarLast) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSlider) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : Style_Vertical));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}

    case CC_Slider:
	{
	    const QSlider *slider = (const QSlider *) widget;
	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  data),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  data);

	    if ((controls & SC_SliderGroove) && groove.isValid()) {
		if (flags & Style_HasFocus)
		    drawPrimitive( PE_FocusRect, p, groove, cg );

		if (slider->orientation() == Qt::Horizontal) {
		    int dh = (groove.height() - 5) / 2;
		    groove.addCoords(0, dh, 0, -dh);
		} else {
		    int dw = (groove.width() - 5) / 2;
		    groove.addCoords(dw, 0, -dw, 0);
		}

		drawLightBevel(p, groove, cg,
			       ((flags | Style_Raised) ^ Style_Raised) |
			       ((flags & Style_Enabled) ? Style_Sunken : Style_Default));
	    }

	    if ((controls & SC_SliderHandle) && handle.isValid()) {
		drawLightBevel(p, handle, cg,
			       ((flags | Style_Down) ^ Style_Down) |
			       ((flags & Style_Enabled) ? Style_Raised : Style_Default));

	    }

	    if (controls & SC_SliderTickmarks)
		QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags,
						 SC_SliderTickmarks, active, data );
	    break;
	}

    default:
	QWindowsStyle::drawComplexControl(control, p, widget, r, cg, flags,
					  controls, active, data);
	break;
    }
}


QRect LightStyle::querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  void **data ) const
{
    QRect ret;

    switch (control) {
    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    int sliderstart = 0;
	    int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	    int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
			  scrollbar->width() : scrollbar->height()) - (sbextent * 3);
	    int sliderlen;

	    if (data)
		sliderstart = *((int*) data[0]);
	    else
		sliderstart = sbextent;

	    // calculate slider length
	    if (scrollbar->maxValue() != scrollbar->minValue()) {
		uint range = scrollbar->maxValue() - scrollbar->minValue();
		sliderlen = (scrollbar->pageStep() * maxlen) /
			    (range + scrollbar->pageStep());

		int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
		if ( sliderlen < slidermin || range > INT_MAX / 2 )
		    sliderlen = slidermin;
		if ( sliderlen > maxlen )
		    sliderlen = maxlen;
	    } else
		sliderlen = maxlen;

	    switch (sc) {
	    case SC_ScrollBarSubLine:
		// top/left button
		ret.setRect(0, 0, sbextent, sbextent);
		break;

	    case SC_ScrollBarAddLine:
		// bottom/right button
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(scrollbar->width() - sbextent, 0, sbextent, sbextent);
		else
		    ret.setRect(0, scrollbar->height() - sbextent, sbextent, sbextent);
		break;

	    case SC_ScrollBarSubPage:
		// between top/left button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
		else
		    ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
		break;

	    case SC_ScrollBarAddPage:
		// between bottom/right button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart + sliderlen, 0,
				 maxlen - sliderstart - sliderlen + sbextent, sbextent);
		else
		    ret.setRect(0, sliderstart + sliderlen,
				 sbextent, maxlen - sliderstart - sliderlen + sbextent);
		break;

	    case SC_ScrollBarGroove:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, scrollbar->width() - sbextent * 3,
				 scrollbar->height());
		else
		    ret.setRect(0, sbextent, scrollbar->width(),
				 scrollbar->height() - sbextent * 3);
		break;

	    case SC_ScrollBarSlider:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart, 0, sliderlen, sbextent);
		else
		    ret.setRect(0, sliderstart, sbextent, sliderlen);
		break;

	    default:
		break;
	    }

	    break;
	}

    default:
	ret = QWindowsStyle::querySubControlMetrics(control, widget, sc, data);
	break;
    }

    return ret;
}


QStyle::SubControl LightStyle::querySubControl( ComplexControl control,
						const QWidget *widget,
						const QPoint &pos,
						void **data ) const
{
    QStyle::SubControl ret =
	QWindowsStyle::querySubControl(control, widget, pos, data);

    // this is an ugly hack, but i really don't care, it's the quickest way to
    // enabled the third button
    if (control == CC_ScrollBar &&
	ret == SC_None)
	ret = SC_ScrollBarSubLine;

    return ret;
}


int LightStyle::pixelMetric( PixelMetric metric,
			     const QWidget *widget ) const
{
    int ret;

    switch (metric) {
    case PM_ButtonMargin:
	ret = 4;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_ButtonDefaultIndicator:
    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 13;
	break;

    case PM_TabBarTabOverlap:
	ret = 0;
	break;

    case PM_ScrollBarExtent:
	ret = 14;
	break;

    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}


QSize LightStyle::sizeFromContents( ContentsType contents,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    void **data ) const
{
    QSize ret;

    switch (contents) {
    case CT_PopupMenuItem:
	{
	    if (! widget || ! data)
		break;

	    QMenuItem *mi = (QMenuItem *) data[0];
	    if (mi->isSeparator()) {
		ret = QSize(10, 2);
		break;
	    }

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    int maxpmw = *((int *) data[1]);
	    int w = contentsSize.width(), h = contentsSize.height();

	    // most iconsets are 22x22 in menus
	    if (h < 22)
		h = 22;

	    if (mi->pixmap())
		h = QMAX(h, mi->pixmap()->height());
	    else if (! mi->text().isNull())
		h = QMAX(h, popupmenu->fontMetrics().height());

	    if (mi->iconSet() != 0)
		h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						  QIconSet::Normal).height());
	    h += 2;

	    // check | 4 pixels | item | 8 pixels | accel | 4 pixels | check
	    maxpmw = QMAX(maxpmw, 20);
	    w += (maxpmw * 2) + 8;

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += 8;

	    ret = QSize(w, h);
	    break;
	}

    default:
	ret = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, data);
	break;
    }

    return ret;
}
