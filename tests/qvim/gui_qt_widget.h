/* vi:set ts=8 sts=0 sw=8:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * Port to Qt by Dave Berton
 *
 * based on port to KDE by
 *  (C) 2000 by Thomas Capricelli <orzel@yalbi.com>
 *   http://aquila.rezel.enst.fr/thomas/ for other vim- or
 *
 */

#ifndef GUI_QT_WIDGET
#define GUI_QT_WIDGET


#if 0
#define dbf( format, args... ) { printf( "%s" " : " format "\n" , __FUNCTION__ , ## args ); fflush(stdout); }
#define db()       { printf( "%s\n", __FUNCTION__ );fflush(stdout); }
#else
#define dbf(format, args... )
#define db()
#endif


#define UNIX	    // prevent a warning : a symbol is defined twice in X and Qt

#include <qdialog.h>
#include <qlabel.h>
#include <qsignalmapper.h>
#include <qtimer.h>

#include <qapplication.h>
#include <qwidget.h>
#include <qframe.h>

#undef UNIX	    // prevent a warning

extern "C" {
#include "vim.h"   // add_to_input_buf
}

class VimMainWindow;

void qvim_set_parent( QWidget* parent );
QWidget* qvim_get_parent();
VimMainWindow* qvim_get_editor();

class QPushButton;

class	myFrame : public QFrame
{
public:
	myFrame( QWidget *parent=0, const char *name=0, WFlags f=0 )
	    : QFrame(parent,name,f)
    { setFocusPolicy( WheelFocus ); }

	virtual void paintEvent( QPaintEvent *);
	void	draw_string(int col, int row, QString s, int len, int flags);
};


class myWidget : public QWidget
{
//	Q_OBJECT
public:
	myWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );

	virtual void resizeEvent ( QResizeEvent *e );
	virtual void paintEvent( QPaintEvent *);
	myFrame	drawing_area;
};



enum BlinkState {
	BLINK_NONE,
	BLINK_ON,
	BLINK_OFF
};

class VimMainWindow : public QFrame
{
    Q_OBJECT

public:
    //## destructive close?
    VimMainWindow ( QWidget* parent, const char *name = 0L, WFlags f = WDestructiveClose );
    ~VimMainWindow();
    /* everything is public so that gui_qt*.cc may access it */
    // ## horrible, fix this
    myWidget*	w; //## what the hell? should be pointer
    QFont	*vfont;
    QColor	fg_color;
    QColor	bg_color;

    void setWaitForInput( bool wait ) { waitingForInput = wait; }
    void setText( const QString& text );
    QString text() const;

    /* Init the blinking time */
    void set_blink_time( long, long, long );
    void start_cursor_blinking();
    void stop_cursor_blinking();

protected:
    virtual void keyPressEvent( QKeyEvent * );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    bool event(QEvent* e);

    /* cursor blinking stuff */
    QTimer blink_timer;
    long blink_wait_time, blink_on_time, blink_off_time;
    BlinkState blink_state;

    OPARG	    oa;			    /* operator arguments */

private:
    bool waitingForInput;

public slots:
    void    menu_activated(int dx);
    void    blink_cursor();

};


class VimDialog : public QDialog
{

public:
	VimDialog (int type,		/* type of dialog */
	       unsigned char * title,		/* title of dialog */
	       unsigned char * message,	/* message text */
	       unsigned char * buttons,	/* names of buttons */
	       int def_but);		/* default button */
private:
	QSignalMapper	mapper;
	QLabel		icon;
	QPushButton	*button[10];
	int		butNb;
};


/*
 * QScrollBar  pool
 */
struct GuiScrollbar;

class SBPool : public QObject
{
	Q_OBJECT
public:
	SBPool(void);
	void create(GuiScrollbar * sb, int orient);
	void destroy(GuiScrollbar * sb);
public slots:
	void sbUsed(int who);
private:
	QSignalMapper mapper;
};

extern VimMainWindow	*vmw;
extern SBPool		*sbpool;
extern QWidget*          qvim_parent;


#endif // GUI_QT_WIDGET
