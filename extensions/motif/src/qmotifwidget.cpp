#include "qmotifwidget.h"
#include "qmotif.h"

#include <qobjectlist.h>
#include <qwidgetintdict.h>

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <Xm/RowColumn.h>

#include <X11/Xatom.h>

// ApplicationShell subclass to wrap toplevel motif widgets into QWidgets

typedef struct {
    QMotifWidget *widget;
} QMotifWidgetShellPart;

typedef struct _QMotifWidgetShellRec
{
    // full instance record declaration
    CorePart                    core;
    CompositePart               composite;
    ShellPart                   shell;
    WMShellPart                 wmshell;
    VendorShellPart             vendorshell;
    TopLevelShellPart           toplevelshell;
    ApplicationShellPart        applicationshell;
    QMotifWidgetShellPart       qmotifwidgetshell;
} QMotifWidgetShellRec;

typedef struct
{
    // extension record
    XtPointer extension;
} QMotifWidgetShellClassPart;

typedef struct _QMotifWidgetShellClassRec {
    CoreClassPart               core_class;
    CompositeClassPart          composite_class;
    ShellClassPart              shell_class;
    WMShellClassPart            wmshell_class;
    VendorShellClassPart        vendorshell_class;
    TopLevelShellClassPart      toplevelshell_class;
    ApplicationShellClassPart   applicationshell_class;
    QMotifWidgetShellClassPart  qmotifwidgetshell_class;
} QMotifWidgetShellClassRec;

externalref QMotifWidgetShellClassRec           qmotifWidgetShellClassRec;
externalref WidgetClass                         qmotifWidgetShellWidgetClass;
typedef struct _QMotifWidgetShellClassRec       *QMotifWidgetShellWidgetClass;
typedef struct _QMotifWidgetShellRec            *QMotifWidgetShellWidget;

externaldef(qmotifwidgetshellclassrec)
    QMotifWidgetShellClassRec qmotifWidgetShellClassRec = {
        // core
        {
            (WidgetClass) &applicationShellClassRec,    // superclass
            "QMotifWidgetShell",                        // class name
            sizeof(QMotifWidgetShellRec),               // widget size
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
	    XtExposeCompressSeries,		 	/* compressed exposure */
	    FALSE,					/* do compress enter-leave */
	    FALSE,					/* do have visible_interest */
	    NULL,					/* destroy widget proc */
	    XtInheritResize,				/* resize widget proc */
	    NULL,					/* expose proc */
	    NULL,		 			/* set_values proc */
	    NULL,					/* set_values_hook proc */
	    XtInheritSetValuesAlmost,			/* set_values_almost proc */
	    NULL,					/* get_values_hook */
	    NULL,					/* accept_focus proc */
	    XtVersion,					/* current version */
	    NULL,					/* callback offset    */
	    XtInheritTranslations,			/* default translation table */
	    XtInheritQueryGeometry,			/* query geometry widget proc */
	    NULL,					/* display accelerator    */
	    NULL					/* extension record      */
        },
        // composite
        {
            XtInheritGeometryManager,			/* geometry_manager */
	    qmotif_widget_shell_change_managed,		// change managed
	    XtInheritInsertChild,                       // insert_child
	    XtInheritDeleteChild,               	// delete_child
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
        // applicationshell extension record
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
    QWidgetIntDict wdict;
};

/*!
    \class QMotifWidget
    \brief The QMotifWidget class provides the QWidget API for Xt/Motif widgets.

    \extension QMotif

    QMotifWidget has a single purpose, to provide a QWidget that can
    act as a parent for any Xt/Motif widget.  Since the QMotifWidget
    is a full QWidget, it an be used as a toplevel (e.g. zero parent) or as a
    child of any other QWwidget.

    An Xt/Motif widget with a toplevel QMotifWidget parent can begin
    using the standard Qt dialogs and custom QDialogs while keeping
    the main Xt/Motif interface of the application.  Using a
    QMotifWidget as the parent for the various QDialogs will ensure
    that modality and stacking works properly throughout the entire
    application.

    Applications moving to Qt can have custom Xt/Motif widgets that
    will take time to rewrite with Qt.  Such applications can use
    these custom widgets as QMotifWidget with QWidget parents.  This
    allows the interface of the application to be replaced gradually.
*/

/*!
    Creates a QMotifWidget of the given \a widgetclass as a child of
    \a parent, with the name \a name and widget flags \a flags.

    The \a args and \a argcount arguments are passed on to
    XtCreateWidget.

    The motifWidget() function returns the resulting Xt/Motif widget.
    This widget can be used as a parent for any other Xt/Motif widget.

    If \a parent is a QMotifWidget, the Xt/Motif widget is created as
    a child of the parent's motifWidget().  If \ parent is zero or a
    normal QWidget, the Xt/Motif widget is created as a child of a
    special ApplicationShell widget.  Xt/Motif widgets can use this
    special ApplicationShell parent as the parent for existing
    Xt/Motif dialogs or QMotifDialogs.
*/
QMotifWidget::QMotifWidget( QWidget *parent, WidgetClass widgetclass,
                            ArgList args, Cardinal argcount,
                            const char *name, WFlags flags )
    : QWidget( parent, name, flags )
{
    d = new QMotifWidgetPrivate;

    Widget motifparent = NULL;
    if ( parent && parent->inherits( "QMotifWidget" ) )
        motifparent = ( (QMotifWidget *) parent )->motifWidget();

    if ( ! motifparent ) {
        d->shell = XtAppCreateShell( name, name, qmotifWidgetShellWidgetClass,
                                     QPaintDevice::x11AppDisplay(),
                                     args, argcount );
        ( (QMotifWidgetShellWidget) d->shell )->qmotifwidgetshell.widget = this;
        motifparent = d->shell;
    }

    if ( widgetclass == applicationShellWidgetClass )
	d->widget = d->shell;
    else
	d->widget = XtCreateWidget( name, widgetclass, motifparent, args, argcount );
}

/*!
    Destroys the QMotifWidget.  The special ApplicationShell is also
    destroyed, if it was created during construction.
*/
QMotifWidget::~QMotifWidget()
{
    if ( d->shell ) {
        ( (QMotifWidgetShellWidget) d->shell )->qmotifwidgetshell.widget = 0;
        XtDestroyWidget( d->shell );
    }
    XtDestroyWidget( d->widget );
    delete d;

    destroy( FALSE );
}

/*!
    Returns the embedded Xt/Motif widget.  If a Shell widget was
    created by the constructor, you can access it with XtParent().
*/
Widget QMotifWidget::motifWidget() const
{
    return d->widget;
}

/*!
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
    Unmanaged the embedded Xt/Motif widget and hides the widget.
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

/*!
    Delivers \a event to the widget, or to any Xt/Motif child.
*/
bool QMotifWidget::x11Event( XEvent *event )
{
    // here, lookup a the event window id to see if we have a child motif widget,
    // and if so, resend the event... the QMotifEventLoop has a reentrancy
    // check, so all we need to do is use XtDispatchEvent

    if ( d->wdict.find( event->xany.window ) ) {
	if ( QMotif::redeliverEvent( event ) )
	    return TRUE;
	// Xt didn't handle the event, so we pass it onto Qt instead
    } else if ( event->xany.window == winId() ) {
	// for the dialog window itself, let Xt and Qt process the event
	QMotif::redeliverEvent( event );
    }

    return QWidget::x11Event( event );
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
		    if ( widget->isVisible() )
			XMapWindow( QPaintDevice::x11AppDisplay(), widget->winId() );
		}
	    }
	}

        // re-create this QWidget with the winid from the motif
        // widget... the geometry will be reset to roughly 1/4 of the
        // screen, so we need to restore it below
	create( newid, TRUE, TRUE );

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

	setCaption( cap );

        // restore geometry of the shell
	XMoveResizeWindow( QPaintDevice::x11AppDisplay(), winId(),
			   save.x(), save.y(), save.width(), save.height() );

	// if this QMotifWidget has a parent widget, we should
	// reparent the shell into that parent
	if ( parentWidget() ) {
	    XReparentWindow( QPaintDevice::x11AppDisplay(),
			     winId(),
			     parentWidget()->winId(),
			     x(), y() );
	}
    }
}

/*! \internal
    Motif callback to resolve a QMotifWidget and call
    QMotifWidget::realize().
*/
void qmotif_widget_shell_realize( Widget w, XtValueMask *mask,
                                  XSetWindowAttributes *attr )
{
    XtRealizeProc realize =
        ((CoreWidgetClass)applicationShellClassRec.core_class.
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
        ((CompositeWidgetClass)applicationShellClassRec.core_class.
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
	widget->setGeometry( r );
    }
}
