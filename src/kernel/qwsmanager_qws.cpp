
#include "qapplication.h"
#include "qaccel.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qpopupmenu.h"
#include "qpainter.h"
#include "qregion.h"
#include "qevent.h"
#include "qtimer.h"
#include "qgfx.h"
#include "qwsmanager.h"

#define CORNER_GRAB	16
#define BORDER_WIDTH	4
#define TITLE_HEIGHT	20

// #### merge two copies of pixmaps

static const char * const menu_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"  	c None",
". 	c #A3C511",
"+ 	c #A2C511",
"@ 	c #A2C611",
"# 	c #A2C510",
"$ 	c #A2C513",
"% 	c #A2C412",
"& 	c #A2C413",
"* 	c #A2C414",
"= 	c #A2C515",
"- 	c #A2C50F",
"; 	c #A3C510",
"> 	c #A2C410",
", 	c #A2C411",
"' 	c #A2C314",
") 	c #A2C316",
"! 	c #A2C416",
"~ 	c #A0C315",
"{ 	c #A1C313",
"] 	c #A1C412",
"^ 	c #A2C40F",
"/ 	c #A1C410",
"( 	c #A0C510",
"_ 	c #A0C511",
": 	c #A1C414",
"< 	c #9FC30E",
"[ 	c #98B51B",
"} 	c #5F7609",
"| 	c #5C6E0E",
"1 	c #5B6E10",
"2 	c #5C6C14",
"3 	c #5A6E0A",
"4 	c #839E16",
"5 	c #A0C515",
"6 	c #A0C513",
"7 	c #A2C512",
"8 	c #A1C512",
"9 	c #A1C511",
"0 	c #A1C50F",
"a 	c #91AE12",
"b 	c #505E11",
"c 	c #1F2213",
"d 	c #070606",
"e 	c #040204",
"f 	c #040306",
"g 	c #15160F",
"h 	c #2F3A0D",
"i 	c #859F1B",
"j 	c #A1C215",
"k 	c #A0C50F",
"l 	c #A1C510",
"m 	c #A0C110",
"n 	c #839C1B",
"o 	c #1E240A",
"p 	c #050205",
"q 	c #030304",
"r 	c #323917",
"s 	c #556313",
"t 	c #56680B",
"u 	c #536609",
"v 	c #4A561B",
"w 	c #0B0D04",
"x 	c #030208",
"y 	c #090A05",
"z 	c #5F6F18",
"A 	c #A0C117",
"B 	c #91AF10",
"C 	c #1E2209",
"D 	c #030205",
"E 	c #17190D",
"F 	c #7D981C",
"G 	c #9ABA12",
"H 	c #A3C411",
"I 	c #A3C713",
"J 	c #95B717",
"K 	c #7F9A18",
"L 	c #8FAE1B",
"M 	c #394413",
"N 	c #040305",
"O 	c #090807",
"P 	c #6C7E19",
"Q 	c #A6C614",
"R 	c #A1C411",
"S 	c #64761F",
"T 	c #030105",
"U 	c #070707",
"V 	c #728513",
"W 	c #A2C40C",
"X 	c #A2C70B",
"Y 	c #89A519",
"Z 	c #313B11",
"` 	c #101409",
" .	c #586A19",
"..	c #97B620",
"+.	c #1B2207",
"@.	c #282D11",
"#.	c #A6C41B",
"$.	c #A1C413",
"%.	c #A3C512",
"&.	c #2E370B",
"*.	c #030108",
"=.	c #21260F",
"-.	c #A5C21A",
";.	c #A0C60D",
">.	c #6D841A",
",.	c #0F1007",
"'.	c #040207",
").	c #0E1009",
"!.	c #515F14",
"~.	c #A2C41B",
"{.	c #5E701B",
"].	c #030203",
"^.	c #0B0B04",
"/.	c #87A111",
"(.	c #A0C411",
"_.	c #A0C316",
":.	c #212907",
"<.	c #222C0B",
"[.	c #A3C516",
"}.	c #9CBE1A",
"|.	c #5E6F1B",
"1.	c #0E0F0B",
"2.	c #040205",
"3.	c #181B0D",
"4.	c #93AE25",
"5.	c #A0C610",
"6.	c #617715",
"7.	c #030306",
"8.	c #070704",
"9.	c #809818",
"0.	c #A1C415",
"a.	c #475416",
"b.	c #030309",
"c.	c #12170B",
"d.	c #91B01E",
"e.	c #5C721F",
"f.	c #05050B",
"g.	c #33371D",
"h.	c #0E0F08",
"i.	c #040405",
"j.	c #758921",
"k.	c #46511B",
"l.	c #030207",
"m.	c #131409",
"n.	c #9FB921",
"o.	c #859D21",
"p.	c #080809",
"q.	c #030305",
"r.	c #46521C",
"s.	c #8EB017",
"t.	c #627713",
"u.	c #4D5F17",
"v.	c #97B71D",
"w.	c #77901D",
"x.	c #151708",
"y.	c #0D0D0B",
"z.	c #0C0B08",
"A.	c #455216",
"B.	c #A5C616",
"C.	c #A0C114",
"D.	c #556118",
"E.	c #050307",
"F.	c #050407",
"G.	c #363E17",
"H.	c #5D7309",
"I.	c #A2BF28",
"J.	c #A2C417",
"K.	c #A4C620",
"L.	c #60701D",
"M.	c #030103",
"N.	c #030303",
"O.	c #809A1B",
"P.	c #A0C310",
"Q.	c #A0C410",
"R.	c #A3C415",
"S.	c #9CB913",
"T.	c #6F801F",
"U.	c #1A210A",
"V.	c #1D1E0D",
"W.	c #1D220F",
"X.	c #1E210F",
"Y.	c #0F0F07",
"Z.	c #0E1007",
"`.	c #090906",
" +	c #2B360E",
".+	c #97B813",
"++	c #A2C50E",
"@+	c #A5C517",
"#+	c #90AD20",
"$+	c #5D6C1A",
"%+	c #394115",
"&+	c #050704",
"*+	c #040304",
"=+	c #202807",
"-+	c #5E6B21",
";+	c #728D0C",
">+	c #65791D",
",+	c #29330F",
"'+	c #7A911D",
")+	c #A2C614",
"!+	c #A1C513",
"~+	c #A3C50E",
"{+	c #A3C414",
"]+	c #9CBD11",
"^+	c #95B40C",
"/+	c #94B50F",
"(+	c #95B510",
"_+	c #99B913",
":+	c #A0C414",
"<+	c #9ABC11",
"[+	c #A0C314",
"}+	c #A1C40F",
"|+	c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};


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
#if QT_FEATURE_WIDGETS
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
    static QCursorShape shape[] = { ArrowCursor, ArrowCursor, ArrowCursor,
			    SizeVerCursor, SizeVerCursor, SizeHorCursor,
			    SizeHorCursor, SizeFDiagCursor, SizeBDiagCursor,
			    SizeBDiagCursor, SizeFDiagCursor, ArrowCursor,
			    ArrowCursor, ArrowCursor, ArrowCursor, ArrowCursor};

    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    if (activeRegion == None)
    {
	Region r = pointInRegion(e->globalPos());
	qwsd->selectCursor(managed->winId(), shape[r]);
    } else
	qwsd->selectCursor(managed->winId(), shape[activeRegion]);

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
#if QT_FEATURE_WIDGETS
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
  \class QWSDecorator qwsmanager.h
  \brief The QWSDecorator allows the appearance of the Qt/Embedded Window
  Manager to be customised.

  Qt/Embedded provides window management to top level windows.  The
  appearance of the borders and buttons (the decoration) around the
  managed windows can be customized by creating your own class derived
  from QWSDecorator and overriding a few methods.

  This class is non-portable.  It is available \e only in Qt/Embedded.
*/

const QPixmap* QWSDefaultDecorator::pixmapFor(const QWidget* w, QWSManager::Region type, bool on, int& xoff, int& yoff)
{
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
		xoff = -1;
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
	    ;
    }
    return pm;
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

#if QT_FEATURE_WIDGETS
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
    QStyle &style = QApplication::style();

    const QColorGroup &cg = widget->palette().active();

    QRect r(widget->rect().left() - BORDER_WIDTH,
	    widget->rect().top() - BORDER_WIDTH - TITLE_HEIGHT,
	    widget->rect().width() + 2*BORDER_WIDTH,
	    widget->rect().height() + 2*BORDER_WIDTH + TITLE_HEIGHT);

    style.drawPanel(painter, r.x(), r.y(), r.width(),
		    r.height(), cg, FALSE, 2,
		    &cg.brush(QColorGroup::Background));

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

	style.drawPanel(painter, TITLE_HEIGHT, -TITLE_HEIGHT,
			titleWidth, TITLE_HEIGHT - 1,
			cg, TRUE, 1, &titleBrush);

	painter->setPen(titlePen);
	painter->setFont(widget->font());
	painter->drawText(TITLE_HEIGHT + 4, -TITLE_HEIGHT,
			titleWidth-5, TITLE_HEIGHT - 1,
			QPainter::AlignVCenter, widget->caption());
    }
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
    QStyle &style = QApplication::style();
    const QColorGroup &cg = w->palette().active();

    QRect brect(region(w, w->rect(), type).boundingRect());
    int xoff=2;
    int yoff=2;
    const QPixmap *pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
	style.drawToolButton(painter, brect.x(), brect.y(), brect.width()-1,
		    brect.height()-1, cg, TRUE,
		    &cg.brush(QColorGroup::Background));
	if (pm) painter->drawPixmap(brect.x()+xoff+1, brect.y()+yoff+1, *pm);
    } else {
	painter->fillRect(brect.x(), brect.y(), brect.width()-1,
                    brect.height()-1, cg.brush(QColorGroup::Background));
	if (pm) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, *pm);
    }
}

