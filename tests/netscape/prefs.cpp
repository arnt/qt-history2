#include "prefs.h"
#include <qpainter.h>
#include <qapp.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qcombo.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qgrid.h>
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
    new StrongHeading( "Advanced", 
		       "Change preferences that affect the entire product", 
		       vbox );

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
/*
class DummyCategory : public QLabel {
public:
    DummyCategory( const char * s = 0 ) :
	QLabel(s?s:"Dummy")
    {
	setMinimumSize(100,100);
    }
};
*/


static QWidget *fontPage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Fonts", "Change the fonts in your display" , page );
    QLabelled *frame = new QLabelled( "Fonts and Encodings", page );
    QVBox *box = new QVBox( frame );
    QHBox *hbox = new QHBox( box );
    new QLabel( "For the Encoding", hbox );
    QComboBox *combo = new QComboBox( FALSE, hbox );
    combo->insertItem( "Western (iso-8859-1)" );

    hbox = new QHBox( box );
    new QLabel( "Variable Width Font", hbox );
    combo = new QComboBox( FALSE, hbox );
    combo->insertItem( "Times (Adobe)" );
    new QLabel( "Size:", hbox );
    combo = new QComboBox( FALSE, hbox );
    combo->insertItem( "12.0" );


    hbox = new QHBox( box );
    new QLabel( "Fixed Width Font", hbox );
    combo = new QComboBox( FALSE, hbox );
    combo->insertItem( "Courier (Adobe)" );
    new QLabel( "Size:", hbox );
    combo = new QComboBox( FALSE, hbox );
    combo->insertItem( "10.0" );

    frame = new QLabelled( page );
    box = new QVBox( frame );
    new QLabel( "Sometimes a document will provide its own fonts." );
    new QRadioButton( "Use my default fonts, overriding document-specified fonts", box );
    new QRadioButton( "Use document-specified fonts, but disable Dynamic Fonts", box );
    new QRadioButton( "Use document-specified fonts, including Dynamic Fonts", box );

    page->addStretch();
    return page;
}


static QWidget *colorPage()
{
    QVBox *page = new QVBox;

     new StrongHeading( "Colors", "Change the colors in your display" , page );

    QHBox *topbox = new QHBox( page );

    QLabelled *frame = new QLabelled( "Colors", topbox );
    QGrid *grid = new QGrid( 3, frame );
    new QLabel( "Text:", grid );
    new QPushButton( "dummy", grid );
    new QWidget(grid); //### grid->skip();
    new QLabel( "Background:", grid );
    new QPushButton( "dummy", grid );
    new QWidget(grid); //### grid->skip();
    new QWidget(grid); //### grid->skip();
    new QWidget(grid); //### grid->skip();
    new QPushButton( "Use Default", grid );

    frame = new QLabelled( "Links", topbox );
    QVBox *box = new QVBox( frame );
    grid = new QGrid( 2, box );
    new QLabel( "Unvisited Links:", grid );
    new QPushButton( "dummy", grid );
    new QLabel( "Visited Links:", grid );
    new QPushButton( "dummy", grid );
    new QCheckBox( "Underline links", box );


    frame = new QLabelled( page );
    box = new QVBox( frame );

    new QLabel( "Sometimes a document will provide its own colors and background", box );
    new QCheckBox( "Always use my colors, overriding document", box );    

    page->addStretch();
    return page;
}

static QWidget *appsPage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Applications",   
		       "Specify helper applications for different file types",
		       page );

    QLabelled *frame = new QLabelled( page );
    QVBox *box = new QVBox( frame );
    QListView *lv = new QListView( box );
    lv->addColumn( "Description", 250 );
    lv->addColumn( "Handled By", 200 );
    new QListViewItem( lv, "GIF Image", "Netscape" );
    new QListViewItem( lv, "Hypertext Markup Language", "Netscape" );
    new QListViewItem( lv, "Plain text", "Netscape" );
    new QListViewItem( lv, "Perl Program", "Unknown:Prompt User" );
    new QListViewItem( lv, "Lkjsfdlkdsjgf", "Unknown:Prompt User" );
    new QListViewItem( lv, "Asdfafsafs", "Unknown:Prompt User" );
    new QListViewItem( lv, "Xzcmnv", "Unknown:Prompt User" );
    new QListViewItem( lv, "Aqwreqrpoierw", "Unknown:Prompt User" );
    new QListViewItem( lv, "mnzxcewewq", "Unknown:Prompt User" );
    new QListViewItem( lv, "Oiuycxsxc Xocuy", "Unknown:Prompt User" );
    new QListViewItem( lv, "JPG Image", "Netscape" );
    new QListViewItem( lv, "XBM Image", "Netscape" );

    QButtonRow *br = new QButtonRow( box );
    new QPushButton( "New...", br );
    new QPushButton( "Edit...", br );
    new QPushButton( "Delete", br );

    QHBox *hbox = new QHBox( box );
    new QLabel( "Download files to:", hbox );
    new QLineEdit( hbox );
    new QPushButton( "Choose...", hbox );

    page->addStretch();
    return page;
}


static QWidget *appearPage()
{
    QVBox *page = new QVBox;
    
    new StrongHeading( "Appearance",
		       "Change the appearance of the display" , page );
    QLabelled *frame = new QLabelled( "On startup, launch", page );
    QVBox *box = new QVBox( frame );
    new QCheckBox( "&Navigator", box );
    new QCheckBox( "&Messenger Mailbox", box );
    new QCheckBox( "Collabra &Discussions", box );
    new QCheckBox( "Page &Composer", box );
    new QCheckBox( "N&etcaster", box );
    frame = new QLabelled( "Show Toolbar As", page );
    box = new QVBox( frame );
    new QRadioButton( "&Pictures and Text", box );
    new QRadioButton( "Pictures &Only", box );
    new QRadioButton( "&Text Only", box );
    
    page->addStretch();
    return page;
}

static QWidget *navigatorPage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Navigator",   "Specify the home page location",
			    page );
    QLabelled *frame = new QLabelled( "Browser starts with", page );
    QVBox *box = new QVBox( frame );
    new QRadioButton( "Blank page", box );
    new QRadioButton( "Home page", box );
    new QRadioButton( "Last page visited", box );

    frame = new QLabelled( "Home Page", page );
    box = new QVBox( frame );
    new QLabel( "Clicking the Home button will take you to this page",
		box );
    QHBox *hbox = new QHBox( box );
    new QLabel( "Location:", hbox );
    new QLineEdit( hbox );
    hbox = new QHBox( box );
    new QPushButton( "Use Current Page" );
    new QPushButton ( "Choose" );

    frame = new QLabelled( "History", page );
    hbox = new QHBox( frame );
    new QLabel( "History expires after", hbox );
    new QLineEdit( hbox );
    new QLabel( "days", hbox );
    new QPushButton( "Clear History", hbox );

    page->addStretch();
    return page;
}

static QWidget *cachePage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Cache",   "Designate the size of the cache",
			    page );
    QLabelled *frame = new QLabelled( 0, page );
    QVBox *box = new QVBox( frame );
    
    new QLabel( "The cache is used to keep local copies of frequently accessed docu-\n\
ments and thus reduce time connected to the network. The Reload\n\
button will always compare the cache document to the network\n\
document and show the most recent one. To load pages and images\n\
from the network instead of the cache, press the Shift key and click\n\
the reload button.", box );

    QHBox *hbox = new QHBox( box );
    new QLabel( "Disk cache:", hbox );
    new QLineEdit( hbox );
    new QPushButton( "Clear Disk Cache", hbox );

    hbox = new QHBox( box );
    new QLabel( "Memory cache:", hbox );
    new QLineEdit( hbox );
    new QPushButton( "Clear Memory Cache", hbox );

    hbox = new QHBox( box );
    new QLabel( "Cache Folder:", hbox );
    new QLineEdit( hbox );
    new QPushButton( "Choose...", hbox );

    new QLabel( "Document in cache is compared to document on network", box );

    //    QVBox *box = new QVBox( frame );
    new QRadioButton( "Every time", box );
    new QRadioButton( "Once per session", box );
    new QRadioButton( "Never", box );

    page->addStretch();
    return page;
}

static QWidget *proxyPage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Proxies",   "Configure proxies to access the Internet",
			    page );
    QLabelled *frame = new QLabelled( 0, page );
    QVBox *box = new QVBox( frame );
    
    new QLabel( "A network proxy is used to provide additional security between your\n\
computer and the Internet (usually along with a firewall) and/or to\n\
increase performance between networks by reducing redundant traffic\n\
via caching. Your system administrator can provide you with proper\n\
proxy settings.", box );


    new QRadioButton( "Direct connection to the internet", box );

    QHBox *hbox = new QHBox( box );
    new QRadioButton( "Manual proxy configuration", hbox );
    new QPushButton( "View...", hbox );

    new QRadioButton( "Automatic proxy configuration", box );

    hbox = new QHBox( box );
    new QLabel( "Configuration location (URL)", hbox );
    new QLineEdit( hbox );
    new QPushButton( "&Reload", box );

    page->addStretch();
    return page;
}

static QWidget *identityPage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Identity",   
		       "Set your name, email address, and signature file",
		       page );
    QLabelled *frame = new QLabelled( 0, page );
    QVBox *box = new QVBox( frame );
    
    new QLabel( "The information below is needed before you can send mail. If you do\n\
not know the information requested, please contact your system\n\
administrator or Internet Service Provider.", box );

    new QLabel( "Your name:", box );
    new QLineEdit(box);

    new QLabel( "Email Address:", box );
    new QLineEdit(box);

    new QLabel( "Organization:", box );
    new QLineEdit(box);

    page->addStretch();
    return page;
}

static QWidget *languagePage()
{
    QVBox *page = new QVBox;

    new StrongHeading( "Languages",   
		       "View web pages in different languages",
		       page );
    QLabelled *frame = new QLabelled( 0, page );
    QVBox *box = new QVBox( frame );
    
    new QLabel( "Choose in order of preference the language(s) in which you prefer to\n\
view web pages. Web pages are sometimes available in serveral\n\
languages. Navigator presents the pages in the available language\n\
you most prefer.", box );

    return page;
}


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
    selector->addColumn( "Category", w );
    selector->setMaximumWidth( w );
    selector->setRootIsDecorated( TRUE );
    selector->setFocusPolicy( QWidget::StrongFocus );

    PreferenceItem *group;

    add(group = new PreferenceItem(selector, "Appearance"), 
	appearPage() );
    add(new PreferenceItem(group, "Font"), fontPage() );
    add(new PreferenceItem(group, "Colors"), colorPage() );
    add(group = new PreferenceItem(selector, "Navigator"), navigatorPage() );
    add(new PreferenceItem(group, "Languages"), languagePage() );
    add(new PreferenceItem(group, "Applications"), appsPage() );
    add(new PreferenceItem(selector, "Identity"), identityPage() );

    add(group = new PreferenceItem(selector, "Advanced"), advanced());
    add(new PreferenceItem(group, "Cache"), cachePage() );
    add(new PreferenceItem(group, "Proxies"), proxyPage() );
    //    add(new PreferenceItem(group, "Disk Space"), new DummyCategory);

//     add(group = new PreferenceItem(selector, "Mail & Groups"), new DummyCategory);
//     add(new PreferenceItem(group, "Messages"), new DummyCategory);
//     add(new PreferenceItem(group, "Mail Server"), new DummyCategory);
//     add(new PreferenceItem(group, "Groups Server"), new DummyCategory);
//     add(new PreferenceItem(group, "Directory"), new DummyCategory);
//     add(group = new PreferenceItem(selector, "Composer"), new DummyCategory);
//     add(new PreferenceItem(group, "New Page Colors"), new DummyCategory);
//     add(new PreferenceItem(group, "Publish"), new DummyCategory);



    setCaption("QtScape: Preferences");
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
