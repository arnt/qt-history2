#include <qapplication.h>
#include <stdlib.h>

#include "main.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qmessagebox.h>

class ChoiceItem : public QCheckListItem {
public:
    QString id;
    ChoiceItem(const QString& i, QListViewItem* parent) :
	QCheckListItem(parent,
	    i.mid(6), // strip "QT_NO_" as we reverse the logic
	    CheckBox),
	id(i)
    {
	setOpen(TRUE);
    }

    // We reverse the logic
    void setDefined(bool y) { setOn(!y); }
    bool isDefined() const { return !isOn(); }

    virtual void setOn(bool y)
    {
	QCheckListItem::setOn(y);
	setOpen(y);
	for (QListViewItem* i=firstChild(); i; i = i->nextSibling() ) {
	    ChoiceItem* ci = (ChoiceItem*)i; // all are ChoiceItem
	    if ( ci->isSelectable() != y ) {
		ci->setSelectable(y);
		listView()->repaintItem(ci);
	    }
	}
    }

    void paintBranches( QPainter * p, const QColorGroup & cg,
                            int w, int y, int h, GUIStyle s)
    {
	QListViewItem::paintBranches(p,cg,w,y,h,s);
    }

    void paintCell( QPainter * p, const QColorGroup & cg,
                               int column, int width, int align )
    {
	if ( !isSelectable() ) {
	    QColorGroup c = cg;
	    c.setColor(QColorGroup::Text, gray);
	    QCheckListItem::paintCell(p,c,column,width,align);
	} else {
	    QCheckListItem::paintCell(p,cg,column,width,align);
	}
    }

    void setInfo(const QString& d)
    {
	doc = d;
    }

    QString info() const
    {
	return "<h1>"+id+"</h1>"+doc;
    }
private:
    QString doc;
};

Main::Main()
{
    QHBox* hbox = new QHBox(this);

    lv = new QListView(hbox);
    lv->setSorting(-1);
    lv->setRootIsDecorated(TRUE);
    lv->addColumn("ID");

    info = new QLabel(hbox);

    connect(lv,SIGNAL(selectionChanged(QListViewItem*)),
	  this,SLOT(showInfo(QListViewItem*)));

    setCentralWidget(hbox);
}

// ##### should be in QMap?
template <class K, class D>
QValueList<K> keys(QMap<K,D> map)
{
    QValueList<K> result;
    for (QMap<K,D>::ConstIterator it = map.begin(); it!=map.end(); ++it)
	result.append(it.key());
    return result;
}

void Main::loadFeatures(const QString& filename)
{
    QFile file(filename);
    file.open(IO_ReadOnly);
    QTextStream s(&file);
    QRegExp qt_no_xxx("QT_NO_[A-Z_0-9]*");
    QStringList deps;
    QString sec;
    QString doc;
    bool on = FALSE;
    bool docmode = FALSE;
    QMap<QString,QString> documentation;
    QStringList sections;

    do {
	QString line = s.readLine();

	QStringList token = QStringList::split(QChar(' '),line);
	if ( on ) {
	    if ( docmode ) {
		if ( token[0] == "*/" )
		    docmode = FALSE;
		else
		    doc += line;
	    } else if ( token[0] == "//#define" || token[0] == "#define" ) {
		dependencies[token[1]] = deps;
		section[token[1]] = sec;
		documentation[token[1]] = doc;
		choices.append(token[1]);
		doc = "";
	    } else if ( token[0] == "/*!" ) {
		docmode = TRUE;
	    } else if ( token[0] == "//" ) {
		token.remove(token.begin());
		sec = token.join(" ");
		sections.append(sec);
	    } else if ( token[0] == "#if" ) {
		ASSERT(deps.isEmpty());
		for (int i=1; i<(int)token.count(); i++) {
		    if ( token[i][0] == 'd' ) {
			int index;
			int len;
			index = qt_no_xxx.match(token[i],0,&len);
			if ( index >= 0 ) {
			    deps.append(token[i].mid(index,len));
			}
		    }
		}
	    } else if ( token[0] == "#endif" ) {
		deps.clear();
	    } else if ( token[0].isEmpty() ) {
	    } else {
		qDebug("Cannot parse: %s",token.join(" ").ascii());
	    }
	} else if ( token[0] == "#include" ) {
	    on = TRUE;
	}
    } while (!s.atEnd());

    lv->clear();
    sections.sort();
    // ##### QListView default sort order is reverse of insertion order
    for (QStringList::Iterator se = sections.fromLast(); se != sections.end(); --se) {
	sectionitem[*se] = new QListViewItem(lv,*se);
    }  
    for (QStringList::Iterator ch = choices.begin(); ch != choices.end(); ++ch) {
	QStringList deps = dependencies[*ch];
	QListViewItem* parent = deps.isEmpty() ? 0 : item[deps[0]];
	if ( !parent ) {
	    parent = sectionitem[section[*ch]];
	}
	ChoiceItem* ci = new ChoiceItem(*ch,parent);
	item[*ch] = ci;
	ci->setInfo(documentation[*ch]);
    }
}

void Main::loadConfig(const QString& filename)
{
    QFile file(filename);
    file.open(IO_ReadOnly);
    QTextStream s(&file);
    QRegExp qt_no_xxx("QT_NO_[A-Z_0-9]*");

    for (QStringList::Iterator ch = choices.begin(); ch != choices.end(); ++ch) {
	item[*ch]->setDefined(FALSE);
    }
    do {
	QString line = s.readLine();
	QStringList token = QStringList::split(QChar(' '),line);
	if ( token[0] == "#define" ) {
	    ChoiceItem* i = item[token[1]];
	    if ( i )
		i->setDefined(TRUE);
	    else {
		QMessageBox::warning(this,"Warning",
		    "The item " + token[1] + " is not used by qfeatures.h");
	    }
	}
    } while (!s.atEnd());
}

void Main::showInfo(QListViewItem* i)
{
    if ( !i->parent() ) {
	// section. do nothing for now
    } else {
	ChoiceItem* choice = (ChoiceItem*)i;
	info->setText(choice->info());
    }
}


main(int argc, char** argv)
{
    QApplication app(argc,argv);
    Main m;
    QString qtdir = getenv("QTDIR");
    QString qfeatures = qtdir + "/include/qfeatures.h";
    QString qconfig = qtdir + "/include/qconfig.h";
    for (int i=0; i<argc; i++) {
	QString arg = argv[i];
	if ( arg == "-f" && i+i<argc ) {
	    qfeatures = argv[++i];
	} else {
	    qconfig = argv[i];
	}
    }
    m.loadFeatures(qfeatures);
    m.loadConfig(qconfig);
    app.setMainWidget(&m);
    m.show();
    return app.exec();
}
