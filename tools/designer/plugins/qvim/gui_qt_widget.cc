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


#include <assert.h>

#include <qpainter.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>

#include "gui_qt_widget.h"


QWidget* qvim_parent = 0;

QWidget* qvim_get_parent()
{
    qDebug("qvim_get_parent:%p", qvim_parent);
    return qvim_parent;
}

void qvim_set_parent( QWidget* parent )
{
    qDebug("void qvim_set_parent( QWidget* parent ):%p", parent);
    qvim_parent = parent;
    if ( qvim_parent == 0 )
	qWarning("qvim: Creating editor with no parent!");
}

VimMainWindow* qvim_get_editor()
{
    qDebug("qvim_get_editor:%p", vmw );
    if ( !vmw )
	qWarning("qvim: unable to create vim");
    return vmw;
}

/*
 * custom Frame for drawing ...
 */
void myFrame::paintEvent( QPaintEvent *e)
{
    QRect r = e->rect();
    gui_redraw(r.x(), r.y(), r.width(), r.height() );
}


void myFrame::draw_string(int col, int row, QString s, int len, int flags)
{

    QFont f(*vmw->vfont);
    if (flags & DRAW_BOLD)
	f.setBold( true );
    if (flags & DRAW_UNDERL)
	f.setUnderline( true );
#if defined(DRAW_ITALIC) // should always be true within kvim
    if (flags & DRAW_ITALIC)
	f.setItalic( true );
#endif
    QPainter p(this);
    p.setPen( QPen (vmw->fg_color, 1) );
    p.setBackgroundColor( vmw->bg_color );
    p.setFont( f );
    if ( ! (flags & DRAW_TRANSP)) {
	QFontMetrics fm( f );
	p.fillRect( FILL_X(col),FILL_Y(row), fm.width(s), gui.char_height, vmw->bg_color );
	p.setBackgroundMode( Qt::OpaqueMode );
    } else {
	p.setBackgroundMode( Qt::TransparentMode );
    }
    p.drawText( TEXT_X(col), TEXT_Y(row), s, len);
    p.end();
}



/*
 *  The main widget (everything but toolbar/menubar)
 */

myWidget::myWidget( QWidget *parent=0, const char *name=0, WFlags f=0 )
	:QWidget(parent, name, f),
	drawing_area(this)
{
}

void myWidget::paintEvent( QPaintEvent *e)
{
    QRect r = e->rect();
    gui_redraw(r.x(), r.y(), r.width(), r.height() );
}


void myWidget::resizeEvent ( QResizeEvent *e )
{
    gui_resize_window( width(), height() );
    QWidget::resizeEvent(e);
}

/*
 *  The main Window
 */

VimMainWindow::VimMainWindow ( QWidget* parent, const char *name , WFlags f)
    :QFrame(parent,name,0) //## used to be destructive close
{
    qDebug("VimMainWindow::VimMainWindow");
    setFocusPolicy( WheelFocus );
    QHBoxLayout* lay = new QHBoxLayout( this, 5 );
    w = new myWidget(this, "main vim widget");
    lay->addWidget( w );
    blink_state = BLINK_NONE;
    blink_on_time = 700;
    blink_off_time = 400;
    blink_wait_time = 250;
    connect( &blink_timer, SIGNAL( timeout() ), SLOT( blink_cursor() ));

    //setCentralWidget(&w); ## some geometry management

    //	connect(toolBar(), SIGNAL(clicked(int)), this, SLOT(menu_activated(int)));##

    vfont = new QFont("courier", 12);
    vfont->setFixedPitch(true);
    clear_oparg(&oa);
    w->setFocus();
}

VimMainWindow::~VimMainWindow()
{
    qDebug("VimMainWindow::~VimMainWindow()");
    QFile file( ".qvim_file" );
    file.remove();
}

void VimMainWindow::setText( const QString& text )
{
#if 0
    stuffcharReadbuff(ESC);
    normal_cmd(&(((VimMainWindow*)this)->oa),TRUE);

    QFile file( ".qvim_file" );
    if ( !file.open( IO_WriteOnly | IO_Truncate ) ) {
	qWarning("qvim: unable to open temporary file");
	return;
    }
    QTextStream stream( &file );
    stream << text;
    file.close();
    QString cmd(":e .qvim_file\n");
    add_to_input_buf( (char_u*) cmd.latin1(), cmd.length() );
    cmd = "i";
    add_to_input_buf( (char_u*) cmd.latin1(), cmd.length() );
    normal_cmd(&oa,TRUE);
    normal_cmd(&oa,TRUE);
    QRect r = w->drawing_area.geometry();
    gui_redraw( r.x(), r.y(), r.width(), r.height() );
    w->drawing_area.setFocus();
    qDebug("DONE VimMainWindow::setText");
#endif
}

QString VimMainWindow::text() const
{
#if 0
    stuffcharReadbuff(ESC);
    normal_cmd(&(((VimMainWindow*)this)->oa),TRUE);
    QString cmd(":w! .qvim_file\n");
    add_to_input_buf( (char_u*) cmd.latin1(), cmd.length() );
    normal_cmd(&(((VimMainWindow*)this)->oa),TRUE);
    QFile file( ".qvim_file" );
    if ( !file.open( IO_ReadOnly | IO_Truncate ) ) {
	qWarning("qvim: unable to open temporary file");
	return QString::null;
    }
    QString text;
    QTextStream stream( &file );
    stream >> text;
    file.close();
    return text;
#endif
    return QString::null;
}

bool VimMainWindow::event(QEvent* e)
{
#if 0
    bool b = QFrame::event(e);
    if ( e->type() == QEvent::Show ) {
	QRect r = w->drawing_area.geometry();
	gui_redraw( r.x(), r.y(), r.width(), r.height() );
    }
    return b;
#endif
}

void VimMainWindow::keyPressEvent( QKeyEvent *e )
{
#if 0
    const char *s = e->text().latin1();
    qDebug("VimMainWindow::keyPressEvent: %s", s);
    add_to_input_buf( (char_u*) s, strlen(s) );
    if ( waitingForInput )
	return;

    if (stuff_empty())
	{
	    if (need_check_timestamps)
		check_timestamps(FALSE);
	    if (need_wait_return)	/* if wait_return still needed ... */
		wait_return(FALSE);	/* ... call it now */
	    if (need_start_insertmode && goto_im())
		{
		    need_start_insertmode = FALSE;
		    stuffReadbuff((char_u *)"i");	/* start insert mode next */
		    /* skip the fileinfo message now, because it would be shown
		     * after insert mode finishes! */
		    need_fileinfo = FALSE;
		}
	}
    if (got_int && !global_busy)
	{
	    if (!quit_more)
		(void)vgetc();		/* flush all buffers */
	    got_int = FALSE;
	}
    if (!exmode_active)
	msg_scroll = FALSE;
    quit_more = FALSE;

    /*
	 * If skip redraw is set (for ":" in wait_return()), don't redraw now.
	 * If there is nothing in the stuff_buffer or do_redraw is TRUE,
	 * update cursor and redraw.
	 */
    if (skip_redraw || exmode_active)
	skip_redraw = FALSE;
    else if (do_redraw || stuff_empty())
	{
	    /*
	     * Before redrawing, make sure w_topline is correct, and w_leftcol
	     * if lines don't wrap, and w_skipcol if lines wrap.
	     */
	    update_topline();
	    validate_cursor();

	    if (VIsual_active)
		update_curbuf(INVERTED);/* update inverted part */
	    else if (must_redraw)
		update_screen(must_redraw);
	    else if (redraw_cmdline || clear_cmdline)
		showmode();
	    redraw_statuslines();
	    /* display message after redraw */
	    if (keep_msg != NULL)
		msg_attr(keep_msg, keep_msg_attr);
	    if (need_fileinfo)		/* show file info after redraw */
		{
		    fileinfo(FALSE, TRUE, FALSE);
		    need_fileinfo = FALSE;
		}

	    emsg_on_display = FALSE;	/* can delete error message now */
	    msg_didany = FALSE;		/* reset lines_left in msg_start() */
	    do_redraw = FALSE;
	    showruler(FALSE);

	    setcursor();
	    cursor_on();
	}
#ifdef USE_GUI
    if (need_mouse_correct)
	gui_mouse_correct();
#endif

    /*
	 * Update w_curswant if w_set_curswant has been set.
	 * Postponed until here to avoid computing w_virtcol too often.
	 */
    update_curswant();

    /*
	 * If we're invoked as ex, do a round of ex commands.
	 * Otherwise, get and execute a normal mode command.
	 */
    if (exmode_active)
	do_exmode();
    else
	normal_cmd(&oa, TRUE);
    qDebug("DONE VimMainWindow::keyPressEvent: %s", s);
#endif
}


void VimMainWindow::menu_activated(int dx)
{
#if 0
	if (!dx) return; // tearoff
//	printf("menu_activated : %p\n", (void*)dx);
	gui_menu_cb((VimMenu *) dx);
#endif
}

void VimMainWindow::focusInEvent( QFocusEvent * fe )
{
#if 0
    w->drawing_area.setFocus();
    gui_focus_change(true);
    if (blink_state == BLINK_NONE)
	gui_mch_start_blink();
#endif
}

void VimMainWindow::focusOutEvent( QFocusEvent * fe )
{
#if 0
    gui_focus_change(false);

    if (blink_state != BLINK_NONE)
	gui_mch_stop_blink();

    // ???
#ifdef USE_XIM
    xim_set_focus(FALSE);
#endif
#endif

}

void VimMainWindow::set_blink_time( long wait, long on, long off)
{
#if 0
    blink_wait_time = wait;
    blink_on_time = on;
    blink_off_time = off;
#endif
}

void VimMainWindow::start_cursor_blinking()
{
#if 0
    if (blink_timer.isActive()) blink_timer.stop();

    /* Only switch blinking on if none of the times is zero */
    if (blink_wait_time && blink_on_time && blink_off_time && gui.in_focus) {
	blink_state = BLINK_ON;
	// The first blink appears after wait_time
	blink_timer.start( blink_wait_time, true);
	gui_update_cursor(TRUE, FALSE);
    }
#endif
}
void VimMainWindow::blink_cursor()
{
#if 0
    if (blink_state == BLINK_ON) {
	// set cursor off
	gui_undraw_cursor();
	blink_state = BLINK_OFF;
	blink_timer.start( blink_off_time, true);
    } else {
	// set cursor on
	gui_update_cursor(TRUE, FALSE);
	blink_state = BLINK_ON;
	blink_timer.start( blink_on_time, true);
    }
#endif
}

void VimMainWindow::stop_cursor_blinking()
{
#if 0
	if (blink_timer.isActive()) blink_timer.stop();

	if (blink_state == BLINK_OFF)
		gui_update_cursor(TRUE, FALSE);

	blink_state = BLINK_NONE;
#endif
}

/*
 *   Vim Dialog
 */

VimDialog::VimDialog (int type,		/* type of dialog */
	       char_u * title,		/* title of dialog */
	       char_u * message,	/* message text */
	       char_u * buttons,	/* names of buttons */
	       int def_but)		/* default button */
	:QDialog(vmw, "vim generic dialog", true), // true is for "modal"
	mapper(this, "dialog signal mapper"),
	icon(this, "icon")
{
	int i;


/* TODO : put a nice icon there
	switch(type) {
		case VIM_GENERIC:
		       setIcon( QMessageBox::NoIcon );
		       break;
		case VIM_ERROR:
			setIcon( QMessageBox::Critical );
		       break;
		case VIM_WARNING:
			setIcon( QMessageBox::Warning );
		       break;
		case VIM_INFO:
			setIcon( QMessageBox::Information );
		       break;
		case VIM_QUESTION:
			setIcon( QMessageBox::Warning );
		       break;
		default:
			setIcon( QMessageBox::NoIcon );
			break;
	};
*/

	/*
	 *  Create buttons
	 */
	char_u * mybuttons = vim_strsave( buttons );
	if (mybuttons == NULL)
		return;
	char_u	*p = mybuttons;
	char_u	*q = p + strlen((char*)mybuttons);
	butNb	= 0;

	// example
	// VimDialog::VimDialog (this=0x7fffecb8, type=2, title=0x19008cc "VIM - ATTENTION",
	// message=0x1998a70 "Swap file \".gui_qt.cc.swp\" already exists!", buttons=0x19008f8 "&Open Read-Only\n&Edit anyway\n&Recover\n&Quit", def_but=1)

	for(i=0; i< 10; i++) {
		button[i] = new QPushButton(this, "button");
		button[i]->hide();
	}

	while (1) {
		while(q>p && *q!= DLG_BUTTON_SEP) q--; // find next one
//		printf("adding button : \"%s\"\n", q+1);
		button[butNb++]->setText( QString((char*)(q+1)) );
		if (butNb > 8) {
		    fprintf(stderr, "vim : Qt : could not set all buttons for dialog: '%s'\n", buttons);
		    break;
		}
		if (q<=p) break;
		*q-- = 0;
	}
	vim_free( mybuttons );


	/*
	 *  Layout
	 */
	int w = 2*10 + butNb*120 ;

	resize(w,60+40);
	setCaption(QString((const char *)title));

	QLabel *label = new  QLabel( QString((const char *)message), this);
	label->setGeometry(60+10,10, w-2*10, 60-2*10 );
	icon.move(10,10);

	connect(&mapper, SIGNAL(mapped(int)), this, SLOT(done(int)));
	for (i=0; i<butNb; i++) {
		button[i]->setGeometry( w-10-(i+1)*120, 60+10, 110, 30);
		button[i]->show();
		connect(button[i], SIGNAL(clicked()), &mapper, SLOT(map()));
		mapper.setMapping(button[i], butNb-i);
	}
}


/*
 * ScrollBar pool handling
 */
SBPool::SBPool(void)
:mapper(this, "SBPool signal mapper")
{
	connect(&mapper, SIGNAL(mapped(int)), this, SLOT(sbUsed(int)));
}


void SBPool::create(GuiScrollbar * sb, int orient)
{
	switch(orient) {
		case SBAR_HORIZ:
			sb->w = new QScrollBar(QScrollBar::Horizontal, vmw->w);
			break;
		case SBAR_VERT:
			sb->w = new QScrollBar(QScrollBar::Vertical, vmw->w);
			break;
		default:
			sb->w = 0;
			return;
	}

	connect(sb->w, SIGNAL(valueChanged(int)), &mapper, SLOT(map()));
	mapper.setMapping(sb->w, (int)sb);
//	printf("creating toolbar, sb=%p, widget=%p\n", sb, sb->w);

}


void SBPool::sbUsed(int who)
{
	GuiScrollbar *sb = (GuiScrollbar*)who;
//	printf("sbUsed : %p\n", (void*)who);
	gui_drag_scrollbar( sb, sb->w->value(), FALSE);
}


void SBPool::destroy(GuiScrollbar * sb)
{
	if (!sb->w) return;

//	printf("destroying toolbar, sb=%p, widget=%p\n", sb, sb->w);
	delete sb->w;
	sb->w = 0;
}
