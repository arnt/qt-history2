/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlist.h>

#include "qxtwidget.h"

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <qx11info_x11.h>

typedef struct {
    int empty;
} QWidgetClassPart;

typedef struct _QWidgetClassRec {
    CoreClassPart	core_class;
    QWidgetClassPart	qwidget_class;
} QWidgetClassRec;

//static QWidgetClassRec qwidgetClassRec;

typedef struct {
    /* resources */
    /* (none) */
    /* private state */
    QXtWidget* qxtwidget;
} QWidgetPart;

typedef struct _QWidgetRec {
    CorePart	core;
    QWidgetPart	qwidget;
} QWidgetRec;


static void reparentChildrenOf(QWidget* parent)
{
    const QObjectList children = parent->children();
    if (children.isEmpty())
        return; // nothing to do
    for (int i = 0; i < children.size(); ++i) {
	QWidget *widget = qobject_cast<QWidget *>(children.at(i));
	if (! widget)
            continue;
       	XReparentWindow(widget->x11Info().display(), widget->winId(),
                        parent->winId(), widget->x(), widget->y());
    }
}

void qwidget_realize(
	Widget                widget,
	XtValueMask*          mask,
	XSetWindowAttributes* attributes
    )
{
    widgetClassRec.core_class.realize(widget, mask, attributes);
    QXtWidget* qxtw = ((QWidgetRec*)widget)->qwidget.qxtwidget;
    if (XtWindow(widget) != qxtw->winId()) {
	qxtw->create(XtWindow(widget), false, false);
	reparentChildrenOf(qxtw);
    }
    qxtw->show();
    XMapWindow( QX11Info::display(), qxtw->winId() );
}

static
QWidgetClassRec qwidgetClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	(char*)"QWidget",
    /* widget_size		*/	sizeof(QWidgetRec),
    /* class_initialize		*/	0,
    /* class_part_initialize	*/	0,
    /* class_inited		*/	FALSE,
    /* initialize		*/	0,
    /* initialize_hook		*/	0,
    /* realize			*/	qwidget_realize,
    /* actions			*/	0,
    /* num_actions		*/	0,
    /* resources		*/	0,
    /* num_resources		*/	0,
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	0,
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	0,
    /* set_values_hook		*/	0,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	0,
    /* accept_focus		*/	XtInheritAcceptFocus,
    /* version			*/	XtVersion,
    /* callback_private		*/	0,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	0
  },
  { /* qwidget fields */
    /* empty			*/	0
  }
};
static WidgetClass qWidgetClass = (WidgetClass)&qwidgetClassRec;


/*!
  \class QXtWidget qxt.h
  \brief The QXtWidget class allows mixing of Xt/Motif and Qt widgets.
  \obsolete

  \extension Motif

  QXtWidget acts as a bridge between Xt and Qt. For utilizing old Xt
  widgets, it can be a QWidget based on a Xt widget class. For
  including Qt widgets in an existing Xt/Motif application, it can be
  a special Xt widget class that is a QWidget.  See the constructors
  for the different behaviors.

  \section2 Known QXtWidget Problems

  This class is unsupported and has many known problems and
  limitations.  It is provided only to keep existing source working;
  it should not be used in new code.  Note: These problems will \e not
  be fixed in future releases.

  Below is an imcomplete list of know issues:

  \list 1

  \i Keyboard focus navigation is impossible when using QXtWidget.
  The mouse must be used to focus widgets in both Qt and Xt/Motif
  widgets.  For example, when embedding a QXtWidget into an Xt/Motif
  widget, key events will go to the QXtWidget (and its children) while
  the mouse is over the QXtWidget, regardless of where Xt/Motif has
  placed the focus.

  \i Reparenting does not work.  You cannot use
  QWidget::reparent(). This includes the functions
  QWidget::showFullScreen() and QWidget::showNormal(), which use
  QWidget::reparent().

  \endlist
*/

void QXtWidget::init(const char* name, WidgetClass widget_class,
		    Widget parent, QWidget* qparent,
		    ArgList args, Cardinal num_args,
		    bool managed)
{
    need_reroot=false;
    xtparent = 0;
    if ( parent ) {
	Q_ASSERT(!qparent);
	xtw = XtCreateWidget(name, widget_class, parent, args, num_args);
	if ( widget_class == qWidgetClass )
	    ((QWidgetRec*)xtw)->qwidget.qxtwidget = this;
	xtparent = parent;
	if (managed)
	    XtManageChild(xtw);
    } else {
	Q_ASSERT(!managed);

	String n, c;
	XtGetApplicationNameAndClass(QX11Info::display(), &n, &c);
	xtw = XtAppCreateShell(n, c, widget_class, QX11Info::display(),
			       args, num_args);
	if ( widget_class == qWidgetClass )
	    ((QWidgetRec*)xtw)->qwidget.qxtwidget = this;
    }

    if ( qparent ) {
	XtResizeWidget( xtw, 100, 100, 0 );
	XtSetMappedWhenManaged(xtw, False);
	XtRealizeWidget(xtw);
	XSync(QX11Info::display(), False);    // I want all windows to be created now
	XReparentWindow(QX11Info::display(), XtWindow(xtw), qparent->winId(), x(), y());
	XtSetMappedWhenManaged(xtw, True);
	need_reroot=true;
    }

    Arg reqargs[20];
    Cardinal nargs=0;
    XtSetArg(reqargs[nargs], XtNx, x());	nargs++;
    XtSetArg(reqargs[nargs], XtNy, y());	nargs++;
    XtSetArg(reqargs[nargs], XtNwidth, width());	nargs++;
    XtSetArg(reqargs[nargs], XtNheight, height());	nargs++;
    //XtSetArg(reqargs[nargs], "mappedWhenManaged", False);	nargs++;
    XtSetValues(xtw, reqargs, nargs);

    //#### destroy();   MLK

    if (!parent || XtIsRealized(parent))
	XtRealizeWidget(xtw);
}

/*!
  Constructs a QXtWidget of the special Xt widget class known as
  "QWidget" to the resource manager.

  Use this constructor to utilize Qt widgets in an Xt/Motif
  application.  The QXtWidget is a QWidget, so you can create
  subwidgets, layouts, etc. using Qt functionality.

  The \a name is the object name passed to the QWidget constructor.
  The widget's parent is \a parent.

  If the \a managed parameter is true and \a parent in not null,
  XtManageChild it used to manage the child.
*/
QXtWidget::QXtWidget(const char* name, Widget parent, bool managed)
    : QWidget( 0 ), xtw( 0 )
{
    setObjectName(name);
    init(name, qWidgetClass, parent, 0, 0, 0, managed);
    Arg reqargs[20];
    Cardinal nargs=0;
    XtSetArg(reqargs[nargs], XtNborderWidth, 0);            nargs++;
    XtSetValues(xtw, reqargs, nargs);
}

/*!
  Constructs a QXtWidget of the given \a widget_class called \a name.

  Use this constructor to utilize Xt or Motif widgets in a Qt
  application.  The QXtWidget looks and behaves
  like the Xt class, but can be used like any QWidget.

  Note that Xt requires that the most top level Xt widget is a shell.
  This means, if \a parent is a QXtWidget, the \a widget_class can be
  of any kind. If there isn't a parent or the parent is just a normal
  QWidget, \a widget_class should be something like \c
  topLevelShellWidgetClass.

  The arguments, \a args, \a num_args are passed on to XtCreateWidget.

  If the \a managed parameter is true and \a parent in not null,
  XtManageChild it used to manage the child.
*/
QXtWidget::QXtWidget(const char* name, WidgetClass widget_class,
		     QWidget *parent, ArgList args, Cardinal num_args,
		     bool managed)
    : QWidget( parent ), xtw( 0 )
{
    setObjectName(name);
    if ( !parent )
	init(name, widget_class, 0, 0, args, num_args, managed);
    else if ( parent->inherits("QXtWidget") )
	init(name, widget_class, ( (QXtWidget*)parent)->xtw , 0, args, num_args, managed);
    else
	init(name, widget_class, 0, parent, args, num_args, managed);
    create(XtWindow(xtw), false, false);
}

/*!
  Destructs the QXtWidget.
*/
QXtWidget::~QXtWidget()
{
    // Delete children first, as Xt will destroy their windows
    QList<QWidget *> list = qFindChildren<QWidget *>(0);
    for (int i = 0; i < list.size(); ++i) {
	QWidget *c;
        while ((c = qobject_cast<QWidget*>(list.at(i))) != 0)
            delete c;
    }

    if ( need_reroot ) {
	hide();
	XReparentWindow(QX11Info::display(), winId(), qApp->desktop()->winId(), x(), y());
    }

    XtDestroyWidget(xtw);
    destroy( false, false );
}

/*!
  \fn Widget QXtWidget::xtWidget() const

  Returns the Xt widget equivalent for the Qt widget.
*/



/*!
  Reimplemented to produce the Xt effect of getting focus when the
  mouse enters the widget. The event is passed in \a e.
*/
bool QXtWidget::x11Event( XEvent * e )
{
    if ( e->type == EnterNotify ) {
	if  ( xtparent )
	    activateWindow();
    }
    return QWidget::x11Event( e );
}


/*!
  Implement a degree of focus handling for Xt widgets.
*/
void QXtWidget::activateWindow()
{
    if  ( xtparent ) {
	if ( !QWidget::isActiveWindow() && isActiveWindow() ) {
	    XFocusChangeEvent e;
	    e.type = FocusIn;
	    e.window = winId();
	    e.mode = NotifyNormal;
	    e.detail = NotifyInferior;
	    XSendEvent( QX11Info::display(), e.window, TRUE, NoEventMask, (XEvent*)&e );
	}
    } else {
	QWidget::activateWindow();
    }
}

/*!
  Different from QWidget::isActiveWindow()
 */
bool QXtWidget::isActiveWindow() const
{
    Window win;
    int revert;
    XGetInputFocus( QX11Info::display(), &win, &revert );

    if ( win == None) return false;

    QWidget *w = find( (WId)win );
    if ( w ) {
	// We know that window
	return w->window() == window();
    } else {
	// Window still may be a parent (if top-level is foreign window)
	Window root, parent;
	Window cursor = winId();
	Window *ch;
	unsigned int nch;
	while ( XQueryTree(QX11Info::display(), cursor, &root, &parent, &ch, &nch) ) {
	    if (ch) XFree( (char*)ch);
	    if ( parent == win ) return true;
	    if ( parent == root ) return false;
	    cursor = parent;
	}
	return false;
    }
}

/*!\reimp
 */
void QXtWidget::moveEvent( QMoveEvent* )
{
    if ( xtparent || !xtw )
	return;
    XConfigureEvent c;
    c.type = ConfigureNotify;
    c.event = winId();
    c.window = winId();
    c.x = geometry().x();
    c.y = geometry().y();
    c.width = width();
    c.height = height();
    c.border_width = 0;
    XSendEvent( QX11Info::display(), c.event, TRUE, NoEventMask, (XEvent*)&c );
    XtMoveWidget( xtw, x(), y() );
}

/*!\reimp
 */
void QXtWidget::resizeEvent( QResizeEvent* )
{
    if ( xtparent || !xtw )
	return;
    XtWidgetGeometry preferred;
    (void ) XtQueryGeometry( xtw, 0, &preferred );
    XConfigureEvent c;
    c.type = ConfigureNotify;
    c.event = winId();
    c.window = winId();
    c.x = geometry().x();
    c.y = geometry().y();
    c.width = width();
    c.height = height();
    c.border_width = 0;
    XSendEvent( QX11Info::display(), c.event, TRUE, NoEventMask, (XEvent*)&c );
    XtResizeWidget( xtw, width(), height(), preferred.border_width );
}
