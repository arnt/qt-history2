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

#include <qmenubar.h>

#include <qscrollbar.h>
#include <qmessagebox.h>

#include "gui_qt_widget.h"

extern "C" {
#include "vim.h"
}

#if 0
#define mputs(a)  qDebug(a)
#else
#define mputs(a)  do {} while (0);
#endif


#ifdef USE_TOOLBAR
static const struct  {
    char *name;
    char *pixname;
} built_in_pixmaps[] =
    {
	{"New",		    "filenew" },
	{"Open",	    "fileopen" },
	{"Save",	    "filesave" },
	{"Undo",	    "undo" },
	{"Redo",	    "redo" },
	{"Cut",		    "editcut" },
	{"Copy",	    "editcopy" },
	{"Paste",	    "editpaste" },
	{"Print",	    "fileprint" },
	{"Help",	    "help" },
	{"Find",	    "find" },
	{"Exit",	    "exit" },
	{ NULL,		    NULL } /* end tag */,
	{"SaveAll",	    "" },
	{"SaveSesn",	    "" },
	{"NewSesn",	    "" },
	{"LoadSesn",	    "" },
	{"RunScript",	    "" },
	{"Replace",	    "" },
	{"WinClose",	    "" },
	{"WinMax",	    "" },
	{"WinMin",	    "" },
	{"WinSplit",	    "" },
	{"Shell",	    "" },
	{"FindPrev",	    "" },
	{"FindNext",	    "" },
	{"FindHelp",	    "" },
	{"Make",	    "" },
	{"TagJump",	    "" },
	{"RunCtags",	    "" },
	{ NULL,		    NULL } /* end tag */
    };


static void try_toolbar(const char *name, int idx)
{
    if (strncmp(name, "BuiltIn", (size_t)7) == 0) {
	printf("BuiltIn menu asked ??\n");
	return;
    }

    if ( is_menu_separator((unsigned char *)name) ) {
	//	vmw->toolBar()->insertSeparator();##
	return;
    }

//     if (vmw->toolBar()->getButton(idx))##
//	return; // already here##

    for (int i=0; built_in_pixmaps[i].name ; i++)
	if (strcmp(built_in_pixmaps[i].name, name) == 0) {
	    /* found a match .. */
//	    printf("pixmap found : %s, idx is %p\n", name, (void*)idx);
//	    vmw->toolBar()->insertButton (##
//		QString(built_in_pixmaps[i].pixname),
//		idx,
//		true,
//		QString(built_in_pixmaps[i].name) );
	    return;
	}
//    printf("pixmap not found : %s, idx is %p\n", name, (void*)idx);
}


#endif // USE_TOOLBAR


/*ARGSUSED*/
void
gui_mch_add_menu(VimMenu * menu, VimMenu * parent, int idx)
{
//    printf("gui_mch_add_menu : %s, parent is %s\n", menu->name, parent?parent->name:"NULL" );

//     QPopupMenu *me;

//     if (!menubar_menu(menu->name))
//	return;

//     if (popup_menu(menu->name)) {
//	menu->widget = new QPopupMenu(vmw , (const char *) menu->name);
//	return;
//     }

//     if (parent) {
//	me = new QPopupMenu(parent->widget, (const char *) menu->name);
//	parent->widget->insertItem( QString((const char *)menu->name), me, idx );

//     } else {
//	me = new QPopupMenu(vmw->menuBar() , (const char *) menu->name);
//	vmw->menuBar()->insertItem( QString((const char *)menu->name), me , idx);
//     }

//     me->insertTearOffHandle(0);
//     me->setCaption((const char*)( menu->name + (*menu->name=='&') ));
//     QObject::connect( me, SIGNAL(activated(int)), vmw, SLOT(menu_activated(int)) );
//     menu->widget = me;
//     menu->parent = parent;

}


/*ARGSUSED*/
void
gui_mch_add_menu_item(VimMenu * menu, VimMenu * parent, int /*idx*/)
{
//    printf("gui_mch_add_menu_item : %s, parent is %s\n", menu->dname, parent?parent->dname:"NULL");

    if (!parent) {
	printf("gui_mch_add_menu_item with NULL parent !!! ");
	return;
    }

    if (toolbar_menu(parent->name)) {
#ifdef USE_TOOLBAR
	try_toolbar((char*)(menu->dname), (int)menu);
#endif // USE_TOOLBAR
	return;
    }

    if (!menubar_menu(parent->name))
	return;

    if ( is_menu_separator(menu->name) )
	parent->widget->insertSeparator();
    else
	parent->widget->insertItem( QString((const char *)menu->name), (int)menu );

    menu->parent = parent;
}


void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
    vmw->w->drawing_area.setGeometry(x,y,w,h);
    mputs("gui_mch_set_text_area_pos");
}


#if defined(WANT_MENU) || defined(PROTO)
/*
 * Enable or disable mnemonics for the toplevel menus.
 */
/*ARGSUSED*/
void
gui_gtk_set_mnemonics(int enable)
{
    mputs("gui_gtk_set_mnemonics");
}


void
gui_mch_toggle_tearoffs(int enable)
{
    mputs("gui_mch_toggle_tearoffs");
}
#endif


#if defined(WANT_MENU) || defined(PROTO)
/*
 * Destroy the machine specific menu widget.
 */
void
gui_mch_destroy_menu(VimMenu * menu)
{
#ifdef USE_TOOLBAR
/* FIXME :destroy the toolbar
    if (menu->parent && toolbar_menu(menu->parent->name)) {
	toolbar_remove_item_by_text(GTK_TOOLBAR(gui.toolbar),
					    (const char *)menu->dname);
	return;
    }
*/
#endif
    if (menu->widget)
	delete menu->widget;
    menu->widget = 0;
    mputs("gui_mch_destroy_menu");
}
#endif /* WANT_MENU */


/*
 * Scrollbar stuff.
 */

void
gui_mch_set_scrollbar_thumb(GuiScrollbar * sb, int val, int size, int max)
{
    if (!sb->w) return;

    sb->w->setRange(0, max+1-size);
    sb->w->setValue(val);

    sb->w->setLineStep(1);
    sb->w->setPageStep(size);

    mputs("gui_mch_set_scrollbar_thumb");
}

void
gui_mch_set_scrollbar_pos(GuiScrollbar * sb, int x, int y, int w, int h)
{
    if (!sb->w) return;
    sb->w->setGeometry(x,y,w,h);
    mputs("gui_mch_set_scrollbar_pos");
}

/* SBAR_VERT or SBAR_HORIZ */
void
gui_mch_create_scrollbar(GuiScrollbar * sb, int orient)
{
	sbpool->create(sb,orient);

	gui_mch_update();
	mputs("gui_mch_create_scrollbar");
}

void
gui_mch_destroy_scrollbar(GuiScrollbar * sb)
{
	sbpool->destroy(sb);
	gui_mch_update();
	mputs("gui_mch_destroy_scrollbar");
}

#if defined(USE_BROWSE) || defined(PROTO)
/*
 * Implementation of the file selector related stuff
 */

/*
 * Put up a file requester.
 * Returns the selected name in allocated memory, or NULL for Cancel.
 * saving,			select file to write
 * title			title for the window
 * dflt				default name
 * ext				not used (extension added)
 * initdir			initial directory, NULL for current dir
 * filter			not used (file name filter)
 */
/*ARGSUSED*/
char_u *
gui_mch_browse(int saving,
	       char_u * title,
	       char_u * dflt,
	       char_u * ext,
	       char_u * initdir,
	       char_u * filter)
{
    mputs("gui_mch_browse");
}

#endif	/* USE_BROWSE */

#ifdef GUI_DIALOG

/* ARGSUSED */
/*
 * This makes the Escape key equivalent to the cancel button.
 */

/* ARGSUSED */
int
gui_mch_dialog(int type,		/* type of dialog */
	       char_u * title,		/* title of dialog */
	       char_u * message,	/* message text */
	       char_u * buttons,	/* names of buttons */
	       int def_but)		/* default button */
{
	mputs("gui_mch_dialog");

	VimDialog vd(type, title, message, buttons, def_but);
	return  vd.exec();
}


#endif	/* GUI_DIALOG */

#if defined(WANT_MENU) || defined(PROTO)
void
gui_mch_show_popupmenu(VimMenu * menu)
{
    mputs("gui_mch_show_popupmenu");
}
#endif


void
gui_mch_find_dialog(char_u * arg)
{
    mputs("gui_mch_find_dialog");
}


void
gui_mch_replace_dialog(char_u * arg)
{
    mputs("gui_mch_replace_dialog");
}

void
do_helpfind(void)
{
    mputs("do_helpfind");
}
