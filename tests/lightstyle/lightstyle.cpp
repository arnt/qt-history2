#include "lightstyle.h"

#define INCLUDE_MENUITEM_DEF
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


class LightStylePrivate
{
public:
    LightStylePrivate()
	: hoverWidget(0), ref(1), savePalette(0)
    {
    }

    QGuardedPtr<QWidget> hoverWidget;
    QPalette oldPalette, hoverPalette;
    int ref;
    QPoint mousePos;
    QPalette *savePalette;
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

	singleton->hoverPalette = pal;
	singleton->hoverPalette.setActive(active2);
 	singleton->hoverPalette.setInactive(active2);
    } else
	singleton->ref++;
}


LightStyle::~LightStyle()
{
    if (singleton && singleton->ref-- <= 0) {
	delete singleton;
	singleton = 0;
    }
}


QSize LightStyle::scrollBarExtent() const
{
    return QSize(12 + defaultFrameWidth(), 12 + defaultFrameWidth());
}


int LightStyle::buttonDefaultIndicatorWidth() const
{
    return 2;
}


int LightStyle::sliderThickness() const
{
    return 16;
}

int LightStyle::sliderLength() const
{
    return 13;
}


int LightStyle::buttonMargin() const
{
    return 4;
}


QSize LightStyle::exclusiveIndicatorSize() const
{
    return QSize(13, 13);
}


int LightStyle::defaultFrameWidth() const
{
    return 2;
}


QSize LightStyle::indicatorSize() const
{
    return QSize(13, 13);
}


void LightStyle::polish(QWidget *widget)
{
    if (widget->inherits("QPushButton"))
	widget->installEventFilter(this);

#if QT_VERSION >= 300
    if (widget->inherits("QLineEdit")) {
	QLineEdit *lineedit = (QLineEdit *) widget;
	lineedit->setFrameShape(QFrame::StyledPanel);
	lineedit->setLineWidth(2);
    }
#endif

    QWindowsStyle::polish(widget);
}


void LightStyle::unPolish(QWidget *widget)
{
    if (widget->inherits("QPushButton"))
	widget->removeEventFilter(this);

#if QT_VERSION >= 300
    if (widget->inherits("QLineEdit")) {
	QLineEdit *lineedit = (QLineEdit *) widget;
	lineedit->setLineWidth(1);
	lineedit->setFrameShape(QFrame::WinPanel);
    }
#endif

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

    singleton->hoverPalette = pal;
    singleton->hoverPalette.setActive(active2);
    singleton->hoverPalette.setInactive(active2);

    app->setPalette(pal);
}


void LightStyle::unPolish(QApplication *app)
{
    app->setPalette(singleton->oldPalette);
}


void LightStyle::polishPopupMenu(QPopupMenu *menu)
{
    menu->setMouseTracking(TRUE);
}


void LightStyle::drawPushButton(QPushButton *button, QPainter *p)
{
    int x1, y1, x2, y2;
    button->rect().coords(&x1, &y1, &x2, &y2);

    if (button->isDefault()) {
	p->save();
	p->setPen(button->palette().active().color(QColorGroup::Highlight));
	p->setBrush(button->palette().active().brush(QColorGroup::Highlight));
	p->drawRoundRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, 15, 15);
	p->restore();
    }

    if (button->isDefault() || button->autoDefault()) {
        x1 += buttonDefaultIndicatorWidth();
        y1 += buttonDefaultIndicatorWidth();
        x2 -= buttonDefaultIndicatorWidth();
        y2 -= buttonDefaultIndicatorWidth();

	if (button->isDefault()) {
	    QPointArray pa(8);
	    pa.setPoint(0, x1 + 2, y1    );
	    pa.setPoint(1, x2 - 1, y1    );
	    pa.setPoint(2, x2 + 1, y1 + 2);
	    pa.setPoint(3, x2 + 1, y2 - 2);
	    pa.setPoint(4, x2 - 2, y2 + 1);
	    pa.setPoint(5, x1 + 2, y2 + 1);
	    pa.setPoint(6, x1,     y2 - 1);
	    pa.setPoint(7, x1,     y1 + 2);
	    QRegion r(pa);
	    p->setClipRegion(r);
	}
    }

    QBrush fill;
    if (button->isDown() || button->isOn())
        fill = button->colorGroup().brush(QColorGroup::Mid);
    else
        fill = button->colorGroup().brush(QColorGroup::Button);

    if ( !button->isFlat() || button->isOn() || button->isDown() )
        drawButton(p, x1, y1, x2 - x1 + 1, y2 - y1 + 1,
                   button->colorGroup(), button->isOn() || button->isDown(), &fill);
}


void LightStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g,
                                 bool sunken, const QBrush *fill)
{
    p->save();

    p->fillRect(x, y, w, h, g.background());
    if ( fill )
	p->fillRect(x + 2, y + 2, w - 4, h - 4, *fill);
    else
	p->fillRect(x + 2, y + 2, w - 4, h - 4,
		    QBrush(sunken ? g.mid() : g.button()));

    // frame
    p->setPen(g.dark());
    p->drawLine(x, y + 2, x, y + h - 3); // left
    p->drawLine(x + 2, y, x + w - 3, y); // top
    p->drawLine(x + w - 1, y + 2, x + w - 1, y + h - 3); // right
    p->drawLine(x + 2, y + h - 1, x + w - 3, y + h - 1); // bottom
    p->drawPoint(x + 1, y + 1);
    p->drawPoint(x + 1, y + h - 2);
    p->drawPoint(x + w - 2, y + 1);
    p->drawPoint(x + w - 2, y + h - 2);

    // bevel
    if (sunken)
	p->setPen(g.mid());
    else
	p->setPen(g.light());

    p->drawLine(x + 1, y + 2, x + 1, y + h - 3); // left
    p->drawLine(x + 2, y + 1, x + w - 3, y + 1); // top

    if (sunken)
	p->setPen(g.light());
    else
	p->setPen(g.mid());

    p->drawLine(x + w - 2, y + 2, x + w - 2, y + h - 3); // right + 1
    p->drawLine(x + 2, y + h - 2, x + w - 3, y + h - 2); // bottom + 1

    p->restore();
}


void LightStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &g,
                                      bool sunken, const QBrush *fill)
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}


void LightStyle::getButtonShift(int &x, int &y) const
{
    x = y = 0;
}


void LightStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
			       const QColorGroup &g, bool,
			       bool editable, bool,
			       const QBrush *fill)
{
    drawButton(p, x, y, w, h, g, FALSE, fill);

    if (editable) {
        QRect r = comboButtonRect(x, y, w, h);
        qDrawShadePanel(p, r.x() - 1, r.y() - 1,
			r.width() + defaultFrameWidth(),
			r.height() + defaultFrameWidth(),
			g, TRUE);
    }

    int indent = ((y + h) / 2) - 3;
    int xpos = x;

#if QT_VERSION >= 300
    if( QApplication::reverseLayout() )
        xpos += indent;
    else
#endif
        xpos += w - indent - 5;

    drawArrow(p, Qt::DownArrow, TRUE, xpos, indent, 5, 5, g, TRUE, fill);
}


QRect LightStyle::comboButtonRect( int x, int y, int w, int h ) const
{
    QRect r(x + 3, y + 3, w - 6, h - 6);
    int indent = ((y + h) / 2) - 3;
    r.setRight(r.right() - indent - 10);

#if QT_VERSION >= 300
    if( QApplication::reverseLayout() )
        r.moveBy( indent + 10, 0 );
#endif

    return r;
}


QRect LightStyle::comboButtonFocusRect(int x, int y, int w, int h ) const
{
    return comboButtonRect(x, y, w, h);
}


void LightStyle::drawPanel(QPainter *p, int x, int y, int w, int h,
			 const QColorGroup &g, bool sunken,
			 int lw, const QBrush *fill)
{
    if (lw >= 2) {
	if ( fill )
	    p->fillRect(x + 2, y + 2, w - 4, h - 4, *fill);

	QPen oldpen = p->pen();

	// frame
	p->setPen(g.dark());
	p->drawLine(x, y + 2, x, y + h - 3); // left
	p->drawLine(x + 2, y, x + w - 3, y); // top
	p->drawLine(x + w - 1, y + 2, x + w - 1, y + h - 3); // right
	p->drawLine(x + 2, y + h - 1, x + w - 3, y + h - 1); // bottom
	p->drawPoint(x + 1, y + 1);
	p->drawPoint(x + 1, y + h - 2);
	p->drawPoint(x + w - 2, y + 1);
	p->drawPoint(x + w - 2, y + h - 2);

	// bevel
	if (sunken)
	    p->setPen(g.mid());
	else
	    p->setPen(g.light());

	p->drawLine(x + 1, y + 2, x + 1, y + h - 3); // left
	p->drawLine(x + 2, y + 1, x + w - 3, y + 1); // top

	if (sunken)
	    p->setPen(g.light());
	else
	    p->setPen(g.mid());

	p->drawLine(x + w - 2, y + 2, x + w - 2, y + h - 3); // right + 1
	p->drawLine(x + 2, y + h - 2, x + w - 3, y + h - 2); // bottom + 1

	// corners
	p->setPen(g.background());
	p->drawLine(x, y, x + 1, y);
	p->drawLine(x, y + h - 1, x + 1, y + h - 1);
	p->drawLine(x + w - 2, y, x + w - 1, y);
	p->drawLine(x + w - 2, y + h - 1, x + w - 1, y + h - 1);
	p->drawPoint(x, y + 1);
	p->drawPoint(x, y + h - 2);
	p->drawPoint(x + w - 1, y + 1);
	p->drawPoint(x + w - 1, y + h - 2);

	p->setPen(oldpen);
    } else
	qDrawShadePanel(p, x, y, w, h, g, sunken, lw, fill);
}


void LightStyle::drawIndicator(QPainter *p, int x, int y ,int w, int h,
                               const QColorGroup &g, int state,
                               bool down, bool)
{
    drawButton(p, x, y, w, h, g, TRUE,
	       &g.brush(down ? QColorGroup::Mid : QColorGroup::Base));

    p->save();

    p->setPen(g.foreground());
    if (state == QButton::NoChange) {
	p->drawLine(x + 3, y + h / 2, x + w - 4, y + h / 2);
	p->drawLine(x + 3, y + 1 + h / 2, x + w - 4, y + 1 + h / 2);
	p->drawLine(x + 3, y - 1 + h / 2, x + w - 4, y - 1 + h / 2);
    } else if (state == QButton::On) {
	p->drawLine(x + 4, y + 3, x + w - 4, y + h - 5);
	p->drawLine(x + 3, y + 3, x + w - 4, y + h - 4);
	p->drawLine(x + 3, y + 4, x + w - 5, y + h - 4);
	p->drawLine(x + 3, y + h - 5, x + w - 5, y + 3);
	p->drawLine(x + 3, y + h - 4, x + w - 4, y + 3);
	p->drawLine(x + 4, y + h - 4, x + w - 4, y + 4);
    }

    p->restore();
}


void LightStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, bool on,
					bool down, bool)
{
    p->save();

    p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));

    p->setPen(g.dark());
    p->drawArc(x, y, w, h, 0, 16*360);
    p->setPen(g.mid());
    p->drawArc(x + 1, y + 1, w - 2, h - 2, 45*16, 180*16);
    p->setPen(g.light());
    p->drawArc(x + 1, y + 1, w - 2, h - 2, 235*16, 180*16);

    p->setPen(down ? g.mid() : g.base());
    p->setBrush(down ? g.mid() : g.base());
    p->drawEllipse(x + 2, y + 2, w - 4, h - 4);

    if (on) {
	p->setBrush(g.foreground());
	p->drawEllipse(x + 3, y + 3, w - x - 6, h - y - 6);
    }

    p->restore();
}


void LightStyle::drawTab(QPainter *p, const QTabBar *tabbar, QTab *tab,
                              bool selected)
{
    p->save();

    QColorGroup g = tabbar->colorGroup();
    QRect fr(tab->r);
    fr.setLeft(fr.left() + 2);

    if (! selected) {
        if (tabbar->shape() == QTabBar::RoundedAbove ||
            tabbar->shape() == QTabBar::TriangularAbove) {

            fr.setTop(fr.top() + 2);
        } else {
            fr.setBottom(fr.bottom() - 2);
        }
    }

    QRegion tabr(tab->r);

    QPointArray cliptri(4);
    cliptri.setPoint(0, fr.left(), fr.top());
    cliptri.setPoint(1, fr.left(), fr.top() + 5);
    cliptri.setPoint(2, fr.left() + 5, fr.top());
    cliptri.setPoint(3, fr.left(), fr.top());
    QRegion trir(cliptri);
    p->setClipRegion(tabr - trir);

    p->setPen(selected ? g.button() : g.mid());
    p->setBrush(g.brush(selected ? QColorGroup::Button : QColorGroup::Mid));

    fr.setWidth(fr.width() - 1);
    p->drawRect(fr.left() + 1, fr.top() + 1, fr.width() - 2, fr.height() - 2);

    if (tabbar->shape() == QTabBar::RoundedAbove) {
        // "rounded" tabs on top
        fr.setBottom(fr.bottom() - 1);

        p->setPen(g.dark());
	p->drawLine(fr.left(), fr.top() + 5, fr.left(), fr.bottom() - 1);
	p->drawLine(fr.left(), fr.top() + 5, fr.left() + 5, fr.top());
	p->drawLine(fr.left() + 5, fr.top(), fr.right() - 1, fr.top());
	p->drawLine(fr.right(), fr.top() + 1, fr.right(), fr.bottom() - 1);

	if (selected) {
	    p->drawLine(fr.right(), fr.bottom(), fr.right() + 2, fr.bottom());
	    p->drawPoint(fr.left(), fr.bottom());
	} else
	    p->drawLine(fr.left(), fr.bottom(), fr.right() + 2, fr.bottom());

	if (fr.left() == 2) {
	    p->drawPoint(fr.left() - 1, fr.bottom() + 1);
	    p->drawPoint(fr.left() - 2, fr.bottom() + 2);
	}

	if (selected) {
	    p->setPen(g.mid());
	    p->drawLine(fr.right() - 1, fr.top() + 1, fr.right() - 1, fr.bottom() - 2);
	}

	p->setPen(g.light());
	p->drawLine(fr.left() + 1, fr.top() + 6, fr.left() + 1,
		    fr.bottom() - (selected ? 0 : 1));
	p->drawLine(fr.left() + 1, fr.top() + 5, fr.left() + 5, fr.top() + 1);
	p->drawLine(fr.left() + 6, fr.top() + 1, fr.right() - 3, fr.top() + 1);
	if (selected) {
	    p->drawLine(fr.right() + 1, fr.bottom() + 1,
			fr.right() + 2, fr.bottom() + 1);
	    p->drawLine(fr.left(), fr.bottom() + 1, fr.left() + 1, fr.bottom() + 1);
	} else
	    p->drawLine(fr.left(), fr.bottom() + 1,
			fr.right() + 2, fr.bottom() + 1);
    } else if (tabbar->shape() == QTabBar::RoundedBelow) {
        // "rounded" tabs on bottom
        fr.setTop(fr.top() + 1);

        p->setPen(g.dark());
	p->drawLine(fr.left(), fr.top(), fr.left(), fr.bottom() - 1);
	p->drawLine(fr.left() + 1, fr.bottom(), fr.right() - 1, fr.bottom());
	p->drawLine(fr.right(), fr.top(), fr.right(), fr.bottom() - 1);

	if (! selected)
	    p->drawLine(fr.left(), fr.top(), fr.right() + 3, fr.top());
	else
	    p->drawLine(fr.right(), fr.top(), fr.right() + 3, fr.top());

	p->setPen(g.mid());
	if (selected)
	    p->drawLine(fr.right() - 1, fr.top() + 1, fr.right() - 1, fr.bottom() - 1);
	else
	    p->drawLine(fr.left(), fr.top() - 1, fr.right() + 3, fr.top() - 1);

	p->setPen(g.light());
	p->drawLine(fr.left() + 1, fr.top() + (selected ? -1 : 2),
		    fr.left() + 1, fr.bottom() - 1);

    } else {
        // triangular drawing code
        QCommonStyle::drawTab(p, tabbar, tab, selected);
    }

    p->restore();
}


void LightStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, Orientation orientation,
			  bool above, bool below)
{
    drawButton(p, x, y, w, h, g, FALSE, &g.brush(QColorGroup::Button));

    if (orientation == Horizontal) {
	if (above && below) {
	    drawArrow(p, Qt::UpArrow, FALSE, x + 1, y + 1, w, h / 2, g, TRUE);
	    drawArrow(p, Qt::DownArrow, FALSE, x + 1, y + (h / 2) - 1,
		      w, h / 2, g, TRUE);
	} else
	    drawArrow(p, (above) ? Qt::UpArrow : Qt::DownArrow,
		      FALSE, x + 1, y, w, h, g, TRUE);
    } else {
	if (above && below) {
	    drawArrow(p, Qt::LeftArrow, FALSE, x + 1, y, w / 2, h, g, TRUE);
	    drawArrow(p, Qt::RightArrow, FALSE, x + (w / 2) - 2, y, w / 2, h, g, TRUE);
	} else
	    drawArrow(p, (above) ? Qt::LeftArrow : Qt::RightArrow,
		      FALSE, x, y, w, h, g, TRUE);
    }
}


void LightStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
				const QColorGroup& g, QCOORD,
				Orientation orientation)
{
    int rx = (x + w) / 2;
    int ry = (y + h) / 2;

    if (orientation == Horizontal)
	drawButton(p, x, ry - 3, w, 6, g, TRUE, &g.brush(QColorGroup::Mid));
    else
	drawButton(p, rx - 3, y, 6, h, g, TRUE, &g.brush(QColorGroup::Mid));
}


void LightStyle::scrollBarMetrics(const QScrollBar *scrollbar,
                                       int &sliderMin, int &sliderMax,
                                       int &sliderLength, int &buttonDim) const
{
    int maxLength;
    int length = ((scrollbar->orientation() == Horizontal) ?
		  scrollbar->width() : scrollbar->height());
    int extent = ((scrollbar->orientation() == Horizontal) ?
		  scrollbar->height() : scrollbar->width());
    extent--;

    if (length > (extent + defaultFrameWidth() - 1) * 2 + defaultFrameWidth())
	buttonDim = extent - defaultFrameWidth();
    else
	buttonDim = (length - defaultFrameWidth()) / 2 - 1;

    sliderMin = buttonDim;
    maxLength = length - buttonDim * 3;

    if (scrollbar->maxValue() != scrollbar->minValue()) {
	uint range = scrollbar->maxValue() - scrollbar->minValue();
	sliderLength = (scrollbar->pageStep() * maxLength) /
		       (range + scrollbar->pageStep());

	if (sliderLength < buttonDim || range > INT_MAX / 2)
	    sliderLength = buttonDim;
	if (sliderLength > maxLength)
	    sliderLength = maxLength;
    } else
	sliderLength = maxLength;

    sliderMax = sliderMin + maxLength - sliderLength;
}


QStyle::ScrollControl LightStyle::scrollBarPointOver(const QScrollBar *scrollbar,
						   int sliderStart, const QPoint &p)
{
    if (! scrollbar->rect().contains(p))
	return NoScroll;

    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( scrollbar, sliderMin, sliderMax, sliderLength, buttonDim );

    if (scrollbar->orientation() == Horizontal)
	pos = p.x();
    else
	pos = p.y();

    if (pos < buttonDim)
	return SubLine;
    if (pos < sliderStart)
	return SubPage;
    if (pos < sliderStart + sliderLength)
	return Slider;
    if (pos < sliderMax + sliderLength)
	return AddPage;
    if (pos < sliderMax + sliderLength + buttonDim)
	return SubLine;
    return AddLine;
}



void LightStyle::drawScrollBarControls( QPainter* p, const QScrollBar* scrollbar,
                                             int sliderStart, uint controls,
                                             uint activeControl )
{
    QColorGroup g  = scrollbar->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( scrollbar, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) { // sanity check
        sliderStart = sliderMax;
    }

    QRect addR, subR, subR2, addPageR, subPageR, sliderR;
    int length =  ((scrollbar->orientation() == Horizontal) ?
		   scrollbar->width()  : scrollbar->height());
    int extent =  ((scrollbar->orientation() == Horizontal) ?
		   scrollbar->height() : scrollbar->width());

    if (scrollbar->orientation() == Horizontal) {
	subR.setRect(0, defaultFrameWidth(),
		     buttonDim, buttonDim);
	subR2.setRect(length - (buttonDim * 2), defaultFrameWidth() ,
		      buttonDim, buttonDim);
	addR.setRect(length - buttonDim, defaultFrameWidth(),
		     buttonDim, buttonDim);
    } else {
	subR.setRect(defaultFrameWidth() + 1, 0,
		     buttonDim, buttonDim);
	subR2.setRect(defaultFrameWidth() + 1, length - (buttonDim * 2),
		      buttonDim, buttonDim);
	addR.setRect(defaultFrameWidth() + 1, length - buttonDim,
		     buttonDim, buttonDim);
    }

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - defaultFrameWidth() - 1;
    if (scrollbar->orientation() == Horizontal) {
        subPageR.setRect( subR.right() + 1, defaultFrameWidth(),
                          sliderStart - subR.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, defaultFrameWidth(),
			  subR2.left() - sliderEnd, sliderW );
        sliderR.setRect( sliderStart, defaultFrameWidth(), sliderLength, sliderW );
    } else {
        subPageR.setRect( defaultFrameWidth(), subR.bottom() + 1,
			  sliderW, sliderStart - subR.bottom() - 1 );
        addPageR.setRect( defaultFrameWidth(), sliderEnd,
			  sliderW, subR2.top() - sliderEnd );
        sliderR .setRect( defaultFrameWidth(), sliderStart,
			  sliderW, sliderLength );
    }

    if ( controls == ( AddLine | SubLine | AddPage | SubPage |
                       Slider | First | Last ) ) {
	if (scrollbar->orientation() == Horizontal)
	    qDrawShadePanel(p, 0, 0, length, 2, g, TRUE, 1,
			    &g.brush(QColorGroup::Background));
	else
	    qDrawShadePanel(p, 0, 0, 2, length, g, TRUE, 1,
			    &g.brush(QColorGroup::Background));
    }

    if ( controls & AddLine )
        drawArrow( p, (scrollbar->orientation() == Vertical) ? DownArrow : RightArrow,
		   FALSE, addR.x(), addR.y(),
                   addR.width(), addR.height(),
		   (( activeControl == AddLine ) ?
		    singleton->hoverPalette.active() : g),
		   TRUE, &g.brush(QColorGroup::Background));
    if ( controls & SubLine ) {
        drawArrow( p, (scrollbar->orientation() == Vertical) ? UpArrow : LeftArrow,
		   FALSE, subR.x(), subR.y(),
                   subR.width(), subR.height(),
                   (( activeControl == SubLine ) ?
		    singleton->hoverPalette.active() : g),
		   TRUE, &g.brush(QColorGroup::Background));
        drawArrow( p, (scrollbar->orientation() == Vertical) ? UpArrow : LeftArrow,
		   FALSE, subR2.x(), subR2.y(),
                   subR2.width(), subR2.height(),
                   (( activeControl == SubLine ) ?
		    singleton->hoverPalette.active() : g),
		   TRUE, &g.brush(QColorGroup::Background));
    }

    if ( controls & SubPage )
        p->fillRect( subPageR,
		     ((activeControl == SubPage) ?
		      g.brush( QColorGroup::Dark ) :
		      g.brush( QColorGroup::Mid )));
    if ( controls & AddPage )
        p->fillRect( addPageR,
		     ((activeControl == AddPage) ?
    		      g.brush( QColorGroup::Dark ) :
		      g.brush( QColorGroup::Mid )));

    if ( controls & Slider ) {
        QPoint bo = p->brushOrigin();
        p->setBrushOrigin(sliderR.topLeft());
        if ( sliderR.isValid() ) {
            drawBevelButton( p, sliderR.x(), sliderR.y(),
                             sliderR.width(), sliderR.height(),
			     g, FALSE, &g.brush( QColorGroup::Button ) );
        }

        p->setBrushOrigin(bo);
    }
}


bool LightStyle::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type()) {
    case QEvent::Enter:
        {
            if (! object->isWidgetType() ||
		! object->inherits("QPushButton"))
		break;

	    singleton->hoverWidget = (QWidget *) object;
	    if (! singleton->hoverWidget->isEnabled()) {
		singleton->hoverWidget = 0;
		break;
	    }

	    QPalette pal = singleton->hoverWidget->palette();
	    if (singleton->hoverWidget->ownPalette())
		singleton->savePalette = new QPalette(pal);

	    singleton->hoverWidget->setPalette(singleton->hoverPalette);

	    break;
	}

    case QEvent::Leave:
	{
	    if (object != singleton->hoverWidget)
		break;

	    if (singleton->savePalette) {
		singleton->hoverWidget->setPalette(*(singleton->savePalette));
		delete singleton->savePalette;
		singleton->savePalette = 0;
	    } else
		singleton->hoverWidget->unsetPalette();

	    singleton->hoverWidget = 0;

	    break;
	}

    default:
	{
	    ;
	}
    }

    return QWindowsStyle::eventFilter(object, event);
}
