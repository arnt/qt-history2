#include "prefs.h"
#include <qpainter.h>
#include <qapp.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbt.h>
#include <qbuttonrow.h>
#include <qlayout.h>

class DummyCategory : public QLabel {
public:
    DummyCategory() :
	QLabel("Dummy")
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
    selector->setColumn( "Bug", 30 ); // ### bug in QListView
    selector->setMaximumWidth( w /*BUG*/+30 );

    PreferenceItem *group;
    add(group = new PreferenceItem(selector, "Appearance"), new DummyCategory);
     add(new PreferenceItem(group, "Font"), new DummyCategory);
     add(new PreferenceItem(group, "Colors"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Navigator"), new DummyCategory);
     add(new PreferenceItem(group, "Languages"), new DummyCategory);
     add(new PreferenceItem(group, "Applications"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Mail & Groups"), new DummyCategory);
     add(new PreferenceItem(group, "Identity"), new DummyCategory);
     add(new PreferenceItem(group, "Messages"), new DummyCategory);
     add(new PreferenceItem(group, "Mail Server"), new DummyCategory);
     add(new PreferenceItem(group, "Groups Server"), new DummyCategory);
     add(new PreferenceItem(group, "Directory"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Composer"), new DummyCategory);
     add(new PreferenceItem(group, "New Page Colors"), new DummyCategory);
     add(new PreferenceItem(group, "Publish"), new DummyCategory);
    add(group = new PreferenceItem(selector, "Advanced"), new DummyCategory);
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
