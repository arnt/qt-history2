#include "qmotifwidget.h"
#include "qmotif.h"

#include <qobjectlist.h>
#include <qwidgetintdict.h>

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>


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



// any kind of Motif widget that has a QWidget parent
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

    d->widget = XtCreateWidget( name, widgetclass, motifparent, args, argcount );
}

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

Widget QMotifWidget::motifWidget() const
{
    return d->widget;
}

void QMotifWidget::show()
{
    if ( d->shell ) {
        // since we have a shell, we need to manage it's children
        QMotifWidgetShellWidget motifshell = (QMotifWidgetShellWidget) d->shell;
        XtManageChildren( motifshell->composite.children,
                          motifshell->composite.num_children );
    }

    QWidget::show();
}

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

void QMotifWidget::realize( Widget w )
{
    // use the winid of the dialog shell, reparent any children we have
    if ( XtWindow( w ) != winId() ) {
        // save the geometry of the motif widget, since it has the geometry we want
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

        // re-create this QWidget with the winid from the motif widget... the geometry
        // will be reset to roughly 1/4 of the screen, so we need to restore it below
	create( newid );

        // restore geometry of the shell
        XMoveResizeWindow( QPaintDevice::x11AppDisplay(), winId(),
                           save.x(), save.y(), save.width(), save.height() );
    }
}

// motif callback
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

// motif callback
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
    widget->setGeometry( widget->d->shell->core.x,
			 widget->d->shell->core.y,
			 widget->d->shell->core.width,
			 widget->d->shell->core.height );
}
