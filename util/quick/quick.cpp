#include "quick.h"
#include "quicked.h"
#include "quickpad.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qframe.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qobjcoll.h>
#include <stdlib.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qptrdict.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qtabbar.h>


class QTLWidget : public QWidget {
    QWidget *child;
    enum { margin=4 };
public:
    QTLWidget(QWidget* parent, const char* name=0) :
	QWidget(parent, name)
    {
    }

    void setChild( QWidget* w )
    {
	if (child) removeEventFilter( child );
	w->installEventFilter( this );
	child = w;
	adjust();
    }

private:
    void adjust()
    {
	resize( child->width()+margin*2, child->height()+margin*2 );
	if ( child->pos() != QPoint( margin, margin ) )
	    child->move( margin, margin );
    }

    bool eventFilter( QObject *, QEvent* e )
    {
	if (e->type() == Event_Resize)
	    adjust();
	return FALSE;
    }
};

/*!
  \class Quick quick.h
  \brief The top-level widget in Quick.

  A Quick widget edits a single hierarchy of widgets.
*/

/*!
  Constructs a new Quick.
*/
Quick::Quick(QWidget* parent, const char* name, WFlags f) :
    QWidget(parent,name,f)
{
    changed = FALSE;
    setDetailedCaption(nextUntitled());

    // Menubar setup
    QMenuBar *menubar = new QMenuBar(this);
    menubar->setSeparator(QMenuBar::InWindowsStyle);
    QPopupMenu *file = new QPopupMenu;
    menubar->insertItem("&File", file);
    file->insertItem("&New", this, SIGNAL(newQuick()));
    file->insertItem("&Open", this, SLOT(open()));
    file->insertSeparator();
    file->insertItem("&Save", this, SLOT(save()));
    file->insertItem("Save &as...", this, SLOT(saveAs()));
    file->insertSeparator();
    file->insertItem("&Close", this, SLOT(closeWindow()));
    file->insertItem("&Quit", this, SIGNAL(closeAll()));

    // Listers setup
    QuickPad *pad = new QuickPad(this);
    pad->setBackgroundMode( PaletteDark );

    // Scroller setup
    QScrollView *scroller = new QScrollView(this);
    border = new QTLWidget( scroller );
    border->setBackgroundMode( PaletteDark );
    edited_widget = new QuickEditedWidget(border);
    edited_widget->resize(400,300);
    border->setChild( edited_widget );
    scroller->view(border);

    // Status setup
    QLabel *status = new QLabel("Ready",this);
    status->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    status->setFixedHeight( status->sizeHint().height() );

    // Layout
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->setMenuBar( menubar );
    vbox->addLayout( hbox );
    vbox->addWidget( status );
    hbox->addWidget( pad, 0 );
    hbox->addWidget( scroller, 4 );
    vbox->activate();

    // Connections
    connect( pad, SIGNAL(pulled(QWidgetFactory&)),
	     this, SLOT(dragWidget(QWidgetFactory&)) );
    connect( edited_widget, SIGNAL(changed()),
	     this, SLOT(flagChange()) );

    // Test data...

    QScrollView *sv = new QScrollView(edited_widget);
    sv->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    QWidget *w = new QWidget(sv);
    edited_widget->monitor(w);
    sv->view(w);
    edited_widget->monitor(sv);

    w = new QLabel("clicked()");
    w->setFont(QFont("courier"));
    edited_widget->monitor(w);

    edited_widget->monitor(new QRadioButton("ignore", edited_widget, "ignore"));
    edited_widget->monitor(new QRadioButton("re-emit as signal", edited_widget, "re_emit"));
    edited_widget->monitor(new QRadioButton("connect to slot", edited_widget, "to_slot"));
    //edited_widget->monitor(new QLineEdit(edited_widget, "signal_name"));
    //edited_widget->monitor(new QLineEdit(edited_widget, "slot_name"));
    QComboBox* cb;
    cb = new QComboBox(edited_widget,"protection");
    cb->insertItem("public");
    cb->insertItem("protected");
    edited_widget->monitor(cb);
    cb = new QComboBox(edited_widget,"function_type");
    cb->insertItem("abstract virtual");
    cb->insertItem("unimplemented virtual");
    cb->insertItem("unimplemented");
    cb->insertItem("empty virtual");
    edited_widget->monitor(cb);

    edited_widget->monitor(new QPushButton("Button 1", edited_widget));
    edited_widget->monitor(new QPushButton("Button 2", edited_widget));
}

/*!
  Destructs a new Quick.

  \internal
  GCC 2.7.2 needs this.
*/
Quick::~Quick()
{
}

/*!
  Returns a string which is unique process-wide (not globally).  The string
  is suitable replacement for a filename, but should not be used as a
  filename for saving (it may contain spaces).
*/
QString Quick::nextUntitled()
{
    static int n=0;
    QString result;
    if (n>1)
	result.sprintf("Untitled %d",n);
    else
	result.sprintf("Untitled");
    n++;
    return result;
}

/*!
  Returns TRUE if it is safe to discard the contents of this Quick.
  That is, either the contents has not changed since the last save/open,
  or the user has confirmed the discard.
*/
bool Quick::deleteOkay()
{
    if (!changed) return TRUE;

    QString message;
    message.sprintf("The widget \"%s\" has been modified.%s",
			caption(),
			style()==MotifStyle
			    ? "" : "\nDo you want to save the changes?");
    QMessageBox save_changes("Unsaved!",
			message,
			QMessageBox::Warning,
			QMessageBox::Yes|QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel,
			this);
    if ( style()==MotifStyle ) {
	save_changes.setButtonText(QMessageBox::Yes,"Save");
	save_changes.setButtonText(QMessageBox::No,"Discard");
    }
    switch (save_changes.exec()) {
      case QMessageBox::Cancel:
	break;
      case QMessageBox::Yes:
	save();
	return !changed;
      case QMessageBox::No:
	return TRUE;
    }

    return FALSE;
}

/*!
  Records the fact that the contents has changed.
*/
void Quick::flagChange()
{
    changed = TRUE;
}

/*!
  Reads the widget hierarchy from \a filename, dicarding any current
  contents without confirmation.  If the file cannot be read, and
  \a complain is TRUE, the user is informed, otherwise the filename for
  the contents is changed to \a filename.
*/
void Quick::open(const char* filename, bool complain)
{
    QFile file(filename);
    if (file.open(IO_ReadOnly)) {
	// ... delete old 
	// ... read it
	file.close();
    } else if (complain) {
	// ... error message
	return;
    }
    changed=FALSE;
    setFileName(filename);
}

/*!
  Interacts with the user to read a file.
*/
void Quick::open()
{
    if (deleteOkay()) {
	QString f = QFileDialog::getOpenFileName(0,"*.cpp");
	if (f.isEmpty()) return;
	open(f,TRUE);
    }
}

/*!
  Saves the current contents to the named file.  If \a overwrite
  is FALSE, the user will be asked for confirmation if a file
  with that name already exists.  If an error occurs, the user
  is informed, otherwise the the filename for
  the contents is changed to \a filename.
*/
void Quick::saveAs(const char* filename, bool overwrite)
{
    if (!overwrite) {
	if (QDir::current().exists(filename)) {
	    // ... ask if overwrite is okay
	}
    }

    QFile file(filename);
    if (file.open(IO_WriteOnly)) {
	// ... write it
	file.close();
	setFileName(filename);
	changed=FALSE;
    } else {
	// ... error message
    }
}

/*!
  Sets a caption for this Quick that would be appropriate
  if \a filename was the filename being used to store the contents.
*/
void Quick::setDetailedCaption(const char* filename)
{
    QString c;
    c.sprintf("Quick - %s", filename);
    setCaption(c);
}

/*!
  Sets \a filename as the filename being used to store the contents.
*/
void Quick::setFileName(const char* fn)
{
    setDetailedCaption(fn);
    filename = fn;
}

/*!
  Saves the contents to the current filename, interacting with the
  user if there is no current filename for the contents.
*/
void Quick::save()
{
    if (filename.isNull())
	saveAs();
    else
	saveAs( filename, TRUE );
}

/*!
  Interactively chooses a filename for the contents, and save the contents
  to that filename.
*/
void Quick::saveAs()
{
    QString f = QFileDialog::getSaveFileName(0,"*.cpp",this,filename);
    if (f.isNull()) return;
    saveAs(f,FALSE);
}

/*!
  QWidget::close() as a slot.
*/
void Quick::closeWindow()
{
    close();
}

/*!
  Override to ask for confirmation if required.
*/
void Quick::closeEvent( QCloseEvent*e )
{
    if ( deleteOkay() )
	e->accept();
}

/*!
  \fn void Quick::newQuick() 

  Emitted when the user requests a new Quick window be created.
*/

/*!
  \fn void Quick::closeAll() 

  Emitted when the user requests that all Quick windows be closed.
*/

/*!
  Slot to hit when a factory is providing widgets to drag.
*/
void Quick::dragWidget(QWidgetFactory& factory)
{
    QWidget *w = factory.instance(edited_widget,"no-name");
    QPoint pos = QCursor::pos();
    pos = edited_widget->mapFromGlobal(pos);
    QPoint middle(w->width()/2,w->height()/2);
    pos -= middle;
    w->show();
    w->move( pos );
    edited_widget->monitor(w);
    edited_widget->clearSelection();
    edited_widget->select( w );
    edited_widget->startManip( w, middle );
}
