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


/*!
  \class QWSDecoration qwsmanager_qws.h
  \brief The QWSDecoration allows the appearance of the Qt/Embedded Window
  Manager to be customised.

  Qt/Embedded provides window management to top level windows.  The
  appearance of the borders and buttons (the decoration) around the
  managed windows can be customized by creating your own class derived
  from QWSDecoration and overriding a few methods.

  This class is non-portable.  It is available \e only in Qt/Embedded.

  \sa QApplication::qwsSetDecoration()
*/

/*!
  \fn QWSDecoration::QWSDecoration()

  Constructs a decorator.
*/

/*!
  \fn QWSDecoration::~QWSDecoration()

  Destructs a decorator.
*/

/*!
  \enum QWSDecoration::Region

  This enum describes the regions in the window decorations.

  <ul>
  <li> \c None - used internally.
  <li> \c All - the entire region used by the window decoration.
  <li> \c Title - Displays the window title and allows the window to be
	  moved by dragging.
  <li> \c Top - allows the top of the window to be resized.
  <li> \c Bottom - allows the bottom of the window to be resized.
  <li> \c Left - allows the left edge of the window to be resized.
  <li> \c Right - allows the right edge of the window to be resized.
  <li> \c TopLeft - allows the top-left of the window to be resized.
  <li> \c TopRight - allows the top-right of the window to be resized.
  <li> \c BottomLeft - allows the bottom-left of the window to be resized.
  <li> \c BottomRight - allows the bottom-right of the window to be resized.
  <li> \c Close - clicking in this region closes the window.
  <li> \c Minimize - clicking in this region minimizes the window.
  <li> \c Maximize - clicking in this region maximizes the window.
  <li> \c Normalize - returns a maximized window to previous size.
  <li> \c Menu - clicking in this region opens the window operations menu.
  </ul>
*/

/*!
  \fn QRegion QWSDecoration::region( const QWidget *widget, const QRect &rect, Region type )

  Returns the requested region \a type which will contain \a widget
  with geometry \a rect.
*/

/*!
  Called when the user clicks in the \c Close region.

  \a widget is the QWidget to be closed.

  The default behaviour is to close the widget.
*/
void QWSDecoration::close( QWidget *widget )
{
    widget->close(FALSE);
}

/*!
  Called when the user clicks in the \c Minimize region.

  \a widget is the QWidget to be minimized.

  The default behaviour is to ignore this action.
*/
void QWSDecoration::minimize( QWidget *widget )
{
    qDebug("No minimize functionality provided");
}


/*!
  Called when the user clicks in the \c Maximize region.

  \a widget is the QWidget to be maximized.

  The default behaviour is to resize the widget to be full-screen.
  This method can be overridden to, e.g. avoid launch panels.
*/
void QWSDecoration::maximize( QWidget *widget )
{
    QRect desk = QApplication::desktop()->rect();
    // find out how much space the decoration needs
    QRegion r = region(widget, QApplication::desktop()->rect());
    QRect rect = r.boundingRect();
    QRect nr;
    nr.setLeft(-rect.x());
    nr.setTop(-rect.y());
    nr.setRight(desk.right() - rect.right() + desk.right() );
    nr.setBottom(desk.bottom() - rect.bottom() + desk.bottom() );
    widget->setGeometry(nr);
}

/*!
  Called to create a QPopupMenu containing the valid menu operations.

  The default implementation adds all possible window operations.
*/

#ifndef QT_NO_COMPLEXWIDGETS
QPopupMenu *QWSDecoration::menu(const QWidget *, const QPoint &)
{
    QPopupMenu *m = new QPopupMenu();

    m->insertItem(QObject::tr("&Restore"), (int)Normalize);
    m->insertItem(QObject::tr("&Move"), (int)Title);
    m->insertItem(QObject::tr("&Size"), (int)BottomRight);
    m->insertItem(QObject::tr("Mi&nimize"), (int)Minimize);
    m->insertItem(QObject::tr("Ma&ximize"), (int)Maximize);
    m->insertSeparator();
    m->insertItem(QObject::tr("Close"), (int)Close);

    return m;
}
#endif

/*!
  \fn void QWSDecoration::paint( QPainter *painter, const QWidget *widget )

  Override to paint the border and title decoration around \a widget using
  \a painter.

*/

/*!
  \fn void QWSDecoration::paintButton( QPainter *painter, const QWidget *widget, Region type, int state )

  Override to paint a button \a type using \a painter.

  \a widget is the widget whose button is to be drawn.
  \a state is the state of the button.  It can be a combination of the
  following ORed together:
  <ul>
  <li> \c QWSButton::MouseOver
  <li> \c QWSButton::Clicked
  <li> \c QWSButton::On
  </ul>
*/


QWidget *QWSManager::active = 0;
QPoint QWSManager::mousePos;

QWSManager::QWSManager(QWidget *w)
    : QObject(), activeRegion(QWSDecoration::None), managed(w), popup(0),
      timer(0)
{
    dx = 0;
    dy = 0;
    skipCount = 0;

    menuBtn = new QWSButton(this, QWSDecoration::Menu);
    closeBtn = new QWSButton(this, QWSDecoration::Close);
    minimizeBtn = new QWSButton(this, QWSDecoration::Minimize);
    maximizeBtn = new QWSButton(this, QWSDecoration::Maximize, TRUE);

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
    return QApplication::qwsDecoration().region(managed, managed->geometry());
}

QWSDecoration::Region QWSManager::pointInRegion(const QPoint &p)
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QRect rect(managed->geometry());

    for (int i = QWSDecoration::Title; i <= QWSDecoration::LastRegion; i++) {
	if (dec.region(managed, rect, (QWSDecoration::Region)i).contains(p))
	    return (QWSDecoration::Region)i;
    }

    return QWSDecoration::None;
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
	    case QWSDecoration::Menu:
		menu(managed->pos());
		break;
	    case QWSDecoration::Close:
		closeBtn->setClicked(TRUE);
		break;
	    case QWSDecoration::Minimize:
		minimizeBtn->setClicked(TRUE);
		break;
	    case QWSDecoration::Maximize:
		maximizeBtn->setClicked(TRUE);
		break;
	    default:
		break;
	}
	if (activeRegion != QWSDecoration::None) {
	    active = managed;
	    managed->grabMouse();
	}
	if ( activeRegion != QWSDecoration::None &&
	     activeRegion != QWSDecoration::Close &&
	     activeRegion != QWSDecoration::Minimize &&
	     activeRegion != QWSDecoration::Menu) {
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
	QWSDecoration::Region rgn = pointInRegion(e->globalPos());
	switch (activeRegion) {
	    case QWSDecoration::Close:
		closeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Close) {
		    close();
		    return;
		}
		break;
	    case QWSDecoration::Minimize:
		minimizeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Minimize)
		    minimize();
		break;
	    case QWSDecoration::Maximize:
		maximizeBtn->setClicked(FALSE);
		if (rgn == QWSDecoration::Maximize)
		    toggleMaximize();
		break;
	    default:
		break;
	}

	activeRegion = QWSDecoration::None;
    }

    if (activeRegion == QWSDecoration::None)
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
    if (activeRegion == QWSDecoration::None)
    {
	if ( !QWidget::mouseGrabber() ) {
	    QWSDecoration::Region r = pointInRegion(e->globalPos());
	    qwsd->selectCursor(managed, shape[r]);
	}
    } else
	qwsd->selectCursor(managed, shape[activeRegion]);
#endif //QT_NO_CURSOR
    // resize/move regions
    dx = e->globalX() - mousePos.x();
    dy = e->globalY() - mousePos.y();

    if ( ( activeRegion == QWSDecoration::Title && skipCount > 5 ) || skipCount > 10 )
	handleMove();
    else {
	timer->start(10);
	skipCount++;
    }

    // button regions
    QWSDecoration::Region r = pointInRegion(e->globalPos());
    menuBtn->setMouseOver(r == QWSDecoration::Menu);
    closeBtn->setMouseOver(r == QWSDecoration::Close);
    minimizeBtn->setMouseOver(r == QWSDecoration::Minimize);
    maximizeBtn->setMouseOver(r == QWSDecoration::Maximize);
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
	case QWSDecoration::Title:
	    geom = QRect(x + dx, y + dy, w, h);
	    break;
	case QWSDecoration::Top:
	    geom = QRect(x, y + dy, w, h - dy);
	    break;
	case QWSDecoration::Bottom:
	    geom = QRect(x, y, w, h + dy);
	    break;
	case QWSDecoration::Left:
	    geom = QRect(x + dx, y, w - dx, h);
	    break;
	case QWSDecoration::Right:
	    geom = QRect(x, y, w + dx, h);
	    break;
	case QWSDecoration::TopRight:
	    geom = QRect(x, y + dy, w + dx, h - dy);
	    break;
	case QWSDecoration::TopLeft:
	    geom = QRect(x + dx, y + dy, w - dx, h - dy);
	    break;
	case QWSDecoration::BottomLeft:
	    geom = QRect(x + dx, y, w - dx, h + dy);
	    break;
	case QWSDecoration::BottomRight:
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
    QWSDecoration &dec = QApplication::qwsDecoration();
    QPainter painter(managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paint(&painter, managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paintButton(&painter, managed, QWSDecoration::Menu, menuBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Close, closeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Minimize, minimizeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Maximize, maximizeBtn->state());
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_COMPLEXWIDGETS
    if (!popup) {
	popup = QApplication::qwsDecoration().menu(managed, managed->pos());
	connect(popup, SIGNAL(activated(int)), SLOT(menuActivated(int)));
    }
    popup->setItemEnabled(QWSDecoration::Maximize, normalSize.isNull());
    popup->setItemEnabled(QWSDecoration::Normalize, !normalSize.isNull());
    popup->popup(pos);
#endif
}

void QWSManager::menuActivated(int id)
{
    switch (id) {
	case QWSDecoration::Close:
	    close();
	    return;
	case QWSDecoration::Minimize:
	    minimize();
	    break;
	case QWSDecoration::Maximize:
	case QWSDecoration::Normalize:
	    toggleMaximize();
	    break;
	case QWSDecoration::Title:
	    mousePos = QCursor::pos();
	    activeRegion = QWSDecoration::Title;
	    active = managed;
	    managed->grabMouse();
	    break;
	case QWSDecoration::BottomRight:
	    mousePos = QCursor::pos();
	    activeRegion = QWSDecoration::BottomRight;
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
    QApplication::qwsDecoration().close(managed);
}

void QWSManager::minimize()
{
    QApplication::qwsDecoration().minimize(managed);
}


void QWSManager::maximize()
{
    QApplication::qwsDecoration().maximize(managed);
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
QWSButton::QWSButton(QWSManager *m, QWSDecoration::Region t, bool tb)
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
    QWSDecoration &dec = QApplication::qwsDecoration();
    QPainter painter(manager->widget());
    painter.setClipRegion(dec.region(manager->widget(), manager->widget()->rect()));
    dec.paintButton(&painter, manager->widget(), type, state());
}

const QPixmap* QWSDefaultDecoration::pixmapFor(const QWidget* w, QWSDecoration::Region type, bool on, int& xoff, int& /*yoff*/)
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
	case Menu:
	    pm = w->icon();
	    if ( !pm ) {
		xoff = 1;
		pm = menuPixmap;
	    }
	    break;
	case Close:
	    pm = closePixmap;
	    break;
	case Maximize:
	    if (on)
		pm = normalizePixmap;
	    else
		pm = maximizePixmap;
	    break;
	case Minimize:
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


QWSDefaultDecoration::QWSDefaultDecoration()
    : QWSDecoration()
{
}


QWSDefaultDecoration::~QWSDefaultDecoration()
{
}

QRegion QWSDefaultDecoration::region(const QWidget *, const QRect &rect, QWSDecoration::Region type)
{
    QRegion region;

    switch (type) {
	case All: {
		QRect r(rect.left() - BORDER_WIDTH,
			rect.top() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.width() + 2 * BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT + 2 * BORDER_WIDTH);
		region = r;
		region -= rect;
	    }
	    break;

	case Title: {
		QRect r(rect.left() + TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			rect.width() - 4*TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.width() > 0)
		    region = r;
	    }
	    break;

	case Top: {
		QRect r(rect.left() + CORNER_GRAB,
			rect.top() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.width() - 2 * CORNER_GRAB,
			BORDER_WIDTH);
		region = r;
	    }
	    break;

	case Left: {
		QRect r(rect.left() - BORDER_WIDTH,
			rect.top() - TITLE_HEIGHT + CORNER_GRAB,
			BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT - 2 * CORNER_GRAB);
		region = r;
	    }
	    break;

	case Right: {
		QRect r(rect.right() + 1,
			rect.top() - TITLE_HEIGHT + CORNER_GRAB,
			BORDER_WIDTH,
			rect.height() + TITLE_HEIGHT - 2 * CORNER_GRAB);
		region = r;
	    }
	    break;

	case Bottom: {
		QRect r(rect.left() + CORNER_GRAB,
			rect.bottom() + 1,
			rect.width() - 2 * CORNER_GRAB,
			BORDER_WIDTH);
		region = r;
	    }
	    break;

	case TopLeft: {
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

	case TopRight: {
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

	case BottomLeft: {
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

	case BottomRight: {
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

	case Menu: {
		QRect r(rect.left(), rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		region = r;
	    }
	    break;

	case Close: {
		QRect r(rect.right() - TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.left() > rect.left() + TITLE_HEIGHT)
		    region = r;
	    }
	    break;

	case Maximize: {
		QRect r(rect.right() - 2*TITLE_HEIGHT, rect.top() - TITLE_HEIGHT,
			TITLE_HEIGHT, TITLE_HEIGHT);
		if (r.left() > rect.left() + TITLE_HEIGHT)
		    region = r;
	    }
	    break;

	case Minimize: {
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

void QWSDefaultDecoration::paint(QPainter *painter, const QWidget *widget)
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

void QWSDefaultDecoration::paintButton(QPainter *painter, const QWidget *w,
			QWSDecoration::Region type, int state)
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
