#include "qwsmanager_qws.h"

#ifndef QT_NO_QWS_MANAGER

#include "qdrawutil.h"
#include "qapplication.h"
#include "qaccel.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qpopupmenu.h"
#include "qpainter.h"
#include "qregion.h"
#include "qevent.h"
#include "qtimer.h"
#include "qgfx_qws.h"
#include "qwsdisplay_qws.h"

#define CORNER_GRAB	16
#define BORDER_WIDTH	4
#define TITLE_HEIGHT	20

// #### merge two copies of pixmaps

#ifndef QT_NO_IMAGEIO_XPM
/* XPM */
static const char * const menu_xpm[] = {
/* width height ncolors chars_per_pixel */
"16 16 11 1",
/* colors */
"  c #000000",
". c #336600",
"X c #666600",
"o c #99CC00",
"O c #999933",
"+ c #333300",
"@ c #669900",
"# c #999900",
"$ c #336633",
"% c #666633",
"& c #99CC33",
/* pixels */
"oooooooooooooooo",
"oooooooooooooooo",
"ooooo#.++X#ooooo",
"ooooX      Xoooo",
"oooX  XO#%  X&oo",
"oo#  Ooo&@O  Ooo",
"oo. Xoo#+ @X Xoo",
"oo+ OoO+ +O# +oo",
"oo+ #O+  +## +oo",
"oo. %@ ++ +. Xoo",
"oo#  O@OO+   #oo",
"oooX  X##$   Ooo",
"ooooX        Xoo",
"oooo&OX++X#OXooo",
"oooooooooooooooo",
"oooooooooooooooo"
};


static const char * const close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};

static const char * const maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};

static const char * const minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * const normalize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"     ........   ",
"     .XXXXXXXX  ",
"     .X     .X  ",
"     .X     .X  ",
"  ....X...  .X  ",
"  .XXXXXXXX .X  ",
"  .X     .XXXX  ",
"  .X     .X     ",
"  .X     .X     ",
"  .X......X     ",
"  .XXXXXXXX     ",
"                ",
"                ",
"                "};
#endif

QWidget *QWSManager::active = 0;
QPoint QWSManager::mousePos;

QWSManager::QWSManager(QWidget *w)
    : QObject(), activeRegion(None), managed(w), popup(0), timer(0)
{
    dx = 0;
    dy = 0;
    skipCount = 0;

    menuBtn = new QWSButton(this, Menu);
    closeBtn = new QWSButton(this, Close);
    minimizeBtn = new QWSButton(this, Minimize);
    maximizeBtn = new QWSButton(this, Maximize, TRUE);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(handleMove()));
}

QWSManager::~QWSManager()
{
#ifndef QT_NO_COMPLEXWIDGETS
    if (popup)
	delete popup;
#endif
    delete menuBtn;
    delete closeBtn;
    delete minimizeBtn;
    delete maximizeBtn;
}

QRegion QWSManager::region()
{
    return QApplication::qwsDecorator().region(managed, managed->geometry());
}

QWSManager::Region QWSManager::pointInRegion(const QPoint &p)
{
    QWSDecorator &dec = QApplication::qwsDecorator();
    QRect rect(managed->geometry());

    for (int i = Title; i <= LastRegion; i++) {
	if (dec.region(managed, rect, (QWSManager::Region)i).contains(p))
	    return (QWSManager::Region)i;
    }

    return None;
}

bool QWSManager::event(QEvent *e)
{
    switch (e->type()) {
	case QEvent::MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonPress:
            mousePressEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonRelease:
            mouseReleaseEvent( (QMouseEvent*)e );
            break;

        case QEvent::MouseButtonDblClick:
            mouseDoubleClickEvent( (QMouseEvent*)e );
            break;

	case QEvent::Paint:
	    paintEvent( (QPaintEvent*)e );
            break;

	default:
	    return FALSE;
	    break;
    }

    return TRUE;
}

void QWSManager::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
	mousePos = e->globalPos();
	dx = 0;
	dy = 0;
	activeRegion = pointInRegion(mousePos);
	switch (activeRegion) {
	    case Menu:
		menu(managed->pos());
		break;
	    case Close:
		closeBtn->setClicked(TRUE);
		break;
	    case Minimize:
		minimizeBtn->setClicked(TRUE);
		break;
	    case Maximize:
		maximizeBtn->setClicked(TRUE);
		break;
	    default:
		break;
	}
	if (activeRegion != None) {
	    active = managed;
	    managed->grabMouse();
	}
	if (activeRegion != None && activeRegion != Close
		&& activeRegion != Minimize && activeRegion != Menu) {
	    managed->raise();
	    managed->setActiveWindow();
	}
    } else if (e->button() == Qt::RightButton) {
	menu(e->globalPos());
    }
}

void QWSManager::mouseReleaseEvent(QMouseEvent *e)
{
    managed->releaseMouse();
    if (e->button() == Qt::LeftButton) {
	handleMove();
	mousePos = e->globalPos();
	Region rgn = pointInRegion(e->globalPos());
	switch (activeRegion) {
	    case Close:
		closeBtn->setClicked(FALSE);
		if (rgn == Close) {
		    close();
		    return;
		}
		break;
	    case Minimize:
		minimizeBtn->setClicked(FALSE);
		if (rgn == Minimize)
		    minimize();
		break;
	    case Maximize:
		maximizeBtn->setClicked(FALSE);
		if (rgn == Maximize)
		    toggleMaximize();
		break;
	    default:
		break;
	}

	activeRegion = None;
    }

    if (activeRegion == None)
	active = 0;
}

void QWSManager::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_CURSOR
    static QCursorShape shape[] = { ArrowCursor, ArrowCursor, ArrowCursor,
			    SizeVerCursor, SizeVerCursor, SizeHorCursor,
			    SizeHorCursor, SizeFDiagCursor, SizeBDiagCursor,
			    SizeBDiagCursor, SizeFDiagCursor, ArrowCursor,
			    ArrowCursor, ArrowCursor, ArrowCursor, ArrowCursor};

    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    if (activeRegion == None)
    {
	if ( !QWidget::mouseGrabber() ) {
	    Region r = pointInRegion(e->globalPos());
	    qwsd->selectCursor(managed, shape[r]);
	}
    } else
	qwsd->selectCursor(managed, shape[activeRegion]);
#endif //QT_NO_CURSOR
    // resize/move regions
    dx = e->globalX() - mousePos.x();
    dy = e->globalY() - mousePos.y();

    if ( ( activeRegion == Title && skipCount > 5 ) || skipCount > 10 )
	handleMove();
    else {
	timer->start(10);
	skipCount++;
    }

    // button regions
    Region r = pointInRegion(e->globalPos());
    menuBtn->setMouseOver(r == Menu);
    closeBtn->setMouseOver(r == Close);
    minimizeBtn->setMouseOver(r == Minimize);
    maximizeBtn->setMouseOver(r == Maximize);
}

void QWSManager::handleMove()
{
    skipCount = 0;
    timer->stop();

    if (!dx && !dy)
	return;

    int x = managed->x();
    int y = managed->y();
    int w = managed->width();
    int h = managed->height();

    QRect geom(managed->geometry());

    switch (activeRegion) {
	case Title:
	    geom = QRect(x + dx, y + dy, w, h);
	    break;
	case Top:
	    geom = QRect(x, y + dy, w, h - dy);
	    break;
	case Bottom:
	    geom = QRect(x, y, w, h + dy);
	    break;
	case Left:
	    geom = QRect(x + dx, y, w - dx, h);
	    break;
	case Right:
	    geom = QRect(x, y, w + dx, h);
	    break;
	case TopRight:
	    geom = QRect(x, y + dy, w + dx, h - dy);
	    break;
	case TopLeft:
	    geom = QRect(x + dx, y + dy, w - dx, h - dy);
	    break;
	case BottomLeft:
	    geom = QRect(x + dx, y, w - dx, h + dy);
	    break;
	case BottomRight:
	    geom = QRect(x, y, w + dx, h + dy);
	    break;
	default:
	    return;
    }

    if (geom.width() >= managed->minimumWidth()
	    && geom.width() <= managed->maximumWidth()) {
	mousePos.setX(mousePos.x() + dx);
    }
    else {
	geom.setX(x);
	geom.setWidth(w);
    }
    if (geom.height() >= managed->minimumHeight()
	    && geom.height() <= managed->maximumHeight()) {
	mousePos.setY(mousePos.y() + dy);
    }
    else {
	geom.setY(y);
	geom.setHeight(h);
    }

    if (geom != managed->geometry()) {
	managed->setGeometry(geom);
    }

    dx = 0;
    dy = 0;
}

void QWSManager::paintEvent(QPaintEvent *)
{
    QWSDecorator &dec = QApplication::qwsDecorator();
    QPainter painter(managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paint(&painter, managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paintButton(&painter, managed, Menu, menuBtn->state());
    dec.paintButton(&painter, managed, Close, closeBtn->state());
    dec.paintButton(&painter, managed, Minimize, minimizeBtn->state());
    dec.paintButton(&painter, managed, Maximize, maximizeBtn->state());
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_COMPLEXWIDGETS
    if (!popup) {
	popup = QApplication::qwsDecorator().menu(managed, managed->pos());
	connect(popup, SIGNAL(activated(int)), SLOT(menuActivated(int)));
    }
    popup->setItemEnabled(Maximize, normalSize.isNull());
    popup->setItemEnabled(Normalize, !normalSize.isNull());
    popup->popup(pos);
#endif
}

void QWSManager::menuActivated(int id)
{
    switch (id) {
	case Close:
	    close();
	    return;
	case Minimize:
	    minimize();
	    break;
	case Maximize:
	case Normalize:
	    toggleMaximize();
	    break;
	case Title:
	    mousePos = QCursor::pos();
	    activeRegion = Title;
	    active = managed;
	    managed->grabMouse();
	    break;
	case BottomRight:
	    mousePos = QCursor::pos();
	    activeRegion = BottomRight;
	    active = managed;
	    managed->grabMouse();
	    break;
	default:
	    break;
    }
}

void QWSManager::close()
{
    active = 0;
    managed->close(FALSE);
}

void QWSManager::minimize()
{
    qDebug("Minimize? What's that?");
}


void QWSManager::maximize()
{
    QRect desk = QApplication::desktop()->rect();
    // find out how much space the decoration needs
    QRegion r = QApplication::qwsDecorator().region(managed, QApplication::desktop()->rect());
    QRect rect = r.boundingRect();
    QRect nr;
    nr.setLeft(-rect.x());
    nr.setTop(-rect.y());
    nr.setRight(desk.right() - rect.right() + desk.right() );
    nr.setBottom(desk.bottom() - rect.bottom() + desk.bottom() );
    managed->setGeometry(nr);
}

void QWSManager::toggleMaximize()
{
    if (normalSize.isNull()) {
	normalSize = managed->geometry();
	maximize();
	maximizeBtn->setOn(TRUE);
    } else {
	managed->setGeometry(normalSize);
	maximizeBtn->setOn(FALSE);
	normalSize = QRect();
    }
}

/*
*/
QWSButton::QWSButton(QWSManager *m, QWSManager::Region t, bool tb)
    : flags(0), toggle(tb), type(t), manager(m)
{
}

void QWSButton::setMouseOver(bool m)
{
    int s = state();
    if (m) flags |= MouseOver;
    else flags &= ~MouseOver;
    if (state() != s)
	paint();
}

void QWSButton::setClicked(bool c)
{
    int s = state();
    if (c) flags |= Clicked;
    else flags &= ~Clicked;
    if (state() != s)
	paint();
}

void QWSButton::setOn(bool o)
{
    int s = state();
    if (o) flags |= On;
    else flags &= ~On;
    if (state() != s)
	paint();
}

void QWSButton::paint()
{
    QWSDecorator &dec = QApplication::qwsDecorator();
    QPainter painter(manager->widget());
    painter.setClipRegion(dec.region(manager->widget(), manager->widget()->rect()));
    dec.paintButton(&painter, manager->widget(), type, state());
}

/*!
  \class QWSDecorator qwsmanager_qws.h
  \brief The QWSDecorator allows the appearance of the Qt/Embedded Window
  Manager to be customised.

  Qt/Embedded provides window management to top level windows.  The
  appearance of the borders and buttons (the decoration) around the
  managed windows can be customized by creating your own class derived
  from QWSDecorator and overriding a few methods.

  This class is non-portable.  It is available \e only in Qt/Embedded.
*/

const QPixmap* QWSDefaultDecorator::pixmapFor(const QWidget* w, QWSManager::Region type, bool on, int& xoff, int& /*yoff*/)
{
#ifndef QT_NO_IMAGEIO_XPM
    static QPixmap *menuPixmap=0;
    static QPixmap *closePixmap=0;
    static QPixmap *minimizePixmap=0;
    static QPixmap *maximizePixmap=0;
    static QPixmap *normalizePixmap=0;
    if ( !closePixmap ) {
	menuPixmap = new QPixmap((const char **)menu_xpm);
	closePixmap = new QPixmap((const char **)close_xpm);
	minimizePixmap = new QPixmap((const char **)minimize_xpm);
	maximizePixmap = new QPixmap((const char **)maximize_xpm);
	normalizePixmap = new QPixmap((const char **)normalize_xpm);
    }
    const QPixmap* pm=0;
    switch (type) {
	case QWSManager::Menu:
	    pm = w->icon();
	    if ( !pm ) {
		xoff = 1;
		pm = menuPixmap;
	    }
	    break;
	case QWSManager::Close:
	    pm = closePixmap;
	    break;
	case QWSManager::Maximize:
	    if (on)
		pm = normalizePixmap;
	    else
		pm = maximizePixmap;
	    break;
	case QWSManager::Minimize:
	    pm = minimizePixmap;
	    break;
	default:
	    break;
    }
    return pm;
#else
    return 0;
#endif    
}

/*!
  \fn QWSDecorator::QWSDecorator()

  Constructs a decorator.
*/

QWSDefaultDecorator::QWSDefaultDecorator()
    : QWSDecorator()
{
}

/*!
  \fn QWSDecorator::~QWSDecorator()

  Destructs a decorator.
*/

QWSDefaultDecorator::~QWSDefaultDecorator()
{
}

/*!
  \fn QWSDecorator::region( const QRect &rect, QWSManager::Region type )

  Returns the requested region \a type which will contain \a rect.
*/

QRegion QWSDefaultDecorator::region(const QWidget *, const QRect &rect, QWSManager::Region type)
{
    QRegion region;

    switch (type) {
	case QWSManager::All: {
		QRect r(rect.left() - BORDER_WIDTH,
			rect.top() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.width() + 2 * BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT + 2 * BORDER_WIDTH);
		region = r;
		region -= rect;
	    }
	    break;

	case QWSManager::Title: {
		QRect r(rect.left() + TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			rect.width() - 4*TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.width() > 0)
		    region = r;
	    }
	    break;

	case QWSManager::Top: {
		QRect r(rect.left() + CORNER_GRAB,
			rect.top() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.width() - 2 * CORNER_GRAB,
			BORDER_WIDTH);
		region = r;
	    }
	    break;

	case QWSManager::Left: {
		QRect r(rect.left() - BORDER_WIDTH,
			rect.top() - TITLE_HEIGHT + CORNER_GRAB,
			BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT - 2 * CORNER_GRAB);
		region = r;
	    }
	    break;

	case QWSManager::Right: {
		QRect r(rect.right() + 1,
			rect.top() - TITLE_HEIGHT + CORNER_GRAB,
			BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT - 2 * CORNER_GRAB);
		region = r;
	    }
	    break;

	case QWSManager::Bottom: {
		QRect r(rect.left() + CORNER_GRAB,
			rect.bottom() + 1,
			rect.width() - 2 * CORNER_GRAB,
			BORDER_WIDTH);
		region = r;
	    }
	    break;

	case QWSManager::TopLeft: {
		QRect r1(rect.left() - BORDER_WIDTH,
			rect.top() - BORDER_WIDTH - TITLE_HEIGHT,
			CORNER_GRAB + BORDER_WIDTH,
			BORDER_WIDTH);

		QRect r2(rect.left() - BORDER_WIDTH,
			rect.top() - BORDER_WIDTH - TITLE_HEIGHT,
			BORDER_WIDTH,
			CORNER_GRAB + BORDER_WIDTH);

		region = QRegion(r1) + r2;
	    }
	    break;

	case QWSManager::TopRight: {
		QRect r1(rect.right() - CORNER_GRAB,
			rect.top() - BORDER_WIDTH - TITLE_HEIGHT,
			CORNER_GRAB + BORDER_WIDTH,
			BORDER_WIDTH);

		QRect r2(rect.right() + 1,
			rect.top() - BORDER_WIDTH - TITLE_HEIGHT,
			BORDER_WIDTH,
			CORNER_GRAB + BORDER_WIDTH);

		region = QRegion(r1) + r2;
	    }
	    break;

	case QWSManager::BottomLeft: {
		QRect r1(rect.left() - BORDER_WIDTH,
			rect.bottom() + 1,
			CORNER_GRAB + BORDER_WIDTH,
			BORDER_WIDTH);

		QRect r2(rect.left() - BORDER_WIDTH,
			rect.bottom() - CORNER_GRAB,
			BORDER_WIDTH,
			CORNER_GRAB + BORDER_WIDTH);
		region = QRegion(r1) + r2;
	    }
	    break;

	case QWSManager::BottomRight: {
		QRect r1(rect.right() - CORNER_GRAB,
			rect.bottom() + 1,
			CORNER_GRAB + BORDER_WIDTH,
			BORDER_WIDTH);

		QRect r2(rect.right() + 1,
			rect.bottom() - CORNER_GRAB,
			BORDER_WIDTH,
			CORNER_GRAB + BORDER_WIDTH);
		region = QRegion(r1) + r2;
	    }
	    break;

	case QWSManager::Menu: {
		QRect r(rect.left(), rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		region = r;
	    }
	    break;

	case QWSManager::Close: {
		QRect r(rect.right() - TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.left() > rect.left() + TITLE_HEIGHT)
		    region = r;
	    }
	    break;

	case QWSManager::Maximize: {
		QRect r(rect.right() - 2*TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.left() > rect.left() + TITLE_HEIGHT)
		    region = r;
	    }
	    break;

	case QWSManager::Minimize: {
		QRect r(rect.right() - 3*TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.left() > rect.left() + TITLE_HEIGHT)
		    region = r;
	    }
	    break;

	default:
	    break;
    }

    return region;
}

#ifndef QT_NO_COMPLEXWIDGETS
QPopupMenu *QWSDecorator::menu(const QWidget *, const QPoint &)
{
    QPopupMenu *m = new QPopupMenu();

    m->insertItem(QObject::tr("&Restore"), (int)QWSManager::Normalize);
    m->insertItem(QObject::tr("&Move"), (int)QWSManager::Title);
    m->insertItem(QObject::tr("&Size"), (int)QWSManager::BottomRight);
    m->insertItem(QObject::tr("Mi&nimize"), (int)QWSManager::Minimize);
    m->insertItem(QObject::tr("Ma&ximize"), (int)QWSManager::Maximize);
    m->insertSeparator();
    m->insertItem(QObject::tr("Close"), (int)QWSManager::Close);

    return m;
}
#endif

/*!
  \fn QWSDecorator::paint( QPainter *painter, const QWidget *widget )

  Paints the border and title decoration around \a widget using \a painter.

*/

void QWSDefaultDecorator::paint(QPainter *painter, const QWidget *widget)
{
#ifndef QT_NO_COMPLEXWIDGETS // implies style    
    QStyle &style = QApplication::style();
#endif
    
    QRect r(widget->rect().left() - BORDER_WIDTH,
	    widget->rect().top() - BORDER_WIDTH - TITLE_HEIGHT,
	    widget->rect().width() + 2*BORDER_WIDTH,
	    widget->rect().height() + 2*BORDER_WIDTH + TITLE_HEIGHT);

#ifndef QT_NO_PALETTE
    const QColorGroup &cg = widget->palette().active();

#if !defined(QT_NO_COMPLEXWIDGETS)
    style.drawPanel(painter, r.x(), r.y(), r.width(),
		    r.height(), cg, FALSE, 2,
		    &cg.brush(QColorGroup::Background));
#elif !defined(QT_NO_DRAWUTIL)
    qDrawWinPanel(painter, r.x(), r.y(), r.width(),
		  r.height(), cg, FALSE,
		  &cg.brush(QColorGroup::Background));
#endif
    
    int titleWidth = widget->width()-4*TITLE_HEIGHT-4;
    if (titleWidth > 0) {
	QBrush titleBrush;
	QPen   titlePen;

	if (widget == qApp->activeWindow()) {
	    titleBrush = cg.brush(QColorGroup::Highlight);
	    titlePen   = cg.color(QColorGroup::HighlightedText);
	} else {
	    titleBrush = cg.brush(QColorGroup::Background);
	    titlePen   = cg.color(QColorGroup::Text);
	}

#if !defined(QT_NO_COMPLEXWIDGETS)
	style.drawPanel(painter, TITLE_HEIGHT, -TITLE_HEIGHT,
			titleWidth, TITLE_HEIGHT - 1,
			cg, TRUE, 1, &titleBrush);
#elif !defined(QT_NO_DRAWUTIL)
	qDrawWinPanel(painter, TITLE_HEIGHT, -TITLE_HEIGHT,
			titleWidth, TITLE_HEIGHT - 1,
			cg, TRUE, &titleBrush);
#endif		
	painter->setPen(titlePen);
	painter->setFont(widget->font());
	painter->drawText(TITLE_HEIGHT + 4, -TITLE_HEIGHT,
			titleWidth-5, TITLE_HEIGHT - 1,
			QPainter::AlignVCenter, widget->caption());
    }

#endif //QT_NO_PALETTE

}

/*!
  \fn QWSDecorator::paintButton( QPainter *painter, const QRect &rect,
                        QWSManager::Region type, int state )

  Paints a button \a type using \a painter.
  \a rect contains the widget whose button is to be drawn.
  \a state is the state of the button.

*/

void QWSDefaultDecorator::paintButton(QPainter *painter, const QWidget *w,
			QWSManager::Region type, int state)
{
#ifndef QT_NO_PALETTE    
#ifndef QT_NO_COMPLEXWIDGETS
    QStyle &style = QApplication::style();
#endif
    const QColorGroup &cg = w->palette().active();

    QRect brect(region(w, w->rect(), type).boundingRect());
    int xoff=2;
    int yoff=2;
    const QPixmap *pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
#if !defined(QT_NO_COMPLEXWIDGETS)
	style.drawToolButton(painter, brect.x(), brect.y(), brect.width()-1,
		    brect.height()-1, cg, TRUE,
		    &cg.brush(QColorGroup::Background));
#elif !defined(QT_NO_DRAWUTIL)
	qDrawWinPanel(painter, brect.x(), brect.y(), brect.width()-1,
		    brect.height()-1, cg, TRUE,
		    &cg.brush(QColorGroup::Background));
#endif	
	if (pm) painter->drawPixmap(brect.x()+xoff+1, brect.y()+yoff+1, *pm);
    } else {
	painter->fillRect(brect.x(), brect.y(), brect.width()-1,
                    brect.height()-1, cg.brush(QColorGroup::Background));
	if (pm) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, *pm);
    }
#endif
}

#endif // QT_NO_QWS_MANAGER
