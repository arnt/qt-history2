#include "qmotif.h"
#include "qmotifdialog.h"
#include "qmotifwidget.h"

#include <qapplication.h>
#include <qobjectlist.h>
#include <qwidgetintdict.h>

#include <Xm/DialogS.h>
#include <Xm/DialogSP.h>

#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Command.h>


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
	    XtInheritGeometryManager,			/* geometry_manager */
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
    \brief The QMotifDialog class provides the QDialog API for Motif dialogs.

    \extension Motif

    QMotifDialog provides the QDialog API for Motif dialogs.
    Applications moving to Qt will need to not only rewrite Motif
    dialogs in Qt, they will also need tom move to the Qt modailty
    semantics.  QMotifDialog ensures that modal Motif dialogs continue
    to work when used in a Qt application.

    For the purpose of the QMotif extension, Motif has 2 types of
    dialogs: predefined dialogs and custom dialogs.  The predefined
    Motif dialogs are:

    \list
    \i Prompt
    \i Selection
    \i Command
    \i FileSelection
    \i Template
    \i Error
    \i Information
    \i Message
    \i Question
    \i Warning
    \i Working
    \endlist

    QMotifDialog provides a constructor for the predefined Motif
    dialog types, which creates a dialog shell and the dialog widget
    itself.

    Example usage QMotifDialog to create a predefined Motif dialog:

    \code
    ...
    XmString message = XmStringCreateLocalized( "This is a Message dialog.",
                                                XmSTRING_DEFAULT_CHARSET );
    Arg args[1];
    XtSetArg( args[0], XmNmessageString, message );

    // parent is an ApplicationShell created earlier in the application
    QMotifDialog dailog( QMotifDialog::Message, parent, args, 1,
                         "motif message dialog", TRUE );
    XtAddCallback( dialog.dialog(), XmNokCallback,
                   (XtCallbackProc) QMotifDialog::acceptCallback, &dialog );
    XtAddCallback( dialog.dialog(), XmNcancelCallback,
                   (XtCallbackProc) QMotifDialog::rejectCallback, &dialog );
    dialog.exec();

    XmStringFree( message );
    ...
    \endcode

    QMotifDialog also provides a constructor for custom Motif dialogs,
    which creates only the dialog shell.  The application programmer
    can create a custom dialog using the QMotifDialog shell as the
    parent.

    QMotifDialog can be used with either an Xt/Motif or a QWidget
    parent.
*/

/*! \enum QMotifDialog::DialogType

    This enum covers the predefined Motif dialog types.

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

/*!
    Creates a predefined Motif dialog of type \a dtype with a Motif
    widget \a parent.

    The arguments are passed in \a args, and the number of arguments
    in \a argcount. The \a name, \a modal and \a flags arguments are
    passed on to the QDialog constructor.

    Creates a Shell widget which is a special subclass of
    XmDialogShell. This allows applications to use the QDialog API
    with existing Motif dialogs. This allows appilications to have
    proper modality handling through the QMotif extension. You can
    access the Shell widget with the shell() member function.

    Creates a dialog widget with the Shell widget as it's parent. The
    type of the dialog created is specified by the \a dtype
    argument. See the \c DialogType enum for a list of available dialog
    types. You can access the dialog widget with the dialog() member
    function.

    \warning When QMotifDialog is destroyed, the Shell widget and the
    dialog widget are destroyed. You should not destroy the dialog
    widget yourself.
*/
QMotifDialog::QMotifDialog( DialogType dtype, Widget parent,
			    ArgList args, Cardinal argcount,
			    const char *name, bool modal, WFlags flags )
    : QDialog( 0, name, modal, flags )
{
    d = new QMotifDialogPrivate;

    // tell motif about modality
    Arg *realargs = new Arg[ argcount + 2 ];
    memcpy( realargs, args, argcount * sizeof(Arg) );
    if ( modal ) {
	XtSetArg( realargs[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
	argcount ++;
    }

    // create the dialog shell
    d->shell = XtCreatePopupShell( name, qmotifDialogWidgetClass, parent,
				   realargs, argcount );
    ( (QMotifDialogWidget) d->shell )->qmotifdialog.dialog = this;

    // get the widget class for the dialog type
    WidgetClass widgetclass;
    int dialogtype;
    switch ( dtype ) {
    case Prompt:
	dialogtype = XmDIALOG_PROMPT;
	widgetclass = xmSelectionBoxWidgetClass;
	break;

    case Selection:
	dialogtype = XmDIALOG_SELECTION;
	widgetclass = xmSelectionBoxWidgetClass;
	break;

    case Command:
	dialogtype = XmDIALOG_COMMAND;
	widgetclass = xmCommandWidgetClass;
	break;

    case FileSelection:
	dialogtype = XmDIALOG_FILE_SELECTION;
	widgetclass = xmFileSelectionBoxWidgetClass;
	break;

    case Template:
	dialogtype = XmDIALOG_TEMPLATE;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Error:
	dialogtype = XmDIALOG_ERROR;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Information:
	dialogtype = XmDIALOG_INFORMATION;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Message:
	dialogtype = XmDIALOG_MESSAGE;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Question:
	dialogtype = XmDIALOG_QUESTION;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Warning:
	dialogtype = XmDIALOG_WARNING;
	widgetclass = xmMessageBoxWidgetClass;
	break;

    case Working:
	dialogtype = XmDIALOG_WORKING;
	widgetclass = xmMessageBoxWidgetClass;
	break;
    }

    // set the dialog type
    XtSetArg( realargs[ argcount ], XmNdialogType, dialogtype );
    argcount++;
    d->dialog = XtCreateWidget( name, widgetclass, d->shell, realargs, argcount );
    delete [] realargs;
}

/*!
    Creates a QMotifDialog for use in a custom Motif dialog.

    The dialog's parent is \a parent. The arguments are passed in \a
    args and the number of arguments in \a argcount. The \a name, \a
    modal and \a flags arguments are passed on to the QDialog
    constructor.

    Creates a Shell widget which is a special subclass of
    XmDialogShell. This allows applications to use the QDialog API
    with existing Motif dialogs. This allows appilications to have
    proper modality handling through the QMotif extension. You can
    access the Shell widget with the shell() member function.

    A dialog widget is not created by this constructor. Instead, you
    should create the dialog widget as a child of this dialog. Once
    you do this, QMotifDialog will take over ownership of your custom
    dialog, and you can access it with the dialog() member function.

    \warning When QMotifDialog is destroyed, the Shell widget and the
    dialog widget are destroyed. You should not destroy the dialog
    widget yourself.
*/
QMotifDialog::QMotifDialog( Widget parent, ArgList args, Cardinal argcount,
			    const char *name, bool modal, WFlags flags )
    : QDialog( 0, name, modal, flags )
{
    d = new QMotifDialogPrivate;

    // tell motif about modality
    Arg *realargs = new Arg[ argcount + 1 ];
    memcpy( realargs, args, argcount * sizeof(Arg) );
    if ( modal ) {
	XtSetArg( realargs[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
	argcount++;
    }

    // create the dialog shell
    d->shell = XtCreatePopupShell( name, qmotifDialogWidgetClass, parent,
				   realargs, argcount );
    ( (QMotifDialogWidget) d->shell )->qmotifdialog.dialog = this;
    delete [] realargs;
}

/*!
   Destroys the QDialog, dialog widget and shell widget.
*/
QMotifDialog::~QMotifDialog()
{
    QMotif::unregisterWidget( this );
    ( (QMotifDialogWidget) d->shell )->qmotifdialog.dialog = 0;
    XtDestroyWidget( d->shell );
    delete d;

    destroy( FALSE );
}

/*!
    Returns the Shell widget embedded in this dialog.
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

/*! \reimp
    Manages the dialog widget and shows the dialog.
*/
void QMotifDialog::show()
{
    QMotifDialogWidget motifdialog = (QMotifDialogWidget) d->shell;
    XtManageChildren( motifdialog->composite.children,
		      motifdialog->composite.num_children );

    QDialog::show();
}

/*! \reimp
    Unmanages the dialog and hides the dialog.
*/
void QMotifDialog::hide()
{
    QMotifDialogWidget motifdialog = (QMotifDialogWidget) d->shell;
    XtUnmanageChildren( motifdialog->composite.children,
			motifdialog->composite.num_children );

    QDialog::hide();
}

/*!
    Convenient Xt/Motif callback to accept the QMotifDialog.

    The widget is passed in \a widget and the data in \a client_data.
*/
void QMotifDialog::acceptCallback( Widget, XtPointer client_data, XtPointer )
{
    QMotifDialog *dialog = (QMotifDialog *) client_data;
    dialog->accept();
}

/*!
    Convenient Xt/Motif callback to reject the QMotifDialog.

    The widget is passed in \a widget and the data in \a client_data.
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

	// if this dialog was created without a QWidget parent, then the transient
	// for will be set to the root window, which is not acceptable.
	// instead, set it to the window id of the shell's parent
	if ( ! parent() && XtParent( d->shell ) )
	    XSetTransientForHint( QPaintDevice::x11AppDisplay(), newid,
				  XtWindow( XtParent( d->shell ) ) );
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
}

/*! \internal
    Resets the dialog widget for the QMotifDialog if the current
    dialog widget matches \w.
*/
void QMotifDialog::deleteChild( Widget w )
{
#if defined(QT_CHECK_STATE)
    if ( ! d->dialog ) {
	qWarning( "QMotifDialog::deleteChild: cannot delete widget since no child\n"
		  "                           was inserted." );
	return;
    }

    if ( d->dialog != w ) {
	qWarning( "QMotifDialog::deleteChild: cannot delete widget different from\n"
		  "                           inserted child" );
	return;
    }
#endif

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

    QMotifDialog *dialog =
	( (QMotifDialogWidget) w )->qmotifdialog.dialog;
    if ( ! dialog )
	return;
    dialog->realize( w );
}

/*! \internal
    Motif callback to forward a QMotifDialog and call
    QMotifDialog::insertChild().
*/
void qmotif_dialog_insert_child( Widget w )
{
    XtWidgetProc insert_child = xmDialogShellClassRec.composite_class.insert_child;
    (*insert_child)( w );

    QMotifDialog *dialog =
	( (QMotifDialogWidget) w->core.parent )->qmotifdialog.dialog;
    if ( ! dialog )
	return;
    dialog->insertChild( w );
}

/*! \internal
    Motif callback to resolve a QMotifDialog and call
    QMotifDialog::deleteChild().
*/
void qmotif_dialog_delete_child( Widget w )
{
    XtWidgetProc delete_child = xmDialogShellClassRec.composite_class.delete_child;
    (*delete_child)( w );

    QMotifDialog *dialog =
	( (QMotifDialogWidget) w->core.parent )->qmotifdialog.dialog;
    if ( ! dialog )
	return;
    dialog->deleteChild( w );
}

/*! \internal
    Motif callback to resolve a QMotifDialog and set the initial
    geometry of the dialog.
*/
void qmotif_dialog_change_managed( Widget w )
{
    XtWidgetProc change_managed = xmDialogShellClassRec.composite_class.change_managed;
    (*change_managed)( w );

    QMotifDialog *dialog =
	( (QMotifDialogWidget) w )->qmotifdialog.dialog;
    if ( ! dialog )
	return;
    QRect r( dialog->d->shell->core.x,
	     dialog->d->shell->core.y,
	     dialog->d->shell->core.width,
	     dialog->d->shell->core.height ),
	d = dialog->geometry();
    if ( d != r ) {
	dialog->setGeometry( r );
    }
}

/*!\reimp
 */
bool QMotifDialog::event( QEvent* e )
{
    if ( QMotifWidget::dispatchQEvent( e, this ) )
	return TRUE;
    return QWidget::event( e );
}
