#include <qapplication.h>
#include <qmotifstyle.h>
#include <qwindowsstyle.h>
#include <qmainwindow.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qtabdialog.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qspinbox.h>
#include <qtableview.h>
#include <qtabwidget.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabbar.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qsizegrip.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qiconview.h>
#include <qworkspace.h>
#include <qprintdialog.h>
#include <qprinter.h>
#define private public // huhu :-))
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qkeycode.h>

#include <life.h>
#include "../../examples/dirview/dirview.h"

#include <unistd.h> // for sleep()
#include <stdlib.h>


/* XPM */
static const char *image_xpm[] = {
"17 15 9 1",
" 	c #7F7F7F",
".	c #FFFFFF",
"X	c #00B6FF",
"o	c #BFBFBF",
"O	c #FF6C00",
"+	c #000000",
"@	c #0000FF",
"#	c #6CFF00",
"$	c #FFB691",
"             ..XX",
" ........o   .XXX",
" .OOOOOOOo.  XXX+",
" .O@@@@@@+++XXX++",
" .O@@@@@@O.XXX+++",
" .O@@@@@@OXXX+++.",
" .O######XXX++...",
" .O#####XXX++....",
" .O##$#$XX+o+....",
" .O#$$$$$+.o+....",
" .O##$$##O.o+....",
" .OOOOOOOO.o+....",
" ..........o+....",
" ooooooooooo+....",
"+++++++++++++...."
};

class ImageIconProvider : public QFileIconProvider
{
    Q_OBJECT
    QStrList fmts;
    QPixmap imagepm;

public:
    ImageIconProvider( QWidget *parent=0, const char *name=0 );
    ~ImageIconProvider();

    const QPixmap * pixmap( const QFileInfo &fi );
};

ImageIconProvider::ImageIconProvider( QWidget *parent, const char *name ) :
    QFileIconProvider( parent, name ),
    imagepm(image_xpm)
{
    fmts = QImage::inputFormats();
}

ImageIconProvider::~ImageIconProvider()
{
}

const QPixmap * ImageIconProvider::pixmap( const QFileInfo &fi )
{
    QString ext = fi.extension().upper();
    if ( fmts.contains(ext) ) {
	return &imagepm;
    } else {
	return QFileIconProvider::pixmap(fi);
    }
}

class WidgetDepicter : QObject
{
    QWidget* widget;
    QString filename;
    QString suffix;
    bool includeframe;
    bool done;

public:
    WidgetDepicter( const char* suf) :
	widget(0),
	suffix(suf)
    {
	done = true;
    }
    ~WidgetDepicter()
    {
	if (widget) widget->hide();
    }

    void depict(QProgressDialog* w, const char* savefile,
		const char* widgetname)
    // Special case QDialog:  need to get it visible first
    {
	w->setProgress(0);
	sleep(3);
	w->setProgress(33);
	sleep(1);
	w->setProgress(34);
	depict((QDialog*)w, savefile, widgetname);
    }

    void depict(QDialog* w, const char* savefile, const char* widgetname)
    // Special case QWidget:  want WM frame
    {
	depict(w, savefile, widgetname, true);
    }

    void depict(QWidget* w, const char* savefile, const char* widgetname,
		bool frame=false)
    {
	if (widget) widget->hide();
	widget=w;
	includeframe=frame;
	filename=savefile + suffix;
	widget->setCaption(widgetname);
	widget->show();
	done = false;
	startTimer(800);
	while ( !done ) {
	    qApp->processEvents();
	}
	widget->hide();
	killTimers();
    }

    void timerEvent(QTimerEvent*)
    {
	if ( !done ) {
	    done = true;
	    takePicture();
	}
    }

    void takePicture()
    {
	const int margin = 8;

	QRect r = includeframe ? widget->frameGeometry() : widget->geometry();
	QPixmap pm;
	if ( includeframe ) {
	    pm = QPixmap::grabWindow( QApplication::desktop()->winId(),
		    r.x(), r.y(), r.width(), r.height() );
	} else {
	    pm = QPixmap::grabWidget( widget );
	}

	if (includeframe) {
	    pm.save( filename, "PNG" );
	} else {
	    QPixmap ppm(r.width()+2*margin, r.height()+2*margin);
	    ppm.fill(QApplication::palette().normal().background());
	    QPainter p;
	    p.begin(&ppm);
	    p.drawPixmap(margin,margin,pm);
	    p.end();
	    ppm.save( filename, "PNG" );
	}
    }
};


class EgQCheckBox : public QWidget {
    QCheckBox cb1, cb2, cb3;
public:
    EgQCheckBox() :
	cb1("First",this),
	cb2("Second",this),
	cb3("Third",this)
    {
	cb1.setGeometry(0,0,80,25);
	cb2.setGeometry(0,25,80,25);
	cb3.setGeometry(0,50,80,25);
	cb1.setChecked(true);
	resize(80,75);
    }
};


class EgQPushButton : public QPushButton {
public:
    EgQPushButton() :
	QPushButton( "Press Me", 0 )
    {
	resize(80,25);
    }
};


class EgQRadioButton : public QWidget {
    QRadioButton rb1, rb2, rb3;
public:
    EgQRadioButton(QWidget* parent=0) :
	QWidget(parent),
	rb1("First",this),
	rb2("Second",this),
	rb3("Third",this)
    {
	rb1.setGeometry(0,0,80,25);
	rb2.setGeometry(0,25,80,25);
	rb3.setGeometry(0,50,80,25);
	rb1.setChecked(true);
	resize(80,75);
    }
};


class EgQComboBox1 : public QComboBox {
public:
    EgQComboBox1() :
	QComboBox()
    {
	insertItem("Choice 1");
	resize(100,25);
    }
};


class EgQComboBox2 : public QComboBox {
public:
    EgQComboBox2() :
	QComboBox( true )
    {
	insertItem("Choice 1");
	resize(100,25);
    }
};


class EgQComboBox3 : public QComboBox {
public:
    EgQComboBox3() :
	QComboBox( false )
    {
	insertItem("Choice 1");
	resize(100,25);
    }
};


class EgQFileDialog : public QFileDialog {
public:
    EgQFileDialog() :
#if defined (Q_WS_WIN)
	QFileDialog( "c:\\winnt", "*" )
#else
	QFileDialog( getenv( "HOME" ), "*" )
#endif
    {
	//resize(300,300);
	clearWFlags( WType_Modal );
    }
};

class EgQPrintDialog : public QPrintDialog {
public:
    EgQPrintDialog() :
	QPrintDialog( new QPrinter )
    {
	clearWFlags( WType_Modal );
    }
};

class EgQColorDialog : public QColorDialog {
public:
    EgQColorDialog() :
	QColorDialog()
    {
	clearWFlags( WType_Modal );
    }
};

class EgQFontDialog : public QFontDialog {
public:
    EgQFontDialog() :
	QFontDialog()
    {
	clearWFlags( WType_Modal );
    }
};

class EgQMessageBox : public QMessageBox {
public:
    EgQMessageBox() :
	QMessageBox()
    {
	setText("This is a <b>QMessageBox</b> with a message. It can even display <em>Rich-Text!</em>");
	setIcon(QMessageBox::Information);
	clearWFlags( WType_Modal );
    }
};

class EgQProgressBar : public QProgressBar {
public:
    EgQProgressBar() :
	QProgressBar(100)
    {
	setProgress(65);
	resize(70,sizeHint().height());
    }
};

class EgQProgressDialog : public QProgressDialog {
public:
    EgQProgressDialog() :
	QProgressDialog("Please wait...", "Cancel", 100)
    {
	clearWFlags( WType_Modal );
    }
};

class EgRangeControls : public QVBox
{
public:
    EgRangeControls( QWidget *parent = 0 ) : QVBox( parent ) {
	(void)new QLabel( "Some Range-Controls and a LCD-Number:", this );
	
	setMargin( 5 );
	setSpacing( 5 );
	
	QSlider *slider = new QSlider( this );
	slider->setRange( 0, 100 );
	slider->setValue( 30 );
	slider->setOrientation( Horizontal );
	
	QSpinBox *sb = new QSpinBox( this );
	sb->setRange( 0, 600 );
	sb->setSuffix( " $" );
	sb->setValue( 300 );
	
	QLCDNumber *lcd = new QLCDNumber( this );
	lcd->display( "33" );
	
    }
};

class EgQTabDialog : public QTabDialog {
    QVBox t1;
    QWidget t2;
    QWidget t3;
    EgRangeControls rb;

public:
    EgQTabDialog() :
	t1(this),
	t2(this),
	t3(this),
	rb(&t1)
    {
	addTab(&t1, "Base");
	addTab(&t2, "Innings");
	addTab(&t3, "Style");
	//setDefaultButton();
	setCancelButton();
	setApplyButton();
	resize(300,170);
    }

    void show()
    {
	QWidget::show();
    }
};



class EgQTabWidget : public QTabWidget {
    QWidget t1;
    QWidget t2;
    QWidget t3;
    EgQRadioButton rb;

public:
    EgQTabWidget() :
	t1(this),
	t2(this),
	t3(this),
	rb(&t1)
    {
	rb.move(10,10);
	addTab(&t1, "Base");
	addTab(&t2, "Innings");
	addTab(&t3, "Style");
	resize(250,150);
    }

    void show()
    {
	QWidget::show();
    }
};


class EgQGroupBox : public QGroupBox {
public:
    EgQGroupBox() :
	QGroupBox("Group box")
    {
	resize(150,100);
    }
};

class EgQButtonGroup : public QButtonGroup {
    QRadioButton rb1, rb2, rb3;
public:
    EgQButtonGroup() :
	QButtonGroup("Button group"),
	rb1("First",this),
	rb2("Second",this),
	rb3("Third",this)
    {
	resize(150,100);
	insert(&rb1);
	insert(&rb2);
	insert(&rb3);
	rb1.setGeometry(15,20,80,25);
	rb2.setGeometry(15,45,80,25);
	rb3.setGeometry(15,70,80,25);
	rb1.setChecked(true);
	resize(120,100);
    }
};

class EgQLCDNumber : public QLCDNumber {
public:
    EgQLCDNumber() :
	QLCDNumber(2)
    {
	display(27);
	resize(75,45);
    }
};

class EgQLabel : public QLabel {
public:
    EgQLabel() :
	QLabel("This is a &Label\nit spans\nmultiple lines", 0)
    {
	setAlignment(AlignCenter);
	setBuddy(this);
	resize(100,55);
    }
};

class EgQMenuBar : public QMenuBar {
public:
    EgQMenuBar(QWidget* parent) :
	QMenuBar(parent)
    {
	insertItem("File");
	insertItem("Edit");
	insertItem("Options");
	insertSeparator();
	insertItem("Help");
    }
};

class EgNestedQMenuBar : public QWidget {
    EgQMenuBar mb;
public:
    EgNestedQMenuBar() :
	mb(this)
    {
        resize(300,mb.height());
    }
};

class EgQStatusBar : public QStatusBar {
public:
    EgQStatusBar(QWidget* parent=0) :
	QStatusBar(parent)
    {
	resize( 400, sizeHint().height() );
	QLabel *l = new QLabel("R/W",this);
	addWidget(l,1);
	message("Ready");
	//resize(sizeHint());
    }
};

/* XPM */
static const char *fileopen[] = {
"    16    13        5            1",
". c #040404",
"# c #808304",
"a c #bfc2bf",
"b c #f3f704",
"c c #f3f7f3",
"aaaaaaaaa...aaaa",
"aaaaaaaa.aaa.a.a",
"aaaaaaaaaaaaa..a",
"a...aaaaaaaa...a",
".bcb.......aaaaa",
".cbcbcbcbc.aaaaa",
".bcbcbcbcb.aaaaa",
".cbcb...........",
".bcb.#########.a",
".cb.#########.aa",
".b.#########.aaa",
"..#########.aaaa",
"...........aaaaa"
};
static QPixmap fo()
{
    static QPixmap f(fileopen);
    return f;
}

class EgQToolButton : public QToolButton {
public:
    EgQToolButton(QToolBar* parent=0) :
	QToolButton(fo(),"Open","Open group", 0,0, parent)
    {
    }
};

class EgQToolBar : public QToolBar {
public:
    EgQToolBar(QMainWindow* mw=0) :
	QToolBar("Toolbar", mw)
    {
	(void) new EgQToolButton(this);
	//resize(sizeHint());
    }
};

class EgQTableView : public QTableView {
public:
    EgQTableView() :
	QTableView()
    {
	setNumRows(8);
	setNumCols(20);
	setCellHeight(18);
	setTableFlags(Tbl_autoScrollBars);
	resize(200,100);
    }
    void paintCell (QPainter* p, int row, int col)
    {
	p->setPen(DotLine);
	p->fillRect(0,0,cellWidth(col),cellHeight(row),white);
	p->drawLine(cellWidth(col)-1,0,cellWidth(col)-1,cellHeight(row)-1);
	p->drawLine(cellWidth(col)-1,cellHeight(row)-1,0,cellHeight(row)-1);
    }
    int cellWidth(int i)
    {
	return 30+i*10;
    }
};


class EgQListBoxItem : public QListBoxItem {
public:
    EgQListBoxItem()
    {
    }
    void paint(QPainter* p)
    {
	p->setBrush(Qt::green);
	QRect r(3,3,width(0)-3*2,height(0)-3*2);
	p->drawEllipse(r);
	p->drawText(r,Qt::AlignCenter,"Anything!");
    }
    int width(const QListBox*) const
    {
        return 80;
    }
    int height(const QListBox*) const
    {
	return 40;
    }
};

class EgQListBox : public QListBox {
public:
    EgQListBox() :
	QListBox()
    {
	insertItem("First item");
	insertItem("Second item");
	insertItem("Third item");
	insertItem("Fourth item");
	insertItem(new EgQListBoxItem);
	insertItem("Sixth item");
	insertItem("Seventh item");
	insertItem("Eighth item");
	resize(120,130);
    }
};

class EgQMultiLineEdit : public QMultiLineEdit {
public:
    EgQMultiLineEdit() :
	QMultiLineEdit()
    {
	setText(
	    "This is a QMultiLineEdit with text in it.\n"
	    "\n"
	    "The QMultiLineEdit widget is a simple editor for inputting text.\n"
	    "\n"
	    "The QMultiLineEdit widget provides multiple line text input and display. It is intended for moderate\n"
	    "amounts of text. There are no arbitrary limitations, but if you try to handle megabytes of data,\n"
	    "performance will suffer.\n"
	    "\n"
	    "This widget can be used to display text by calling setReadOnly(true)"
	);
	resize(210,120);
    }
};

class EgQPopupMenu : public QPopupMenu {
    QPushButton btn;
public:
    EgQPopupMenu() :
	QPopupMenu(), btn(0)
    {
	insertItem("&New",&btn,SIGNAL(clicked()),CTRL+Key_N);
	insertItem("&Open",&btn,SIGNAL(clicked()),CTRL+Key_O);
	insertSeparator();
	insertItem("&Save",&btn,SIGNAL(clicked()),CTRL+Key_S);
	insertItem("Save &as",&btn,SIGNAL(clicked()),CTRL+Key_A);
	insertItem("&Print",&btn,SIGNAL(clicked()),CTRL+Key_P);
	insertSeparator();
	insertItem("&Quit",&btn,SIGNAL(clicked()),CTRL+Key_Q);
    }
};

class EgQSpinBox : public QSpinBox {
public:
    EgQSpinBox() :
	QSpinBox()
    {
	setValue(42);
	resize(sizeHint());
    }
};

#if 0
class EgQDial : public QDial {
public:
    EgQDial() :
	QDial()
    {
	setValue(42);
	setNotchesVisible(true);
	resize(60,60);
    }
};
#endif
class EgQSpinBox2 : public EgQSpinBox {
public:
    EgQSpinBox2() :
	EgQSpinBox()
    {
	setButtonSymbols(PlusMinus);
    }
};


class EgQHeader : public QHeader {
public:
    EgQHeader() :
	QHeader()
    {
	addLabel("Name");
	addLabel("Address");
	addLabel("Birth date");
	resize(400,sizeHint().height());
    }
};


class EgQSplitter : public QSplitter {
    QLabel a, b;
public:
    EgQSplitter() :
	QSplitter(),
	a("Some widget",this),
	b("Another widget",this)
    {
	setFrameStyle(QFrame::Sunken|QFrame::Panel);
	a.setAlignment(AlignCenter);
	b.setAlignment(AlignCenter);
	setOrientation(Vertical);
	resize(100,100);
    }
};


class EgQLineEdit : public QLineEdit {
public:
    EgQLineEdit() : QLineEdit(0)
    {
	setText("Hello");
	resize(200,30);
    }
};


class EgQScrollBar : public QScrollBar {
public:
    EgQScrollBar() :
	QScrollBar( QScrollBar::Horizontal,0 )
    {
	resize(160,20);
    }
};

class EgQMainWindow : public QMainWindow {
public:
    EgQMainWindow() :
	QMainWindow()
    {
	
	menuBar()->insertItem("File");
	menuBar()->insertItem("Edit");
	menuBar()->insertItem("Options");
	menuBar()->insertSeparator();
	menuBar()->insertItem("Help");
	statusBar()->message( "Ready" );
	QToolBar *tb = new QToolBar( this );
	(void)new EgQToolButton( tb );
	addToolBar( tb );
	
	//(void) new EgQMenuBar(this);
	//(void) new EgQStatusBar(this);
	//(void) new EgQToolBar(this);
	QLabel *f = new QLabel("Central\nWidget",this);
	f->setAlignment(AlignCenter);
	f->setFrameStyle(QFrame::Sunken|QFrame::Panel);
	setCentralWidget(f);
	resize(200,150);
    }
};


class EgQSlider : public QSlider {
public:
    EgQSlider() :
	QSlider( QSlider::Horizontal,0 )
    {
	setTickmarks( QSlider::Below );
	resize(160,26);
    }
};




class EgQSizeGrip : public QSizeGrip {
public:
    EgQSizeGrip() :
	QSizeGrip( 0 )
    {
	resize(sizeHint());
    }
};



class EgQHBox : public QHBox {
public:
    EgQHBox() :
	QHBox()
	{
	    (new QLabel( "One", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Two", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Three", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Four", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Five", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	}
};

class EgQVBox : public QVBox {
public:
    EgQVBox() :
	QVBox()
	{
	    (new QLabel( "One", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Two", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Three", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Four", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Five", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	}
};

class EgQGrid : public QGrid {
public:
    EgQGrid() :
	QGrid( 2 )
	{
	    (new QLabel( "One", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Two", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Three", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Four", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	    (new QLabel( "Five", this ) )->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
	}
};


class EgQTabBar : public QTabBar {
public:
    EgQTabBar()
    {
	QTab* t;
	addTab((t=new QTab,t->label="Base",t->enabled=true,t));
	addTab((t=new QTab,t->label="Innings",t->enabled=true,t));
	addTab((t=new QTab,t->label="Style",t->enabled=true,t));
	resize(160,26);
    }
};

class EgQScrollView : public QScrollView {
    LifeWidget life;
public:
    EgQScrollView()
    {
	addChild(&life);

	life.resize(400,400);
	life.setBackgroundColor(QColor(170,180,170));

	life.setPoint(10,15);
	life.setPoint(11,15);
	life.setPoint(12,15);
	life.setPoint(12,16);
	life.setPoint(11,17);

	life.setPoint(16,16);
	life.setPoint(17,16);
	life.setPoint(16,17);
	life.setPoint(17,17);

	life.nextGeneration();
	life.nextGeneration();
	life.nextGeneration();
	life.nextGeneration();

	resize(190,160);

	center(150,150);
	setFrameStyle(WinPanel|Sunken);
    }
};

class EgComplexMainWindow : public QMainWindow
{
public:
    EgComplexMainWindow() : QMainWindow() {
	menuBar()->insertItem( "&File" );
	menuBar()->insertItem( "&Edit" );
	menuBar()->insertItem( "&Insert" );
	menuBar()->insertItem( "&Tools" );
	menuBar()->insertItem( "&Plugins" );
	menuBar()->insertItem( "E&xtra" );
	menuBar()->insertItem( "&Windows" );
	menuBar()->insertSeparator();
	menuBar()->insertItem( "&Help" );
	
	QToolBar *t = new QToolBar( this );
	QToolButton *tb;
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "fileopen.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "filesave.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "fileprint.xpm" ) );

	t = new QToolBar( this );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "editcut.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "editcopy.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "editpaste.xpm" ) );
	moveToolBar( t, Left );
	
	t = new QToolBar( this );
	QComboBox *cb = new QComboBox( t );
	cb->insertItem( "Helvetica" );
	t->setStretchableWidget( cb );
	cb = new QComboBox( t );
	cb->insertItem( "20" );
	t->addSeparator();
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "leftjust.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "centrejust.xpm" ) );
	tb = new QToolButton( t );
	tb->setPixmap( QPixmap( "rightjust.xpm" ) );
	
	QWorkspace *ws = new QWorkspace( this );
	setCentralWidget( ws );
	QMultiLineEdit *me = new QMultiLineEdit( ws );
	me->setText( "This is the Central Widget." );
	me->setCaption( "Document 1" );

	statusBar()->message( "For Hel, Press F1" );
	
	resize( 400, 300 );
	
    }
};

class EgComplexSplitter : public QSplitter
{
public:
    EgComplexSplitter() : QSplitter() {
	setOrientation( Vertical );
	QSplitter *sp = new QSplitter( this );
	sp->setOrientation( Horizontal );
	
	QListBox *lb = new QListBox( sp );
	lb->insertItem( QPixmap( "fileopen.xpm" ), "File-Open" );
	lb->insertItem( QPixmap( "filesave.xpm" ), "File-Save" );
	lb->insertItem( QPixmap( "fileprint.xpm" ), "File-Print" );
	lb->insertItem( QPixmap( "editcut.xpm" ), "File-Cut" );
	lb->insertItem( QPixmap( "editcopy.xpm" ), "File-Copy" );
	lb->insertItem( QPixmap( "editpaste.xpm" ), "File-Paste" );
	lb->setCurrentItem( 2 );
	
	QListView *lv = new QListView( sp );
	lv->setRootIsDecorated( true );
	lv->addColumn( "Action" );
	lv->addColumn( "Description" );
	QListViewItem *li;
	lv->setSorting( -1 );
	li = new QListViewItem( lv, "File", "File Operations" );
	QListViewItem *r = li;
	li->setOpen( true );
	li = new QListViewItem( r, "Open", "Open a local or remote file" );
	li->setPixmap( 0, QPixmap( "fileopen.xpm" ) );
	li = new QListViewItem( r, "Save", "Save the document to a file" );
	li->setPixmap( 0, QPixmap( "filesave.xpm" ) );
	li = new QListViewItem( r, "Print", "Print out the contents of the document" );
	li->setPixmap( 0, QPixmap( "filePrint.xpm" ) );
	li = new QListViewItem( lv, "Edit", "Edit Operations" );
	li->setOpen( true );
	r = li;
	li = new QListViewItem( r, "Cut", "Cut the selection" );
	li->setPixmap( 0, QPixmap( "editcut.xpm" ) );
	li = new QListViewItem( r, "Copy", "Copy the selection to the clipboard" );
	li->setPixmap( 0, QPixmap( "editcopy.xpm" ) );
	li = new QListViewItem( r, "Paste", "Paste the data from the clipboard" );
	li->setPixmap( 0, QPixmap( "editpaste.xpm" ) );
 	li = new QListViewItem( lv, "Extra", "Extra Operations" );
 	li = new QListViewItem( lv, "Help", "Help and Documentation" );
	lv->setMinimumSize( 300, 200 );
	
	QIconView *iv = new QIconView( this );
	new QIconViewItem( iv, "Actions", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "User Settings", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "Global Settings", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "Plugins", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "Administration", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "Network", QPixmap( "qtlogo.xpm" ) );
	new QIconViewItem( iv, "Update", QPixmap( "qtlogo.xpm" ) );
	iv->setCurrentItem( iv->firstItem() );
	iv->setSelected( iv->firstItem(), false );
	iv->setMinimumHeight( 150 );
	iv->setFocus();
	
	resize( 400, 450 );
    }
};

class EgComplexGroupBox : public QGroupBox
{
public:
    EgComplexGroupBox() : QGroupBox( 1, Horizontal, "Group Box" ) {
	(void)new QLineEdit( "Some Text", this );
	QComboBox *cb = new QComboBox( false, this );
	cb->insertItem( "First Item" );
	cb = new QComboBox( true, this );
	cb->insertItem( "Third Item" );
    }
};

class EgComplexButtonGroup : public QButtonGroup
{
public:
    EgComplexButtonGroup() : QButtonGroup( 1, Horizontal, "Group Box" ) {
	QRadioButton *rb = new QRadioButton( "Radiobutton &1", this );
	rb->setChecked( true );
	rb = new QRadioButton( "Radiobutton &2", this );
	rb = new QRadioButton( "Radiobutton &3", this );
	rb = new QRadioButton( "Radiobutton &4", this );
	QCheckBox *cb = new QCheckBox( "&Checkbox 1", this );
	cb = new QCheckBox( "&Checkbox 2", this );
	cb->setChecked( true );
    }
};

class EgQIconView : public QIconView
{
public:
    EgQIconView() : QIconView() {
	(void)new QIconViewItem( this, "Item 1" );
	(void)new QIconViewItem( this, "Item 2", QPixmap( "qtlogo.xpm" ) );
	(void)new QIconViewItem( this, "Item 3", QPixmap( "editcut.xpm" ) );
	(void)new QIconViewItem( this, "Item 4", QPixmap( "editcopy.xpm" ) );
	(void)new QIconViewItem( this, "Item 5", QPixmap( "editpaste.xpm" ) );
	(void)new QIconViewItem( this, "Item 6", QPixmap( "fileopen.xpm" ) );
	(void)new QIconViewItem( this, "Item 7", QPixmap( "filesave.xpm" ) );
	(void)new QIconViewItem( this, "Item 8", QPixmap( "fileprint.xpm" ) );
	resize( 300, 150 );
    }
};

class EgQTextBrowser : public QTextBrowser
{
public:
    EgQTextBrowser() : QTextBrowser() {
	QString s( getenv( "QTDIR" ) );
	s += "/doc/html/index.html";
	setSource( s );
	resize( 400, 300 );
    }
};

class EgQListView : public DirectoryView
{
public:
    EgQListView() : DirectoryView() {
	addColumn( "Name" );
	addColumn( "Type" );
	(void)new Directory( this, "/" );
	firstChild()->setOpen( true );
	firstChild()->firstChild()->nextSibling()->setOpen( true );
	resize( 300, 300 );
    }
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QFileDialog::setIconProvider( new ImageIconProvider );

    bool first = true;
    QString suffix = "-m.png";
    QApplication::setStyle( new QMotifStyle );

    for ( ;; ) {
	WidgetDepicter wd(suffix);

#define DEPICT(eg, ofile, wname) \
wd.depict( new eg(), ofile, wname );
#define DEPICTFRAMED(eg, ofile, wname) \
wd.depict( new eg(), ofile, wname, true );

/*
	DEPICT( EgQButtonGroup, "qbttngrp", "QButtonGroup" );
	DEPICT( EgQTabDialog, "qtabdlg", "QTabDialog" );
	DEPICT( EgQTabWidget, "qtabwidget", "QTabWidget" );
	DEPICT( EgQGroupBox, "qgrpbox", "QGroupBox" );
	DEPICT( EgQCheckBox, "qchkbox", "QCheckBox" );
	DEPICT( EgQPushButton, "qpushbt", "QPushButton" );
	DEPICT( EgQRadioButton, "qradiobt", "QRadioButton" );
	DEPICT( EgQComboBox1, "qcombo1", "QComboBox()" );
	DEPICT( EgQComboBox2, "qcombo2", "QComboBox(true)" );
	DEPICT( EgQComboBox3, "qcombo3", "QComboBox(false)" );
	DEPICT( EgQLCDNumber, "qlcdnum", "QLCDNumber" );
	DEPICT( EgQLabel, "qlabel", "QLabel" );
	DEPICT( EgNestedQMenuBar, "qmenubar", "QMenuBar" );
	DEPICT( EgQTableView, "qtablevw", "QTableView" );
	DEPICT( EgQListBox, "qlistbox", "QListBox" );
	DEPICT( EgQMultiLineEdit, "qmlined", "QMultiLineEdit" );
	DEPICT( EgQPopupMenu, "qpopmenu", "QPopupMenu" );
	DEPICT( EgQLineEdit, "qlined", "QLineEdit" );
	DEPICT( EgQScrollBar, "qscrbar", "QScrollBar" );
	DEPICT( EgQSlider, "qslider", "QSlider" );
	DEPICT( EgQTabBar, "qtabbar", "QTabBar" );
	DEPICT( EgQProgressBar, "qprogbar", "QProgressBar" );
	DEPICT( EgQProgressDialog, "qprogdlg", "QProgressDialog" );
	DEPICTFRAMED( EgQMainWindow, "qmainwindow", "QMainWindow" );
	DEPICT( EgQScrollView, "qscrollview", "QScrollView" );
	DEPICT( EgQSpinBox, "qspinbox", "QSpinBox" );
	DEPICT( EgQSpinBox2, "qspinbox2", "QSpinBox" );
	DEPICT( EgQHeader, "qheader", "QHeader" );
	DEPICT( EgQSplitter, "qsplitter", "QSplitter" );
	// These crash as top-level objects
	//DEPICT( EgQToolBar, "qtoolbar", "QToolBar" );
	//DEPICT( EgQToolButton, "qtoolbutton", "QToolButton" );
	DEPICT( EgQStatusBar, "qstatusbar", "QStatusBar" );
	DEPICT( EgQGrid, "qgrid", "QGrid" );
	DEPICT( EgQHBox, "qhbox", "QHBox" );
	DEPICT( EgQVBox, "qvbox", "QVBox" );
	//	DEPICT( EgQDial, "qdial", "QDial" );
	DEPICT( EgQSizeGrip, "qsizegrip", "QSizeGrip" );	
	DEPICT( EgComplexSplitter, "splitter-views", "QSplitter" );	
	DEPICTFRAMED( EgComplexMainWindow, "mainwindow", "QMainWindow" );	
	DEPICT( EgQFileDialog, "qfiledlg", "QFileDialog" );
	DEPICT( EgQPrintDialog, "qprintdlg", "QPrintDialog" );
	DEPICT( EgQFontDialog, "qfontdlg", "QFontDialog" );
	DEPICT( EgQColorDialog, "qcolordlg", "QColorDialog" );
	DEPICT( EgQMessageBox, "qmsgbox", "QMessageBox" );
	DEPICT( EgComplexGroupBox, "groupbox", "QGroupBox" );
	DEPICT( EgComplexButtonGroup, "buttongroup", "QButtonGroup" );
	DEPICT( EgQTabDialog, "qtabdlg", "QTabDialog" );
	DEPICT( EgQIconView, "qiconview", "QIconView" );
*/
	DEPICT( EgQTextBrowser, "qtextbrowser", "QTextBrowser" );
	DEPICT( EgQListView, "qlistview", "QListView" );
	
	if ( !first ) break;

	first = false;
	QApplication::setStyle( new QWindowsStyle );
	suffix = "-w.png";
    }

    return 0;
}

#include "makeppms.moc"
