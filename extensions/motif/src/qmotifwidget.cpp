/****************************************************************************
**
** Implementation of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmotifwidget.h"
#include "qmotif.h"

#include <qapplication.h>
#include <qobjectlist.h>
#include <qwidgetintdict.h>
#include <qevent.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/Xatom.h>

const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

// TopLevelShell subclass to wrap toplevel motif widgets into QWidgets

void qmotif_widget_shell_realize( Widget w, XtValueMask *mask,
				  XSetWindowAttributes *attr );
void qmotif_widget_shell_change_managed( Widget w );

typedef struct {
    QMotifWidget *widget;
} QMotifWidgetShellPart;

typedef struct _QMotifWidgetShellRec
{
    // full instance record declaration
    CorePart			core;
    CompositePart		composite;
    ShellPart			shell;
    WMShellPart			wmshell;
    VendorShellPart		vendorshell;
    TopLevelShellPart		toplevelshell;
    QMotifWidgetShellPart	qmotifwidgetshell;
} QMotifWidgetShellRec;

typedef struct
{
    // extension record
    XtPointer extension;
} QMotifWidgetShellClassPart;

typedef struct _QMotifWidgetShellClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ShellClassPart		shell_class;
    WMShellClassPart		wmshell_class;
    VendorShellClassPart	vendorshell_class;
    TopLevelShellClassPart	toplevelshell_class;
    QMotifWidgetShellClassPart	qmotifwidgetshell_class;
} QMotifWidgetShellClassRec;

externalref QMotifWidgetShellClassRec		qmotifWidgetShellClassRec;
externalref WidgetClass				qmotifWidgetShellWidgetClass;
typedef struct _QMotifWidgetShellClassRec	*QMotifWidgetShellWidgetClass;
typedef struct _QMotifWidgetShellRec		*QMotifWidgetShellWidget;

externaldef(qmotifwidgetshellclassrec)
    QMotifWidgetShellClassRec qmotifWidgetShellClassRec = {
	// core
	{
	    (WidgetClass) &topLevelShellClassRec,	// superclass
	    "QMotifWidgetShell",			// class name
	    sizeof(QMotifWidgetShellRec),		// widget size
	    NULL,					/* class_initialize proc */
	    NULL,					/* class_part_initialize proc */
	    FALSE,					/* class_inited flag */
	    NULL,					/* instance initialize proc */
	    NULL,					/* init_hook proc */
	    qmotif_widget_shell_realize,		/* realize widget proc */
	    NULL,					/* action table for class */
	    0,						/* num_actions */
	    NULL,					/* resource list of class */
	    0,						/* num_resources in list */
	    NULLQUARK,					/* xrm_class ? */
	    FALSE,					/* don't compress_motion */
	    XtExposeCompressSeries,			/* compressed exposure */
	    FALSE,					/* do compress enter-leave */
	    FALSE,					/* do have visible_interest */
	    NULL,					/* destroy widget proc */
	    XtInheritResize,				/* resize widget proc */
	    NULL,					/* expose proc */
	    NULL,					/* set_values proc */
	    NULL,					/* set_values_hook proc */
	    XtInheritSetValuesAlmost,			/* set_values_almost proc */
	    NULL,					/* get_values_hook */
	    NULL,					/* accept_focus proc */
	    XtVersion,					/* current version */
	    NULL,					/* callback offset    */
	    XtInheritTranslations,			/* default translation table */
	    XtInheritQueryGeometry,			/* query geometry widget proc */
	    NULL,					/* display accelerator	  */
	    NULL					/* extension record	 */
	},
	// composite
	{
	    XtInheritGeometryManager,			/* geometry_manager */
	    qmotif_widget_shell_change_managed,		// change managed
	    XtInheritInsertChild,			// insert_child
	    XtInheritDeleteChild,			// delete_child
	    NULL					// extension record
	},
	// shell extension record
	{ NULL },
	// wmshell extension record
	{ NULL },
	// vendorshell extension record
	{ NULL },
	// toplevelshell extension record
	{ NULL },
	// qmotifwidgetshell extension record
	{ NULL }
    };

externaldef(qmotifwidgetshellwidgetclass)
    WidgetClass qmotifWidgetShellWidgetClass = (WidgetClass)&qmotifWidgetShellClassRec;


class QMotifWidgetPrivate
{
public:
    QMotifWidgetPrivate() : widget( NULL ), shell( NULL ) { }

    Widget widget;
    Widget shell;
};

/*!
    \class QMotifWidget
    \brief The QMotifWidget class provides the QWidget API for Xt/Motif widgets.

    \extension Motif

    QMotifWidget exists to provide a QWidget that can act as a parent
    for any Xt/Motif widget. Since the QMotifWidget is a proper
    QWidget, it an be used as a top-level widget (e.g. 0 parent) or as
    a child of any other QWidget. Note: Since QMotifWidget acts as a
    parent for Xt/Motif widgets, you should not create QWidgets with a
    QMotifWidget parent.

    An Xt/Motif widget with a top-level QMotifWidget parent can begin
    using the standard Qt dialogs and custom QDialogs while keeping
    the main Xt/Motif interface of the application. Using a
    QMotifWidget as the parent for the various QDialogs will ensure
    that modality and stacking works properly throughout the entire
    application.

    Applications moving to Qt may have custom Xt/Motif widgets that
    will take time to rewrite with Qt. Such applications can use
    these custom widgets as QMotifWidget with QWidget parents. This
    allows the application's interface to be replaced gradually.

    \warning QMotifWidget uses the X11 window ID of the Motif widget
    directly, instead of creating its own.  Because ot this,
    QWidget::reparent() will not work.  This includes the functions
    QWidget::showFullScreen() and QWidget::showNormal(), which use
    QWidget::reparent().
*/

/*!
    Creates a QMotifWidget of the given \a widgetclass as a child of
    \a parent, with the name \a name and widget flags \a flags.

    The \a args and \a argcount arguments are passed on to
    XtCreateWidget.

    The motifWidget() function returns the resulting Xt/Motif widget.
    This widget can be used as a parent for any other Xt/Motif widget.

    If \a parent is a QMotifWidget, the Xt/Motif widget is created as
    a child of the parent's motifWidget(). If \ parent is 0 or a
    normal QWidget, the Xt/Motif widget is created as a child of a
    special TopLevelShell widget. Xt/Motif widgets can use this
    special TopLevelShell parent as the parent for existing
    Xt/Motif dialogs or QMotifDialogs.
*/
QMotifWidget::QMotifWidget( QWidget *parent, WidgetClass widgetclass,
			    ArgList args, Cardinal argcount,
			    const char *name, WFlags flags )
    : QWidget( parent, name, flags )
{
    setFocusPolicy( StrongFocus );
    d = new QMotifWidgetPrivate;

    Widget motifparent = NULL;
    if ( parent ) {
	if ( parent->inherits( "QMotifWidget" ) ) {
	    // not really supported, but might be possible
	    motifparent = ( (QMotifWidget *) parent )->motifWidget();
	} else {
	    // keep an eye on the position of the toplevel widget, so that we
	    // can tell our hidden shell its real on-screen position
	    parent->topLevelWidget()->installEventFilter( this );
	}
    }

    if ( ! motifparent || ( widgetclass == applicationShellWidgetClass ||
			    widgetclass == topLevelShellWidgetClass ) ) {
	ArgList realargs = new Arg[argcount + 3];
	Cardinal nargs = argcount;
	memcpy( realargs, args, sizeof( Arg ) * argcount );

	if ( ! QPaintDevice::x11AppDefaultVisual() ) {
	    // make Motif use the same visual/colormap/depth as Qt (if
	    // Qt is not using the default)
	    XtSetArg( realargs[ nargs++ ], XtNvisual,
		      QPaintDevice::x11AppVisual() );
	    XtSetArg( realargs[ nargs++ ], XtNcolormap,
		      QPaintDevice::x11AppColormap() );
	    XtSetArg( realargs[ nargs++ ], XtNdepth,
		      QPaintDevice::x11AppDepth() );
	}

	d->shell = XtAppCreateShell( name, name, qmotifWidgetShellWidgetClass,
				     QPaintDevice::x11AppDisplay(), realargs, nargs );
	( (QMotifWidgetShellWidget) d->shell )->qmotifwidgetshell.widget = this;
	motifparent = d->shell;

	delete [] realargs;
	realargs = 0;
    }

    if ( widgetclass == applicationShellWidgetClass ||
	 widgetclass == topLevelShellWidgetClass )
	d->widget = d->shell;
    else
	d->widget = XtCreateWidget( name, widgetclass, motifparent, args, argcount );

    if (! extraData()) {
        // createExtra() is private, so use topData() to ensure that
        // the extra data is created
        (void) topData();
    }
    extraData()->compress_events = FALSE;
}

/*!
    Destroys the QMotifWidget. The special TopLevelShell is also
    destroyed, if it was created during construction.
*/
QMotifWidget::~QMotifWidget()
{
    QMotif::unregisterWidget( this );
    XtDestroyWidget( d->widget );
    if ( d->shell ) {
	( (QMotifWidgetShellWidget) d->shell )->qmotifwidgetshell.widget = 0;
	XtDestroyWidget( d->shell );
    }
    delete d;

    destroy( FALSE );
}

/*!
    Returns the embedded Xt/Motif widget. If a Shell widget was
    created by the constructor, you can access it with XtParent().
*/
Widget QMotifWidget::motifWidget() const
{
    return d->widget;
}

/*!
    \reimp

    Manages the embedded Xt/Motif widget and shows the widget.
*/
void QMotifWidget::show()
{
    if ( d->shell ) {
	// since we have a shell, we need to manage it's children
	QMotifWidgetShellWidget motifshell = (QMotifWidgetShellWidget) d->shell;
	XtManageChildren( motifshell->composite.children,
			  motifshell->composite.num_children );
	if ( ! XtIsRealized( d->shell ) )
	    XtRealizeWidget( d->shell );
    }

    QWidget::show();
}

/*!
    \reimp

    Unmanages the embedded Xt/Motif widget and hides the widget.
*/
void QMotifWidget::hide()
{
    if ( d->shell ) {
	// since we have a shell, we need to manage it's children
	QMotifWidgetShellWidget motifshell = (QMotifWidgetShellWidget) d->shell;
	XtUnmanageChildren( motifshell->composite.children,
			    motifshell->composite.num_children );
    }

    QWidget::hide();
}


/*! \internal
    Wraps the Motif widget by setting the X window for the
    QMotifWidget to the X window id of the widget shell.
*/
void QMotifWidget::realize( Widget w )
{
    // use the winid of the dialog shell, reparent any children we
    // have
    if ( XtWindow( w ) != winId() ) {
	// save the geometry of the motif widget, since it has the
	// geometry we want
	QRect save( w->core.x, w->core.y, w->core.width, w->core.height );

	// save the caption
	QString cap;
	if ( ! caption().isNull() ) {
	    cap = caption();
	    setCaption( QString::null );
	} else {
	    setCaption( QString::null );
	    XTextProperty text_prop;
	    if (XGetWMName( QPaintDevice::x11AppDisplay(), winId(), &text_prop)) {
		if (text_prop.value && text_prop.nitems > 0) {
		    if (text_prop.encoding == XA_STRING) {
			cap = QString::fromLocal8Bit( (char *) text_prop.value );
		    } else {
			text_prop.nitems = strlen((char *) text_prop.value);

			char **list;
			int num;
			if (XmbTextPropertyToTextList(QPaintDevice::x11AppDisplay(),
						      &text_prop,
						      &list, &num) == Success &&
			    num > 0 && *list) {
			    cap = QString::fromLocal8Bit( *list );
			    XFreeStringList(list);
			}
		    }
		}
	    }
	}

	Window newid = XtWindow( w );
	if ( children() ) {
	    QObjectListIt it( *children() );
	    for ( ; it.current(); ++it ) {
		if ( it.current()->isWidgetType() ) {
		    QWidget *widget = (QWidget *) it.current();
		    XReparentWindow( QPaintDevice::x11AppDisplay(),
				     widget->winId(),
				     newid,
				     widget->x(),
				     widget->y() );
		    if ( !widget->isHidden() )
			XMapWindow( QPaintDevice::x11AppDisplay(), widget->winId() );
		}
	    }
	}
	QApplication::syncX();

	// re-create this QWidget with the winid from the motif
	// widget... the geometry will be reset to roughly 1/4 of the
	// screen, so we need to restore it below
	create( newid, TRUE, TRUE );

	// restore the caption
	setCaption( cap );

	// restore geometry of the shell
	XMoveResizeWindow( QPaintDevice::x11AppDisplay(), winId(),
			   save.x(), save.y(), save.width(), save.height() );

	// if this QMotifWidget has a parent widget, we should
	// reparent the shell into that parent
	if ( parentWidget() ) {
	    XReparentWindow( x11Display(), winId(),
			     parentWidget()->winId(), x(), y() );
	}
    }
    QMotif::registerWidget( this );
}

/*! \internal
    Motif callback to resolve a QMotifWidget and call
    QMotifWidget::realize().
*/
void qmotif_widget_shell_realize( Widget w, XtValueMask *mask,
				  XSetWindowAttributes *attr )
{
    XtRealizeProc realize =
	((CoreWidgetClass)topLevelShellClassRec.core_class.
	 superclass)->core_class.realize;
    (*realize)( w, mask, attr );

    QMotifWidget *widget =
	( (QMotifWidgetShellWidget) w )->qmotifwidgetshell.widget;
    if ( ! widget )
	return;
    widget->realize( w );
}

/*! \internal
    Motif callback to resolve a QMotifWidget and set the initial
    geometry of the widget.
*/
void qmotif_widget_shell_change_managed( Widget w )
{
    XtWidgetProc change_managed =
	((CompositeWidgetClass)topLevelShellClassRec.core_class.
	 superclass)->composite_class.change_managed;
    (*change_managed)( w );

    QMotifWidget *widget =
	( (QMotifWidgetShellWidget) w )->qmotifwidgetshell.widget;
    if ( ! widget )
	return;
    QRect r( widget->d->shell->core.x,
	     widget->d->shell->core.y,
	     widget->d->shell->core.width,
	     widget->d->shell->core.height ),
	d = widget->geometry();
    if ( d != r ) {
	if ( ! widget->isTopLevel() && widget->parentWidget() &&
	     widget->parentWidget()->layout() != 0 ) {
	    // the widget is most likely resized by a layout
	    XtMoveWidget( w, d.x(), d.y() );
	    XtResizeWidget( w, d.width(), d.height(), 0 );
	    widget->setGeometry( d );
	} else {
	    // take the size from the motif widget
	    widget->setGeometry( r );
	}
    }
}

/*!\reimp
 */
bool QMotifWidget::event( QEvent* e )
{
    if ( dispatchQEvent( e, this ) )
	return TRUE;
    return QWidget::event( e );
}

/*!\reimp
 */
bool QMotifWidget::eventFilter( QObject *object, QEvent *event )
{
    if ( object != topLevelWidget() || event->type() != QEvent::Move )
	return FALSE;

    // the motif widget is embedded in our special shell, so when the
    // top-level widget moves, we need to inform the special shell
    // about our new position
    QPoint p = topLevelWidget()->geometry().topLeft() +
	       mapTo( topLevelWidget(), QPoint( 0, 0 ) );
    d->shell->core.x = p.x();
    d->shell->core.y = p.y();

    return FALSE;
}

bool QMotifWidget::dispatchQEvent( QEvent* e, QWidget* w)
{
    switch ( e->type() ) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
	if ( QMotif::lastEvent() ) {
	    QMotif::lastEvent()->xany.window = w->winId();
	    QMotif::redeliverEvent( QMotif::lastEvent() );
	}
	break;
    case QEvent::FocusIn:
    {
	XFocusInEvent ev = { XFocusIn, 0, TRUE, w->x11Display(), w->winId(),
			       NotifyNormal, NotifyPointer  };
	QMotif::redeliverEvent( (XEvent*)&ev );
	break;
    }
    case QEvent::FocusOut:
    {
	XFocusOutEvent ev = { XFocusOut, 0, TRUE, w->x11Display(), w->winId(),
			       NotifyNormal, NotifyPointer  };
	QMotif::redeliverEvent( (XEvent*)&ev );
	break;
    }
    default:
	break;
    }
    return FALSE;
}
