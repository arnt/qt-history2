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

#include <qscrollbar.h>
#include <qdatetime.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qpaintdevice.h>
#include <qpainter.h>

#include "gui_qt_widget.h"

extern "C" {
#include "vim.h"
}

// int main ( int argc, char** argv )
// {
//     QApplication app( argc, argv );
//     return vim_main( argc, argv );
// }

#include <stdio.h>

#if 0
#define mputs(a)  qDebug(a)
#else
#define mputs(a)  do {} while (0);
#endif

/*
 * global variable for Qt, we can't put them in Gui, cause there are C++ types
 */
VimMainWindow	*vmw=0;
SBPool		*sbpool=0;


/* This is the single only fixed width font in X11, which seems to be present
 * on all servers and available in all the variants we need.
 *
 * Don't try to tell me that X11 is a wonderfull technology.  If You need to
 * look for a nice GUI design look for the PalmOS instead!
 */

#define DFLT_FONT		"-adobe-courier-medium-r-normal-*-14-*-*-*-m-*-*-*"

/*
 * Parse the GUI related command-line arguments.  Any arguments used are
 * deleted from argv, and *argc is decremented accordingly.  This is called
 * when vim is started, whether or not the GUI has been started.
 */
void
gui_mch_prepare(int *argc, char **argv)
{
    qDebug("gui_mch_prepare");
}

/****************************************************************************
 * Focus handlers:
 */

/*
 * Initialises time intervals for the cursor blinking
 */
void
gui_mch_set_blinking(long waittime, long on, long off)
{
    qDebug("gui_mch_set_blinking");
    vmw->set_blink_time( waittime, on, off );
}

/*
 * Stop the cursor blinking.  Show the cursor if it wasn't shown.
 */
void
gui_mch_stop_blink()
{
    qDebug("gui_mch_stop_blink");
    vmw->stop_cursor_blinking();
}

/*
 * Start the cursor blinking.  If it was already blinking, this restarts the
 * waiting time and shows the cursor.
 */
void
gui_mch_start_blink()
{
    qDebug("gui_mch_start_blink");
    vmw->start_cursor_blinking();
}

/*
 * Check if the GUI can be started.  Called before gvimrc is sourced.
 * Return OK or FAIL.
 */
int
gui_mch_init_check(void)
{
    mputs("gui_mch_init_check");

    gui.dpy = qt_xdisplay();
    gui.num_cols = Columns = 80;
    gui.num_rows = Rows = 25;

    return OK;
}

/*
 * Initialise the X GUI.  Create all the windows, set up all the call-backs etc.
 * Returns OK for success, FAIL when the GUI can't be started.
 */
int
gui_mch_init()
{

    vmw = new VimMainWindow( qvim_get_parent(), "kvim" );
    sbpool = new SBPool;

    gui.in_focus = FALSE; // will be updated

    if (1) { /* reverse*/
	gui.def_norm_pixel = gui_mch_get_color((char_u *)"White");
	gui.def_back_pixel = gui_mch_get_color((char_u *)"Black");
	vmw->fg_color = Qt::white;
	vmw->bg_color = Qt::black;
    } else {
	gui.def_norm_pixel = gui_mch_get_color((char_u *)"Black");
	gui.def_back_pixel = gui_mch_get_color((char_u *)"White");
	vmw->fg_color = Qt::black;
	vmw->bg_color = Qt::white;
    }

    vmw->w->drawing_area.setBackgroundColor ( vmw->bg_color );


    gui.norm_pixel = gui.def_norm_pixel;
    gui.back_pixel = gui.def_back_pixel;

    gui.border_width  = 0;
    gui.border_offset = gui.border_width;

    QFontMetrics qfm(*vmw->vfont);
    gui.char_width  = qfm.maxWidth();
    gui.char_height = qfm.height();
    gui.char_ascent = qfm.ascent();

    mputs("gui_mch_init");
    return OK;
}


/*
 * Called when the foreground or background color has been changed.
 */
void
gui_mch_new_colors()
{
    mputs("gui_mch_new_colors");
}

/*
 * Open the GUI window which was created by a call to gui_mch_init().
 */
int
gui_mch_open()
{
    set_normal_colors();

    /* Check that none of the colors are the same as the background color */
    gui_check_colors();

    /* Get the colors for the highlight groups (gui_check_colors() might have
     * changed them).
     */
    highlight_gui_started();    /* re-init colors and fonts */

    vmw->show();

    qDebug("gui_mch_open");
    return OK;
}


/*ARGSUSED*/
void
gui_mch_exit(int rc)
{
    mputs("gui_mch_exit");
    qApp->quit();
}

/*
 * Get the position of the top left corner of the window.
 */
int
gui_mch_get_winpos(int *x, int *y)
{
    *x = vmw->x();
    *y = vmw->y();
    mputs("gui_mch_get_winpos");
    return OK;
}

/*
 * Set the position of the top left corner of the window to the given
 * coordinates.
 */
void
gui_mch_set_winpos(int x, int y)
{
    vmw->move(x,y);
    mputs("gui_mch_set_winpos");
}

/*
 * Set the windows size.
 */
/*ARGSUSED*/
void
gui_mch_set_winsize(int width, int height,
		    int min_width, int min_height,
		    int base_width, int base_height)
{
    qDebug("gui_mch_set_winsize");
    vmw->resize(width, height);
    vmw->setMinimumSize(min_width, min_height);

    gui_mch_update();

    mputs("gui_mch_set_winsize");
}


/*
 * The screen size is used to make sure the initial window doesn't get bigger
 * then the screen.  This subtracts some room for menubar, toolbar and window
 * decoreations.
 */
void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
    *screen_w = qApp->desktop()->width();
    *screen_h = qApp->desktop()->height();

    // XXX orzel: we may want to substract other widgets size (see gtk)

    mputs("gui_mch_get_screen_dimensions");
}

#if defined(WANT_MENU) || defined(PROTO)
void
gui_mch_enable_menu(int flag)
{
    mputs("gui_mch_enable_menu");
}
#endif


#if defined(USE_TOOLBAR) || defined(PROTO)
void
gui_mch_show_toolbar(int showit)
{
    mputs("gui_mch_show_toolbar");
}
#endif


/*
 * Initialise vim to use the font with the given name.
 * Return FAIL if the font could not be loaded, OK otherwise.
 */
int
gui_mch_init_font(char_u * font_name)
{
	dbf("%s", font_name);
	return OK;
}


GuiFont
gui_mch_get_font(char_u * name, int report_error)
{
	dbf("%s", name);
	return (GuiFont) name;
}

/*
 * Set the current text font.
 * Since we create all GC on demand, we use just gui.current_font to
 * indicate the desired current font.
 */
void
gui_mch_set_font(GuiFont font)
{
	//dbf("%s", font);
}


/*
 * If a font is not going to be used, free its structure.
 */
void
gui_mch_free_font(GuiFont font)
{
	//dbf("%s", font);
}


/*
 * Return the Pixel value (color) for the given color name.  This routine was
 * pretty much taken from example code in the Silicon Graphics OSF/Motif
 * Programmer's Guide.
 * Return -1 for error.
 */
GuiColor
gui_mch_get_color(char_u * name)
{
    dbf( "%s", name );

    int i;
    static char *(vimnames[][2]) =
    {
    /* A number of colors that some X11 systems don't have */
	{"LightRed", "#FFA0A0"},
	{"LightGreen", "#80FF80"},
	{"LightMagenta", "#FFA0FF"},
	{"DarkCyan", "#008080"},
	{"DarkBlue", "#0000C0"},
	{"DarkRed", "#C00000"},
	{"DarkMagenta", "#C000C0"},
	{"DarkGrey", "#C0C0C0"},
	{NULL, NULL}
    };

    if (!gui.in_use)		/* can't do this when GUI not running */
	return (GuiColor)(-1);

    QColor _color((const char *)name);


    if (_color.isValid()) {
//	printf("color allocated : pixel %x, rgb %x\n", _color.pixel(), _color.rgb() );
	return (GuiColor) _color.pixel();
    }

    /* add a few builtin names */
    for (i = 0;; ++i) {
	if (vimnames[i][0] == NULL)
	    return (GuiColor)(-1);
	if (STRICMP(name, vimnames[i][0]) == 0) {
	    name = (char_u *) vimnames[i][1];
	    return gui_mch_get_color(name);
	}
    }

    return (GuiColor)(-1); // dead code, should not be reached..
}

/*
 * Set the current text foreground color.
 */
void
gui_mch_set_fg_color(GuiColor color)
{
    dbf("%lX", color);

    if ( ((GuiColor)-1) == color) return;
    vmw->fg_color = QColor(0x111111, color); // 0x111111 is a fake color here, only the pixel matters..
    mputs("gui_mch_set_fg_color");
}

/*
 * Set the current text background color.
 */
void
gui_mch_set_bg_color(GuiColor color)
{
    dbf("%lX", color);

    if ( ((GuiColor)-1) == color) return;
    vmw->bg_color = QColor(0x111111, color); // 0x111111 is a fake color here, only the pixel matters..
    mputs("gui_mch_set_bg_color");
}

/*
 * Use the blank mouse pointer or not.
 *
 * hide: TRUE = use blank ptr, FALSE = use parent ptr
 */
void
gui_mch_mousehide(int hide)
{
    if (hide == gui.pointer_hidden) return;

    //    vmw->cursor().setShape( (hide)?Qt::WaitCursor:Qt::ArrowCursor ); //## fix hidden cursor
    gui.pointer_hidden = hide;

    mputs("gui_mch_mousehide");
}

void
gui_mch_draw_string(int row, int col, char_u * s, int len, int flags)
{
    QString text( (const char*)s);
    text.truncate( len );
    vmw->w->drawing_area.draw_string( col, row, text, len, flags );
}

/*
 * Return OK if the key with the termcap name "name" is supported.
 */
int
gui_mch_haskey(char_u * name)
{
    dbf("%s", name);
    return FAIL;
}

#if defined(WANT_TITLE) || defined(PROTO)
/*
 * Return the text window-id and display.  Only required for X-based GUI's
 */
int
gui_get_x11_windis(Window * win, Display ** dis)
{
	puts("gui_get_x11_windis");
	*win = vmw->w->drawing_area.winId();
	*dis = qt_xdisplay();
	return OK;
}
#endif

void
gui_mch_beep()
{
    mputs("gui_mch_beep");
    qApp->beep();
}

void
gui_mch_flash(int msec)
{
    mputs("gui_mch_flash");
}

/*
 * Invert a rectangle from row r, column c, for nr rows and nc columns.
 */
void
gui_mch_invert_rectangle(int r, int c, int nr, int nc)
{
    qDebug("gui_mch_invert_rectangle");
}

/*
 * Iconify the GUI window.
 */
void
gui_mch_iconify()
{
    mputs("gui_mch_iconify");
}

/*
 * Draw a cursor without focus.
 */
void
gui_mch_draw_hollow_cursor(GuiColor color)
{
    gui_mch_set_fg_color( color );
    QPainter p(&(vmw->w->drawing_area));
    p.setPen( vmw->fg_color );

/*
#if defined(USE_FONTSET) && defined(MULTI_BYTE)
    if (gui.fontset)
    {
	if (IsLeadByte(LinePointers[gui.row][gui.col])
# ifdef HANGUL_INPUT
		|| composing_hangul
# endif
	   )
	    p.drawRect( FILL_X(gui.col), FILL_Y(gui.row),
			       2*gui.char_width - 1, gui.char_height - 1);
	else
	    p.drawRect( FILL_X(gui.col), FILL_Y(gui.row),
			       gui.char_width - 1, gui.char_height - 1);
    }
    else
#endif
*/
    p.drawRect( FILL_X(gui.col), FILL_Y(gui.row)+1,
		       gui.char_width , gui.char_height-1 );

    p.end();

}

/*
 * Draw part of a cursor, "w" pixels wide, and "h" pixels high, using
 * color "color".
 */
void
gui_mch_draw_part_cursor(int w, int h, GuiColor color)
{
    gui_mch_set_fg_color(color);
    qDebug("gui_mch_draw_part_cursor %d x %d, color %lX", w, h, color);

    QPainter p(&(vmw->w->drawing_area));
    p.fillRect(
	       FILL_X(gui.col),
	       FILL_Y(gui.row) + gui.char_height - h+1,
	       w, h-2, vmw->fg_color);
    p.end();

}


/*
 * Catch up with any queued X11 events.  This may put keyboard input into the
 * input buffer, call resize call-backs, trigger timers etc.  If there is
 * nothing in the X11 event queue (& no timers pending), then we return
 * immediately.
 */
void
gui_mch_update()
{
    qDebug("gui_mch_update(), calling qApp->processEvents()");
    qApp->processEvents();
    mputs("gui_mch_update");
}


/*
 * GUI input routine called by gui_wait_for_chars().  Waits for a character
 * from the keyboard.
 *  wtime == -1     Wait forever.
 *  wtime == 0      This should never happen.
 *  wtime > 0       Wait wtime milliseconds for a character.
 * Returns OK if a character was found to be available within the given time,
 * or FAIL otherwise.
 */
int
gui_mch_wait_for_chars(long wtime)
{
    printf("gui_mch_wait_for_chars wtime = %ld\n", wtime);

    if (wtime>0) {
	printf("waiting for %l time\n", wtime);
	/* wait for wtime max  */
	QTime start = QTime::currentTime();
	QTime now;
	int r = FAIL;
	vmw->setWaitForInput( TRUE );
	while ( vim_is_input_buf_empty() ) {
	    qApp->processEvents(1); // 1 ms : as small as possible
	    now = QTime::currentTime();
	    if ( start.msecsTo(now) > wtime ) {
		r = vim_is_input_buf_empty() ? FAIL : OK;
		break;
	    }
	}
	vmw->setWaitForInput( FALSE );
	return r;
    } else {
	/* wait forever */
	vmw->setWaitForInput( TRUE );
	printf("waiting forever\n");
	while (vim_is_input_buf_empty() )
	    qApp->processOneEvent();
	printf("DONE WAITING\n");
	vmw->setWaitForInput( FALSE );
    }

    // gui_mch_update(); // same as gtk..
    return OK;
}


/****************************************************************************
 * Output drawing routines.
 ****************************************************************************/


/* Flush any output to the screen */
void
gui_mch_flush()
{
    qApp->flushX();
    mputs("gui_mch_flush");
}

/*
 * Clear a rectangular region of the screen from text pos (row1, col1) to
 * (row2, col2) inclusive.
 */
void
gui_mch_clear_block(int row1, int col1, int row2, int col2)
{
    vmw->w->drawing_area.erase(
       FILL_X(col1), FILL_Y(row1),
       (col2 - col1 + 1) * gui.char_width + 1,
       (row2 - row1 + 1) * gui.char_height);

    qDebug("gui_mch_clear_block");
}

void
gui_mch_clear_all(void)
{

    vmw->w->drawing_area.erase();
    qDebug("gui_mch_clear_all");
}

/*
 * Delete the given number of lines from the given row, scrolling up any
 * text further down within the scroll region.
 */
void
gui_mch_delete_lines(int row, int num_lines)
{
//    printf("gui_mch_delete_lines : %d,%d", row, num_lines);

    if (num_lines <= 0)
	return;

    if (row + num_lines > gui.scroll_region_bot) {
	/* Scrolled out of region, just blank the lines out */
	gui_clear_block(row, 0, gui.scroll_region_bot, (int) Columns - 1);
    } else {
	bitBlt (
	    &vmw->w->drawing_area,
	    FILL_X(0), FILL_Y(row),
	    &vmw->w->drawing_area,
	    FILL_X(0), FILL_Y(row + num_lines),
	    gui.char_width * (int) Columns + 1,
	    gui.char_height * (gui.scroll_region_bot - row - num_lines + 1),
	    Qt::CopyROP,	    // raster Operation
	    true );		    // ignoreMask

	gui_clear_block(gui.scroll_region_bot - num_lines + 1, 0,
			gui.scroll_region_bot, (int) Columns - 1);
    }

}

/*
 * Insert the given number of lines before the given row, scrolling down any
 * following text within the scroll region.
 */
void
gui_mch_insert_lines(int row, int num_lines)
{
    mputs("gui_mch_insert_lines");

    if (num_lines <= 0)
	return;

    if (row + num_lines > gui.scroll_region_bot) {
	/* Scrolled out of region, just blank the lines out */
	gui_clear_block(row, 0, gui.scroll_region_bot, (int) Columns - 1);
    } else {
	bitBlt (
	    &vmw->w->drawing_area,
	    FILL_X(0), FILL_Y(row + num_lines),
	    &vmw->w->drawing_area,
	    FILL_X(0), FILL_Y(row),
	    gui.char_width * (int) Columns + 1,
	    gui.char_height * (gui.scroll_region_bot - row - num_lines + 1),
	    Qt::CopyROP,	    // raster Operation
	    true );		    // ignoreMask

	gui_clear_block(row, 0, row + num_lines - 1, (int) Columns - 1);
    }


}

/*
 * X Selection stuff, for cutting and pasting text to other windows.
 */
void
clip_mch_request_selection()
{
    mputs("clip_mch_request_selection");
}

void
clip_mch_lose_selection()
{
    mputs("clip_mch_lose_selection");
}

/*
 * Check whatever we allready own the selection.
 */
int
clip_mch_own_selection()
{
    mputs("clip_mch_own_selection");
    return OK;
}

/*
 * Send the current selection to the clipboard.
 */
void
clip_mch_set_selection()
{
    mputs("clip_mch_set_selection");
}


#if defined(WANT_MENU) || defined(PROTO)
/*
 * Make a menu item appear either active or not active (grey or not grey).
 */
void
gui_mch_menu_grey(VimMenu * menu, int grey)
{
    mputs("gui_mch_menu_grey");
    if ( !menu || !menu->parent || !menu->parent->widget ) return;
    //    menu->parent->widget->setItemEnabled((int)menu, !grey);##
}

/*
 * Make menu item hidden or not hidden.
 */
void
gui_mch_menu_hidden(VimMenu * menu, int hidden)
{
	puts("gui_mch_menu_hidden");
	gui_mch_menu_grey(menu,hidden); // it's hard to remove an item in a QPopupMenu
}

/*
 * This is called after setting all the menus to grey/hidden or not.
 */
void
gui_mch_draw_menubar()
{
    mputs("gui_mch_draw_menubar");
    // nothing to do for qt
}
#endif

/*
 * Scrollbar stuff.
 */
void
gui_mch_enable_scrollbar(GuiScrollbar * sb, int flag)
{
    if (!sb->w) return;
    if (flag)
	sb->w->show();
    else
	sb->w->hide();
    mputs("gui_mch_enable_scrollbar");
}


/*
 * Return the lightness of a pixel.  White is 255.
 */
int
gui_mch_get_lightness(GuiColor pixel)
{
    mputs("gui_mch_get_lightness");
    return 128;// ???
}

#if (defined(SYNTAX_HL) && defined(WANT_EVAL)) || defined(PROTO)

/*
 * Return the RGB value of a pixel as "#RRGGBB".
 */

// XXX orzel : wrong, wrong, wrong, a pixel isn't a RGB anymore under kvim, don't
//		know how to get the actual rgb value though. I'm not sure
//		QColor(0x1111, pixel) will give you the right rgb..
char_u *
gui_mch_get_rgb(GuiColor pixel)
{
    static  char_u retval[10];
    QRgb    c = (QRgb) pixel;

    qDebug("gui_mch_get_rgb");

    sprintf((char *) retval, "#%02x%02x%02x",
	    (unsigned) qRed(c) >> 8,
	    (unsigned) qGreen(c) >> 8,
	    (unsigned) qBlue(c) >> 8);

    /* WOAH!!! Returning pointer to static string!  Could be overwritten when
     * this function is called recursively (e.g., when some event occurs). */
    return  retval;
}
#endif

/*
 * Get current y mouse coordinate in text window.
 * Return -1 when unknown.
 */
int
gui_mch_get_mouse_x(void)
{
    mputs("gui_mch_get_mouse_x");
    return vmw->mapFromGlobal( QCursor::pos() ).x();
}

int
gui_mch_get_mouse_y(void)
{
    mputs("gui_mch_get_mouse_y");
    return vmw->mapFromGlobal( QCursor::pos() ).y();
}

void
gui_mch_setmouse(int x, int y)
{
    mputs("gui_mch_setmouse");
    QCursor::setPos( vmw->mapToGlobal( QPoint(x,y)) );
}
