#include "prefs.h"
#include <qpainter.h>
#include <qapp.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbt.h>
#include <qlabelled.h>
#include <qbuttonrow.h>
#include <qlayout.h>

class StrongHeading : public QHBox {
public:
    StrongHeading(const char* label, const char* desc, QWidget* parent=0, const char* name=0) :
	QHBox(parent, name)
    {
	QLabel* l = new QLabel(label, this);
	QLabel* d = new QLabel(desc, this);
	QPalette p = palette();
	QColorGroup n = palette().normal();
	QColorGroup g(n.background(), n.foreground(), n.light(), n.dark(),
		      n.mid(), n.background(), n.base());
	p.setNormal( g );
	setPalette(p);
	l->setPalette(p);
	d->setPalette(p);
	l->setMargin(3);
	d->setMargin(2);

	QFont bold = *QApplication::font();
	bold.setBold(TRUE);
	bold.setPointSize(bold.pointSize()+2);
	l->setFont( bold );

	l->setFixedSize(l->sizeHint());
    }
};

static QWidget* advanced()
{
    QVBox *vbox = new QVBox;
    StrongHeading *l = new StrongHeading( "Advanced", "Change preferences that affect the entire product", vbox );

    QVBox *box;

    QLabelled *frame = new QLabelled( vbox );
    box = new QVBox( frame );
    new QCheckBox( "&Automatically load images and other data types\n"
	   "(Otherwise, click the Images button to load when needed)", box );
    new QCheckBox( "Enable &Java", box );
    new QCheckBox( "Enable Ja&vaScript", box );
    new QCheckBox( "Enable &Style Sheets", box );
    new QCheckBox( "Enable Auto&Install", box );
    new QCheckBox( "Send email address as anonymous &FTP password", box );

    box = new QVBox( new QLabelled( "Cookies", vbox ) );
    new QRadioButton( "Accept all &cookies", box );
    new QRadioButton( "&Only accept cookies originating from the same server as\n"
		      "the page being viewed", box );
    new QRadioButton( "&Do not accept or send cookies", box );
    new QCheckBox( "&Warn me before accepting a cookie", box );

    return vbox;
}

class DummyCategory : public QLabel {
public:
    DummyCategory( const char * s = 0 ) :
	QLabel(s?s:"Dummy")
    {
	setMinimumSize(100,100);
    }
};

Preferences::Preferences(QWidget* parent, const char* name, int f) :
    QDialog(parent, name, f)
{
    QGridLayout* grid = new QGridLayout(this,1,1,5);
    QVBox*       vbox = new QVBox(this);
    grid->addWidget(vbox,0,0);
    grid->activate();

    QHBox*        hbox = new QHBox(vbox);
    QListView*    selector = new QListView(hbox);
    categories = new QWidgetStack(hbox);
    QButtonRow*   buttons = new QButtonRow(vbox);
    QPushButton*  ok = new QPushButton("OK",buttons);
    QPushButton*  cancel = new QPushButton("Cancel",buttons);

    ok->setDefault(TRUE);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    QFontMetrics fm=fontMetrics();
    int w = fm.width("New Page Colors ")+selector->treeStepSize()*2;
    selector->setColumn( "Category", w );
#define LISTVIEW_BUG_FIXED
#ifdef LISTVIEW_BUG_FIXED
    selector->setMaximumWidth( w );
#else
    selector->setColumn( "Bug", 30 );
    selector->setMaximumWidth( w+30 );
#endif
    selector->setRootIsDecorated( TRUE );
    selector->setFocusPolicy( QWidget::StrongFocus );

    PreferenceItem *group;

    QVBox *appearance = new QVBox;
    {
	QLabel *l = new QLabel( "Appearance   Change the appearance of the display" , appearance );
	QLabelled *frame = new QLabelled( "On startup, launch", appearance );
	QVBox *box = new QVBox( frame );
	new QCheckBox( "&Navigator", box );
	new QCheckBox( "&Messenger Mailbox", box );
	new QCheckBox( "Collabra &Discussions", box );
	new QCheckBox( "Page &Composer", box );
	new QCheckBox( "N&etcaster", box );
	frame = new QLabelled( "Show Toolbar As", appearance );
	box = new QVBox( frame );
	new QRadioButton( "&Pictures and Text", box );
	new QRadioButton( "Pictures &Only", box );
	new QRadioButton( "&Text Only", box );
    }
    QVBox *navigator = new QVBox;
    {
	QLabel *l = new QLabel( "Navigator   Specify the home page location",
				navigator );
	QLabelled *frame = new QLabelled( "Browser starts with", navigator );
	QVBox *box = new QVBox( frame );
	new QRadioButton( "Blank page", box );
	new QRadioButton( "Home page", box );
	new QRadioButton( "Last page visited", box );

	frame = new QLabelled( "Home Page", navigator );
	box = new QVBox( frame );
	new QLabel( "Clicking the Home button will take you to this page",
		    box );
	QHBox *hbox = new QHBox( box );
	new QLabel( "Location:", hbox );
	new QLineEdit( hbox );
	hbox = new QHBox( box );
	new QPushButton( "Use Current Page" );
	new QPushButton ( "Choose" );

	frame = new QLabelled( "History", navigator );
	hbox = new QHBox( frame );
	new QLabel( "History expires after", hbox );
	new QLineEdit( hbox );
	new QLabel( "days", hbox );
	new QPushButton( "Clear History", hbox );

    }
    add(group = new PreferenceItem(selector, "Appearance"), 
	appearance );
    add(new PreferenceItem(group, "Font"), new DummyCategory("Font"));
    add(new PreferenceItem(group, "Colors"), new DummyCategory("Colors"));
    add(group = new PreferenceItem(selector, "Navigator"), navigator);
    add(new PreferenceItem(group, "Languages"), 
	new DummyCategory("Languages"));
    add(new PreferenceItem(group, "Applications"), 
	new DummyCategory("Applications"));
    add(group = new PreferenceItem(selector, "Mail & Groups"), new DummyCategory);
    add(new PreferenceItem(group, "Identity"), new DummyCategory);
    add(new PreferenceItem(group, "Messages"), new DummyCategory);
    add(new PreferenceItem(group, "Mail Server"), new DummyCategory);
    add(new PreferenceItem(group, "Groups Server"), new DummyCategory);
    add(new PreferenceItem(group, "Directory"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Composer"), new DummyCategory);
    add(new PreferenceItem(group, "New Page Colors"), new DummyCategory);
    add(new PreferenceItem(group, "Publish"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Advanced"), advanced());
    add(new PreferenceItem(group, "Cache"), new DummyCategory);
    add(new PreferenceItem(group, "Proxies"), new DummyCategory);
    add(new PreferenceItem(group, "Disk Space"), new DummyCategory);

    setCaption("Netscape: Preferences");
}

void Preferences::add( PreferenceItem* item, QWidget* stack_item)
{
    categories->addWidget(stack_item, item->id());
    connect( item, SIGNAL(activated(int)), categories, SLOT(raiseWidget(int)) );
}

PreferenceItem::PreferenceItem( QListView* view, const char* label ) :
    QListViewItem( view, label, 0 ),
    i(next_id++)
{
    setExpandable(TRUE);
}

PreferenceItem::PreferenceItem( QListViewItem* group, const char* label ) :
    QListViewItem( group, label, 0 ),
    i(next_id++)
{
    setExpandable(FALSE);
}

void PreferenceItem::activate()
{
    emit activated(i);
}

int PreferenceItem::next_id = 0;


main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Preferences m;
    m.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    return app.exec();
}
