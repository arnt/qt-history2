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

#include "qmotif.h"
#include "qmotifdialog.h"
#include "qmotifwidget.h"

#include <qapplication.h>

#include <X11/StringDefs.h>
#include <Xm/DialogS.h>
#include <Xm/DialogSP.h>

#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Command.h>


XtGeometryResult qmotif_dialog_geometry_manger( Widget,
						XtWidgetGeometry *,
						XtWidgetGeometry * );
void qmotif_dialog_realize( Widget, XtValueMask *, XSetWindowAttributes * );
void qmotif_dialog_insert_child( Widget );
void qmotif_dialog_delete_child( Widget );
void qmotif_dialog_change_managed( Widget );

// XmDialogShell subclass to wrap motif dialogs into QDialogs

typedef struct {
    QMotifDialog *dialog;
} QMotifDialogPart;

typedef struct _QMotifDialogRec
{
    // full instance record declaration
    CorePart			core;
    CompositePart		composite;
    ShellPart			shell;
    WMShellPart			wmshell;
    VendorShellPart		vendorshell;
    TransientShellPart		transientshell;
    XmDialogShellPart		dialogshell;
    QMotifDialogPart		qmotifdialog;
} QMotifDialogRec;

typedef struct
{
    // extension record
    XtPointer extension;
} QMotifDialogClassPart;

typedef struct _QMotifDialogClassRec
{
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ShellClassPart		shell_class;
    WMShellClassPart		wmshell_class;
    VendorShellClassPart	vendorshell_class;
    TransientShellClassPart	transientshell_class;
    XmDialogShellClassPart	dialogshell_class;
    QMotifDialogClassPart	qmotifdialog_class;
} QMotifDialogClassRec;

externalref QMotifDialogClassRec	qmotifDialogShellClassRec;
externalref WidgetClass			qmotifDialogWidgetClass;
typedef struct _QMotifDialogClassRec   *QMotifDialogWidgetClass;
typedef struct _QMotifDialogRec	       *QMotifDialogWidget;

externaldef(qmotifdialogclassrec)
    QMotifDialogClassRec qmotifDialogClassRec = {
	// Core
	{
	    (WidgetClass) &xmDialogShellClassRec,	/* superclass */
	    "QMotifDialog",				/* class_name */
	    sizeof(QMotifDialogRec),			/* widget_size */
	    NULL,					/* class_initialize proc */
	    NULL,					/* class_part_initialize proc */
	    FALSE,					/* class_inited flag */
	    NULL,					/* instance initialize proc */
	    NULL,					/* init_hook proc */
	    qmotif_dialog_realize,			/* realize widget proc */
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
	    NULL,					/* extension record      */
	},
	// Composite
	{
	    qmotif_dialog_geometry_manger,		// geometry_manager
	    qmotif_dialog_change_managed,		// change managed
	    qmotif_dialog_insert_child,			// insert_child
	    qmotif_dialog_delete_child,			// delete_child
	    NULL,					// extension record
	},
	// Shell extension record
	{ NULL },
	// WMShell extension record
	{ NULL },
	// VendorShell extension record
	{ NULL },
	// TransientShell extension record
	{ NULL },
	// XmDialogShell extension record
	{ NULL },
	// QMOTIGDIALOG
	{ NULL }
    };

externaldef(qmotifdialogwidgetclass)
    WidgetClass qmotifDialogWidgetClass = (WidgetClass)&qmotifDialogClassRec;


class QMotifDialogPrivate
{
public:
    QMotifDialogPrivate() : shell( NULL ), dialog( NULL ) { }

    Widget shell;
    Widget dialog;
};

/*!
    \class QMotifDialog
    \brief The QMotifDialog class provides the QDialog API for Motif-based dialogs.

    \extension Motif

    QMotifDialog provides two separate modes of operation. The
    application programmer can use QMotifDialog with an existing
    Motif-based dialog and a QWidget parent, or the application
    programmer can use QMotifDialog with a custom Qt-based dialog and
    a Motif-based parent. Modality continues to work as expected.

    Motif-based dialogs must have a \c Shell widget parent with a
    single child, due to requirements of the Motif toolkit. The \c
    Shell widget, which is a special subclass of \c XmDialogShell, is
    created during construction. It can be accessed using the shell()
    member function.

    The single child of the \c Shell can be accessed using the
    dialog() member function \e after it has been created.

    The acceptCallback() and rejectCallback() functions provide a
    convenient way to call QDialog::accept() and QDialog::reject()
    through callbacks. A pointer to the QMotifDialog should be passed
    as the \c client_data argument to the callback.

    The API and behavior QMotifDialog is identical to that of QDialog
    when using a custom Qt-based dialog with a Motif-based parent.
    The only difference is that a Motif-based \e parent argument is
    passed to the constructor, instead of a QWidget parent.
*/

/*! \obsolete
    \enum QMotifDialog::DialogType

    This enum lists the predefined Motif dialog types.

    \value Prompt
    \value Selection
    \value Command
    \value FileSelection
    \value Template
    \value Error
    \value Information
    \value Message
    \value Question
    \value Warning
    \value Working
*/

/*! \obsolete

    Creates a QMotifDialog using one of the predefined Motif dialog
    types. The \a name, \a modal and \a flags arguments are passed to
    the QDialog constructor.

    This constructor creates a Shell widget, which is a special
    subclass of XmDialogShell. The \a parent, \a args and \a argcount
    arguments are passed to XtCreatePopupShell() when creating the
    subclass. You can access the Shell widget with the shell() member
    function.

    This constructor also creates the dialog widget with the Shell
    widget as its parent. The type of the dialog created is specified
    by the \a dialogtype argument. See the DialogType enum for a list
    of available dialog types. You can access the dialog widget with
    the dialog() member function.

    \warning QMotifDialog takes ownership of the child widget and
    destroys it during destruction. You should not destroy the dialog
    widget yourself.

    \sa DialogType shell() dialog()
*/
QMotifDialog::QMotifDialog( DialogType dialogtype, Widget parent,
			    ArgList args, Cardinal argcount,
			    const char *name, bool modal, WFlags flags )
    : QDialog( 0, name, modal, flags )
{
    init( parent, args, argcount );

    // get the widget class for the dialog type
    WidgetClass widgetclass;
    int dtype;
    switch ( dialogtype ) {
    case Prompt:
	dtype = XmDIALOG_PROMPT;
	widgetclass = xmSelectionBoxWidgetClass;
	break;

    case Selection:
	dtype = XmDIALOG_SELECTION;
	widgetclass = xmSelectionBoxWidgetClass;
	break;

    case Command:
	dtype = XmDIALOG_COMMAND;
	widgetclass = xmCommandWidgetClass;
	break;

    case FileSelection:
	dtype = XmDIALOG_FILE_SELECTION;
	widgetclass = xmFileSelectionBoxWidgetClass;
	break;

    case Template:
	dtype = XmDIALOG_TEMPLATE;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Error:
	dtype = XmDIALOG_ERROR;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Information:
	dtype = XmDIALOG_INFORMATION;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Message:
	dtype = XmDIALOG_MESSAGE;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Question:
	dtype = XmDIALOG_QUESTION;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Warning:
	dtype = XmDIALOG_WARNING;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Working:
	dtype = XmDIALOG_WORKING;
	widgetclass = xmMessageBoxWidgetClass;
	break;
    }

    // set the dialog type
    Arg *realargs = new Arg[ argcount + 1 ];
    memcpy( realargs, args, argcount * sizeof(Arg) );
    XtSetArg(realargs[argcount], XmNdialogType, dtype);
    ++argcount;

    d->dialog = XtCreateWidget( name, widgetclass, d->shell, realargs, argcount );

    delete [] realargs;
    realargs = 0;
}

/*! \obsolete

    Creates a QMotifDialog which allows the application programmer to
    use the Motif-based \a parent for a custom QDialog. The \a name,
    \a modal and \a flags arguments are passed to the QDialog
    constructor.

    This constructor creates a Shell widget, which is a special
    subclass of XmDialogShell. The \a args and \a argcount arguments
    are passed to XtCreatePopupShell() when creating the subclass.
    You can access the Shell widget with the shell() member function.

    The dialog widget is not created by the constructor. You must
    create the dialog widget as a child of the the widget returned by
    shell(). You can access the child widget with the dialog() member
    function.

    A dialog widget is not created by this constructor. Instead, you
    should create the dialog widget as a child of this
    dialog. QMotifDialog will take ownership of your custom dialog,
    and you can access it with the dialog() member function.

    \warning QMotifDialog takes ownership of the child widget and
    destroys it during destruction. You should not destroy the dialog
    widget yourself.

    \sa shell() dialog()
*/
QMotifDialog::QMotifDialog( Widget parent, ArgList args, Cardinal argcount,
			    const char *name, bool modal, WFlags flags )
    : QDialog( 0, name, modal, flags )
{
    init( parent, args, argcount );
}

/*!
    Creates a QMotifDialog which allows the application programmer to
    use the Motif-based \a parent for a custom QDialog. The \a name,
    \a modal and \a flags arguments are passed to the QDialog
    constructor.

    This constructor creates a \c Shell widget, which is a special
    subclass of \c XmDialogShell. You can access the \c Shell widget
    with the shell() member function.

    \sa shell()
*/
QMotifDialog::QMotifDialog( Widget parent, const char *name, bool modal, WFlags flags )
    : QDialog( 0, name, modal, flags )
{
    init( parent );
}

/*!
    Creates a QMotifDialog which allows the application programmer to
    use a QWidget parent for an existing Motif-based dialog. The \a
    parent, \a name, \a modal and \a flags arguments are passed to the
    QDialog constructor.

    This constructor creates a \c Shell widget, which is a special
    subclass of \c XmDialogShell. You can access the \c Shell widget
    with the shell() member functon.

    A dialog widget is not created by this constructor. Instead, you
    should create the dialog widget as a child of this
    dialog. QMotifDialog will take ownership of your custom dialog,
    and you can access it with the dialog() member function.

    \warning QMotifDialog takes ownership of the child widget and
    destroys it during destruction. You should not destroy the dialog
    widget yourself.

    \sa shell() dialog()
*/
QMotifDialog::QMotifDialog( QWidget *parent, const char *name,
			    bool modal, WFlags flags )
    : QDialog( parent, name, modal, flags )
{
    init();
}

/*! \internal

    Initiailizes QMotifDialog by creating the QMotifDialogPrivate data
    and the Shell widget.
*/
void QMotifDialog::init( Widget parent, ArgList args, Cardinal argcount )
{
    d = new QMotifDialogPrivate;

    Arg *realargs = new Arg[ argcount + 3 ];
    memcpy( realargs, args, argcount * sizeof(Arg) );
    if ( ! QPaintDevice::x11AppDefaultVisual() ) {
	// make Motif use the same visual/colormap/depth as Qt (if Qt
	// is not using the default)
	XtSetArg(realargs[argcount], XtNvisual, QPaintDevice::x11AppVisual());
	++argcount;
	XtSetArg(realargs[argcount], XtNcolormap, QPaintDevice::x11AppColormap());
	++argcount;
	XtSetArg(realargs[argcount], XtNdepth, QPaintDevice::x11AppDepth());
	++argcount;
    }

    // create the dialog shell
    if ( parent ) {
	d->shell = XtCreatePopupShell( name(), qmotifDialogWidgetClass, parent,
				       realargs, argcount );
    } else {
	d->shell = XtAppCreateShell( name(), name(), qmotifDialogWidgetClass,
				     QMotif::x11Display(), realargs, argcount );
    }

    ( (QMotifDialogWidget) d->shell )->qmotifdialog.dialog = this;

    delete [] realargs;
    realargs = 0;
}

/*!
    Destroys the QDialog, dialog widget and \c Shell widget.
*/
QMotifDialog::~QMotifDialog()
{
    QMotif::unregisterWidget( this );
    ( (QMotifDialogWidget) d->shell )->qmotifdialog.dialog = 0;
    XtDestroyWidget( d->shell );

    delete d;

    // make sure we don't have any pending requests for the window we
    // are about to destroy
    XSync(x11Display(), FALSE);
    destroy( FALSE );
}

/*!
    Returns the \c Shell widget embedded in this dialog.
*/
Widget QMotifDialog::shell() const
{
    return d->shell;
}

/*!
    Returns the Motif widget embedded in this dialog.
*/
Widget QMotifDialog::dialog() const
{
    return d->dialog;
}

/*!
  \reimp
*/
void QMotifDialog::accept()
{
    QDialog::accept();
}

/*!
  \reimp
*/
void QMotifDialog::reject()
{
    QDialog::reject();
}

/*!
    \reimp

    Manages the dialog widget and shows the dialog.
*/
void QMotifDialog::show()
{
    // tell motif about modality
    Arg args[1];
    XtSetArg( args[0], XmNdialogStyle,
	      ( testWFlags(WShowModal) ? XmDIALOG_FULL_APPLICATION_MODAL :
		XmDIALOG_MODELESS ) );
    XtSetValues( d->shell, args, 1 );

    XtSetMappedWhenManaged( d->shell, False );

    if ( d->dialog ) {
	XtManageChild( d->dialog );

	XSync(x11Display(), FALSE);
	XSync(QMotif::x11Display(), FALSE);
    } else if ( !parentWidget() ) {
	adjustSize();
	QApplication::sendPostedEvents(this, QEvent::LayoutHint);
	QApplication::sendPostedEvents(this, QEvent::Resize);

	Widget p = XtParent( d->shell ), s = p;
	while ( s != NULL && !XtIsShell( s ) ) // find the shell
	    s = XtParent( s );

	if ( p && s ) {
	    int offx = ( (  XtWidth( p ) -  width() ) / 2 );
	    int offy = ( ( XtHeight( p ) - height() ) / 2 );
	    move( XtX ( s ) + offx, XtY( s ) + offy );
	}
    }

    QDialog::show();

    XtSetMappedWhenManaged( d->shell, True );
}

/*!
    \reimp

    Unmanages the dialog and hides the dialog.
*/
void QMotifDialog::hide()
{
    if ( d->dialog )
	XtUnmanageChild( d->dialog );

    QDialog::hide();
}

/*!
    Convenient Xt/Motif callback to accept the QMotifDialog.

    The data is passed in \a client_data.
*/
void QMotifDialog::acceptCallback( Widget, XtPointer client_data, XtPointer )
{
    QMotifDialog *dialog = (QMotifDialog *) client_data;
    dialog->accept();
}

/*!
    Convenient Xt/Motif callback to reject the QMotifDialog.

    The data is passed in \a client_data.
*/
void QMotifDialog::rejectCallback( Widget, XtPointer client_data, XtPointer )
{
    QMotifDialog *dialog = (QMotifDialog *) client_data;
    dialog->reject();
}

/*! \internal
    Wraps the Motif dialog by setting the X window for the
    QMotifDialog to the X window id of the dialog shell.
*/
void QMotifDialog::realize( Widget w )
{
    // use the winid of the dialog shell, reparent any children we have
    if ( XtWindow( w ) != winId() ) {
	XSync(QMotif::x11Display(), FALSE);

	XtSetMappedWhenManaged( d->shell, False );

	// save the caption
	QString cap;
	if ( ! caption().isNull() ) {
	    cap = caption();
	    setCaption( QString::null );
	} else {
	    setCaption( QString::null );
	    XTextProperty text_prop;
	    if (XGetWMName(x11Display(), winId(), &text_prop)) {
		if (text_prop.value && text_prop.nitems > 0) {
		    if (text_prop.encoding == XA_STRING) {
			cap = QString::fromLocal8Bit( (char *) text_prop.value );
		    } else {
			text_prop.nitems = strlen((char *) text_prop.value);

			char **list;
			int num;
			if (XmbTextPropertyToTextList(x11Display(), &text_prop,
						      &list, &num) == Success &&
			    num > 0 && *list) {
			    cap = QString::fromLocal8Bit( *list );
			    XFreeStringList(list);
			}
		    }
		}
	    }
	}

	Window newid = XtWindow(w);
	QObjectList list = children();
	for (int i = 0; i < list.size(); ++i) {
	    QWidget *widget = qt_cast<QWidget*>(list.at(i));
	    if (!widget) continue;

	    XReparentWindow(widget->x11Display(), widget->winId(), newid,
			    widget->x(), widget->y());
	}
	QApplication::syncX();

	create( newid, TRUE, TRUE );

	// restore the caption
	setCaption( cap );

	// if this dialog was created without a QWidget parent, then the transient
	// for will be set to the root window, which is not acceptable.
	// instead, set it to the window id of the shell's parent
	if ( ! parent() && XtParent( d->shell ) )
	    XSetTransientForHint(x11Display(), newid, XtWindow(XtParent(d->shell)));
    }
    QMotif::registerWidget( this );
}

/*! \internal
    Sets the dialog widget for the QMotifDialog to \w.
*/
void QMotifDialog::insertChild( Widget w )
{
#if defined(QT_CHECK_STATE)
    if ( d->dialog != NULL && d->dialog != w ) {
	qWarning( "QMotifDialog::insertChild: cannot insert widget since one child\n"
		  "                           has been inserted already." );
	return;
    }
#endif

    d->dialog = w;

    XtSetMappedWhenManaged( d->shell, True );
}

/*! \internal
    Resets the dialog widget for the QMotifDialog if the current
    dialog widget matches \w.
*/
void QMotifDialog::deleteChild(Widget w)
{
    if (!d->dialog) {
	qWarning("QMotifDialog::deleteChild: cannot delete widget since no child\n"
		 "                           was inserted.");
	return;
    }

    if (d->dialog != w) {
	qWarning("QMotifDialog::deleteChild: cannot delete widget different from\n"
		 "                           inserted child");
	return;
    }

    d->dialog = NULL;
}

/*! \internal
    Motif callback to resolve a QMotifDialog and call
    QMotifDialog::realize().
*/
void qmotif_dialog_realize( Widget w, XtValueMask *mask, XSetWindowAttributes *attr )
{
    XtRealizeProc realize = xmDialogShellClassRec.core_class.realize;
    (*realize)( w, mask, attr );

    QMotifDialog *dialog = ( (QMotifDialogWidget) w )->qmotifdialog.dialog;
    dialog->realize( w );
}

/*! \internal
    Motif callback to forward a QMotifDialog and call
    QMotifDialog::insertChild().
*/
void qmotif_dialog_insert_child( Widget w )
{
    if ( XtIsShell( w ) ) return; // do not allow shell children

    XtWidgetProc insert_child = xmDialogShellClassRec.composite_class.insert_child;
    (*insert_child)( w );

    QMotifDialog *dialog = ( (QMotifDialogWidget) w->core.parent )->qmotifdialog.dialog;
    dialog->insertChild( w );
}

/*! \internal
    Motif callback to resolve a QMotifDialog and call
    QMotifDialog::deleteChild().
*/
void qmotif_dialog_delete_child( Widget w )
{
    if ( XtIsShell( w ) ) return; // do not allow shell children

    XtWidgetProc delete_child = xmDialogShellClassRec.composite_class.delete_child;
    (*delete_child)( w );

    QMotifDialog *dialog = ( (QMotifDialogWidget) w->core.parent )->qmotifdialog.dialog;
    dialog->deleteChild( w );
}

/*! \internal
    Motif callback to resolve a QMotifDialog and set the initial
    geometry of the dialog.
*/
void qmotif_dialog_change_managed( Widget w )
{
    QMotifDialog *dialog = ( (QMotifDialogWidget) w )->qmotifdialog.dialog;

    Widget p = w->core.parent;

    TopLevelShellRec shell;

    if ( p == NULL && dialog->parentWidget() ) {
	// fake a motif widget parent for proper dialog
	// sizing/placement, which happens during change_managed by
	// going through geometry_manager

	QWidget *qwidget = dialog->parentWidget();
	QRect geom = qwidget->geometry();

	memset( &shell, 0, sizeof( shell ) );

	char fakename[] = "fakename";

	shell.core.self = (Widget) &shell;
	shell.core.widget_class = (WidgetClass) &topLevelShellClassRec;
	shell.core.parent = NULL;
	shell.core.xrm_name = w->core.xrm_name;
	shell.core.being_destroyed = False;
	shell.core.destroy_callbacks = NULL;
	shell.core.constraints = NULL;
	shell.core.x = geom.x();
	shell.core.y = geom.y();
	shell.core.width = geom.width();
	shell.core.height = geom.height();
	shell.core.border_width = 0;
	shell.core.managed = True;
	shell.core.sensitive = True;
	shell.core.ancestor_sensitive = True;
	shell.core.event_table = NULL;
	shell.core.accelerators = NULL;
	shell.core.border_pixel = 0;
	shell.core.border_pixmap = 0;
	shell.core.popup_list = NULL;
        shell.core.num_popups = 0;
	shell.core.name = fakename;
	shell.core.screen = ScreenOfDisplay( qwidget->x11Display(),
					     qwidget->x11Screen() );
	shell.core.colormap = qwidget->x11Colormap();
	shell.core.window = qwidget->winId();
	shell.core.depth = qwidget->x11Depth();
	shell.core.background_pixel = 0;
	shell.core.background_pixmap = None;
	shell.core.visible = True;
	shell.core.mapped_when_managed = True;

	w->core.parent = (Widget) &shell;
    }

    XtWidgetProc change_managed = xmDialogShellClassRec.composite_class.change_managed;
    (*change_managed)( w );
    w->core.parent = p;
}

XtGeometryResult qmotif_dialog_geometry_manger( Widget w,
						XtWidgetGeometry *req,
						XtWidgetGeometry *rep )
{
    XtGeometryHandler geometry_manager =
	xmDialogShellClassRec.composite_class.geometry_manager;
    XtGeometryResult result = (*geometry_manager)( w, req, rep );

    QMotifDialog *dialog = ( (QMotifDialogWidget) w->core.parent )->qmotifdialog.dialog;
    dialog->setGeometry( XtX( w ), XtY( w ), XtWidth( w ), XtHeight( w ) );

    return result;
}


/*!\reimp
 */
bool QMotifDialog::event( QEvent* e )
{
    if ( QMotifWidget::dispatchQEvent( e, this ) )
	return TRUE;
    return QDialog::event( e );
}
