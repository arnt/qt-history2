#include <qapplication.h>

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
#include <qlistbox.h>
#include <qmultilined.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabbar.h>
#include <qscrollview.h>

#include <life.h>

#include <qtextstream.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <stdlib.h>

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
	done = TRUE;
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
	depict(w, savefile, widgetname, TRUE);
    }

    void depict(QWidget* w, const char* savefile, const char* widgetname,
		bool frame=FALSE)
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
	    done = TRUE;
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
	    pm = QPixmap::grabWindow( widget->winId() );
	}
	if (includeframe) {
	    pm.save( filename, "PPM" );
	} else {
	    QPixmap ppm(r.width()+2*margin, r.height()+2*margin);
	    ppm.fill(QApplication::palette()->normal().background());
	    QPainter p;
	    p.begin(&ppm);
	    p.drawPixmap(margin,margin,pm);
	    p.end();
	    ppm.save( filename, "PPM" );
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
	cb1.setChecked(TRUE);
	resize(80,75);
    }
};


class EgQPushButton : public QPushButton {
public:
    EgQPushButton() :
	QPushButton( "Press Me" )
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
	rb1.setChecked(TRUE);
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
	QComboBox( TRUE )
    {
	insertItem("Choice 1");
	resize(100,25);
    }
};


class EgQComboBox3 : public QComboBox {
public:
    EgQComboBox3() :
	QComboBox( FALSE )
    {
	insertItem("Choice 1");
	resize(100,25);
    }
};


class EgQFileDialog : public QFileDialog {
public:
    EgQFileDialog() :
	QFileDialog( "/", "*" )
    {
	clearWFlags( WType_Modal );
    }
};

class EgQMessageBox : public QMessageBox {
public:
    EgQMessageBox() :
	QMessageBox()
    {
	setText("This is a QMessageBox\nwith a message");
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

class EgQTabDialog : public QTabDialog {
    QWidget t1;
    QWidget t2;
    QWidget t3;
    EgQRadioButton rb;

public:
    EgQTabDialog() :
	t1(this),
	t2(this),
	t3(this),
	rb(&t1)
    {
	rb.move(10,10);
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
	rb1.setChecked(TRUE);
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
	QLabel("This is a &Label\nit spans\nmultiple lines")
    {
	setAlignment(AlignCenter);
	setBuddy(this);
	resize(100,55);
    }
};

class EgQMenuBar : public QWidget {
    QMenuBar mb;
public:
    EgQMenuBar() :
	mb(this)
    {
	mb.insertItem("File");
	mb.insertItem("Edit");
	mb.insertItem("Options");
	mb.insertSeparator();
	mb.insertItem("Help");
	resize(300,mb.height());
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
	p->setBrush(green);
	QRect r(3,3,width(0)-3*2,height(0)-3*2);
	p->drawEllipse(r);
	p->drawText(r,AlignCenter,"Anything!");
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
	    "This widget can be used to display text by calling setReadOnly(TRUE)"
	);
	resize(210,120);
    }
};

class EgQPopupMenu : public QPopupMenu {
    QPushButton btn;
public:
    EgQPopupMenu() :
	QPopupMenu()
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


class EgQLineEdit : public QLineEdit {
public:
    EgQLineEdit()
    {
	setText("Hello");
	resize(200,30);
    }
};


class EgQScrollBar : public QScrollBar {
public:
    EgQScrollBar() :
	QScrollBar( QScrollBar::Horizontal )
    {
	resize(160,20);
    }
};


class EgQSlider : public QSlider {
public:
    EgQSlider() :
	QSlider( QSlider::Horizontal )
    {
	setTickmarks( QSlider::Below );
	resize(160,26);
    }
};


class EgQTabBar : public QTabBar {
public:
    EgQTabBar()
    {
	QTab* t;
	addTab((t=new QTab,t->label="Base",t->enabled=TRUE,t));
	addTab((t=new QTab,t->label="Innings",t->enabled=TRUE,t));
	addTab((t=new QTab,t->label="Style",t->enabled=TRUE,t));
	resize(160,26);
    }
};

class EgQScrollView : public QScrollView {
    LifeWidget life;
public:
    EgQScrollView()
    {
	view(&life);

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

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    bool first = true;
    QString suffix = "-m.ppm";
    QApplication::setStyle( MotifStyle );

    while ( 1 ) {
	WidgetDepicter wd(suffix);

#define DEPICT(eg, ofile, wname) \
    wd.depict( new eg(), ofile, wname );

	DEPICT( EgQButtonGroup, "qbttngrp", "QButtonGroup" );
	DEPICT( EgQTabDialog, "qtabdlg", "QTabDialog" );
	DEPICT( EgQGroupBox, "qgrpbox", "QGroupBox" );
	DEPICT( EgQCheckBox, "qchkbox", "QCheckBox" );
	DEPICT( EgQPushButton, "qpushbt", "QPushButton" );
	DEPICT( EgQRadioButton, "qradiobt", "QRadioButton" );
	DEPICT( EgQComboBox1, "qcombo1", "QComboBox()" );
	DEPICT( EgQComboBox2, "qcombo2", "QComboBox(TRUE)" );
	DEPICT( EgQComboBox3, "qcombo3", "QComboBox(FALSE)" );
	DEPICT( EgQFileDialog, "qfiledlg", "QFileDialog" );
	DEPICT( EgQMessageBox, "qmsgbox", "QMessageBox" );
	DEPICT( EgQLCDNumber, "qlcdnum", "QLCDNumber" );
	DEPICT( EgQLabel, "qlabel", "QLabel" );
	DEPICT( EgQMenuBar, "qmenubar", "QMenuBar" );
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
	// Not-yet-released
	//DEPICT( EgQScrollView, "qscrollview", "QScrollView" );
	//DEPICT( EgQSpinBox, "qspinbox", "QSpinBox" );
	//DEPICT( EgQHeader, "qheader", "QHeader" );

	if ( !first ) break;

	first = false;
	QApplication::setStyle( WindowsStyle );
	suffix = "-w.ppm";
    }

    return 0;
}
