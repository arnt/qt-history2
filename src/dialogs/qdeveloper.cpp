/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qdeveloper.cpp#8 $
**
** Implementation of QDeveloper class
**
** Created : 980830
**
** Copyright (C)1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include <stdlib.h>
#include "qapplication.h"
#include "qdeveloper.h"
#include "qhbox.h"
#include "qlabel.h"
#include "qlist.h"
#include "qlistview.h"
#include "qmenubar.h"
#include "qmetaobject.h"
#include "qobjectdict.h"
#include "qobjectlist.h"
#include "qpushbutton.h"
#include "qsplitter.h"
#include "qstatusbar.h"
#include "qvbox.h"
#include "qwidgetstack.h"


class QDeveloperClassItem : public QListViewItem {
    QDeveloperPrivate* d;
    QMetaObject* meta;
public:
    QDeveloperClassItem( QListView * parent, QMetaObject* mo, QDeveloperPrivate* pd ) :
	QListViewItem( parent, mo->className() )
    {
	d = pd;
	meta = mo;
    }

    QDeveloperClassItem( QListViewItem * parent, QMetaObject* mo, QDeveloperPrivate* pd ) :
	QListViewItem( parent, mo->className() )
    {
	d = pd;
	meta = mo;
    }

    QMetaObject* at() { return meta; }

    void setup()
    {
	QListViewItem::setup();
	if ( childCount() )
	    return;
	QDictIterator<QMetaObject> it(*objectDict);
	QMetaObject* child;
	while ((child = it.current())) {
	    ++it;
	    if ( child->superClass() == meta ) {
		// Subclass of this
		(void)new QDeveloperClassItem(this,child,d);
	    }
	}
    }
};

class QDeveloperTranslationScope;

class QDeveloperTranslationKey : public QListViewItem {
public:
    QDeveloperTranslationKey( QDeveloperTranslationScope* parent, const char* key );
    void setup()
    {
	QListViewItem::setup();
	int lines = 1+text(0).contains('\n');
	setHeight( listView()->fontMetrics().height()*lines );
    }
};

class QDeveloperTranslationScope : public QListViewItem {
public:
    QDeveloperTranslationScope( QListView* parent, const char* scope ) :
	QListViewItem(parent, scope)
    {
    }
   ~QDeveloperTranslationScope() {}
    QDict<QDeveloperTranslationKey> keys;
};

QDeveloperTranslationKey::QDeveloperTranslationKey( QDeveloperTranslationScope* parent,
						    const char* key ) :
	QListViewItem(parent, key)
    {
    }


class QDeveloperPrivate {
public:
    QDeveloperPrivate()
    {
    }

    void init(QWidget* parent, QDeveloper* trtarget)
    {
	makeTranslator(parent,trtarget);
	makeExplorer(parent);
    }

    void makeTranslator(QWidget* parent, QDeveloper* trtarget)
    {
	QObject::connect(qApp,SIGNAL(unknownTranslation(const char*, const char*)),
	    trtarget, SLOT(recordUnknownTranslation(const char*, const char*)));

	// This one first - because "Scope" probably hasn't been translated...
	translations = new QListView(parent);
	translations->setFrameStyle(QFrame::Sunken|QFrame::Panel);
	translations->addColumn(QDeveloper::tr("Scope/Key"));

	// We'll need to add other stuff around it later.
	translator = translations;

        // ##### This at least.  But would it make sense to be able to
        // ##### look at some OTHER translation while trying to
        // ##### translate this one?  ie. use an auxiliary QTranslator
        // ##### for another column.
	translations->addColumn(getenv("LANG"));
    }

    void makeExplorer(QWidget* parent)
    {
	explorer = new QVBox(parent);
	QSplitter* splitter = new QSplitter(explorer);
	//QHBox* hbox = new QHBox(explorer);
	//(void)new QLabel(QDeveloper::tr("Class\nstuff\nhere"),hbox);
	//(void)new QLabel(QDeveloper::tr("Object\nstuff\nhere"),hbox);

	classes = new QListView(splitter);
	classes->setFrameStyle(QFrame::Sunken|QFrame::Panel);
	classes->addColumn(QDeveloper::tr("Class"));

	objects = new QListView(splitter);
	objects->setFrameStyle(QFrame::Sunken|QFrame::Panel);
	objects->addColumn(QDeveloper::tr("Object"));
	objects->addColumn(QDeveloper::tr("Class"));
	objects->addColumn(QDeveloper::tr("Address"));
    }

    QDeveloperObjectItem* findTopLevel( QObject* o )
    {
	QListViewItem* cursor = objects->firstChild();
	while ( cursor ) {
	    QDeveloperObjectItem* boi = (QDeveloperObjectItem*)cursor;
	    if ( boi->at() == o ) {
		return boi;
	    }
	    cursor = cursor->nextSibling();
	}
	return 0;
    }

    void removeTopLevel( QObject* o )
    {
	delete findTopLevel(o);
    }

    void selectClass( QMetaObject* mo )
    {
	QDeveloperClassItem* bci = (QDeveloperClassItem*)selectClassFrom(mo);
	classes->setSelected(bci,TRUE);
	classes->ensureItemVisible(bci);
    }

    QListViewItem* selectClassFrom( QMetaObject* mo )
    {
	if ( mo->superClass() ) {
	    QListViewItem* cursor = selectClassFrom( mo->superClass() );
	    cursor = cursor->firstChild();
	    while ( cursor ) {
		QDeveloperClassItem* bci = (QDeveloperClassItem*)cursor;
		if ( bci->at() == mo ) {
		    bci->setOpen(TRUE);
		    return bci;
		}
		cursor = cursor->nextSibling();
	    }
	    fatal(QDeveloper::tr("Huh?"));
	    return 0;
	} else {
	    // QObject
	    QListViewItem* bci = classes->firstChild();
	    bci->setOpen(TRUE);
	    return bci;
	}
    }

    QWidget* explorer;
    QWidget* translator;
    QListView* classes;
    QListView* objects;
    QListView* translations;
    QDict<QDeveloperTranslationScope> translationScopes;
};

static
QString adrtext(void* a)
{
    QString s;
    s.sprintf("0x%p", a);
    return s;
}

static
bool find( QObject* o, QListViewItem* cursor )
{
    while ( cursor ) {
	QDeveloperObjectItem* boi = (QDeveloperObjectItem*)cursor;
	if ( boi->at() == o ) {
	    return TRUE;
	}
	cursor = cursor->nextSibling();
    }
    return FALSE;
}

QDeveloperObjectItem::QDeveloperObjectItem( QListView * parent, QObject* o, QDeveloperPrivate *pd ) :
    QListViewItem( parent, o->name(), o->className(), adrtext(o) )
{
    d = pd;
    object = o;
    object->installEventFilter(this);
    connect(object, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
}

QDeveloperObjectItem::QDeveloperObjectItem( QListViewItem * parent, QObject* o, QDeveloperPrivate *pd ) :
    QListViewItem( parent, o->name(), o->className(), adrtext(o) )
{
    d = pd;
    object = o;
    object->installEventFilter(this);
    connect(object, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
}

bool QDeveloperObjectItem::eventFilter(QObject* o, QEvent* e)
{
    if ( o == object ) {
	QChildEvent* ce = (QChildEvent*)e;
	switch ( e->type() ) {
	  case QEvent::ChildInserted: {
		d->removeTopLevel( ce->child() );
		if ( !find(ce->child(), firstChild()) ) {
		    QDeveloperObjectItem* lvi = new QDeveloperObjectItem( this, ce->child(), d );
		    lvi->fillExistingTree();
		}
		setExpandable(TRUE);
	    }
	    break;
	  case QEvent::ChildRemoved: {
		QListViewItem *cursor = firstChild();
		while ( cursor ) {
		    QDeveloperObjectItem* boi = (QDeveloperObjectItem*)cursor;
		    if ( boi->object == ce->child() ) {
			delete cursor;
			cursor = 0;
		    } else {
			cursor = cursor->nextSibling();
		    }
		}
	    }
	    break;
	  default:
	    break;
	}
    }
    return FALSE;
}

void QDeveloperObjectItem::setup()
{
    QListViewItem::setup();
}

void QDeveloperObjectItem::fillExistingTree()
{
    QObjectList *list = (QObjectList*)object->children();
    if ( !list ) return;
    QObjectListIt it(*list);
    QObject* child;
    while ((child = it.current())) {
	++it;
	// Child object of this
	if ( !find(child,firstChild()) ) {
	    QDeveloperObjectItem* lvi = new QDeveloperObjectItem(this,child,d);
	    lvi->fillExistingTree();
	}
    }
    setExpandable(TRUE);
}

void QDeveloperObjectItem::objectDestroyed()
{
    // Me too.
    delete this;
}

QDeveloper::QDeveloper() :
    QMainWindow(0,0,WDestructiveClose)
{
    QWidgetStack* stack = new QWidgetStack(this);
    d = new QDeveloperPrivate;
    d->init(stack, this);

    setCentralWidget(stack);

    int id;

    QPopupMenu * file = new QPopupMenu();
    menuBar()->insertItem( tr("&File"), file );

    QPopupMenu * edit = new QPopupMenu();
    id=menuBar()->insertItem( tr("&Edit"), edit );
    menuBar()->setItemEnabled(id,FALSE);

    // The QDeveloper is modal because by restricting to one window,
    // it doesn't get mixed with application widgets, nor take up too
    // much space.
    QPopupMenu * mode = new QPopupMenu();
    menuBar()->insertItem( tr("&Mode"), mode );

    stack->addWidget(d->explorer, mode->insertItem(tr("&Explorer")));
    stack->addWidget(d->translator, mode->insertItem(tr("&Translator")));
    connect(mode,SIGNAL(activated(int)),stack,SLOT(raiseWidget(int)));

    QString msg;
    int nclasses = QMetaObjectInit::init()+1; // +1 for QObject
    if ( nclasses ) {
	msg.sprintf(tr("Qt Developer - %d classes"), nclasses);
	QListViewItem *lvi = new QDeveloperClassItem( d->classes, QObject::metaObject(), d );
	lvi->setOpen(TRUE);
    } else {
	msg = tr("Sorry, your compiler/platform is insufficient for "
		"the Qt Developer to operate");
    }
    statusBar()->message(msg);

    connect( d->objects, SIGNAL(selectionChanged(QListViewItem*)),
	     this, SLOT(selectObject(QListViewItem*)) );

    connect( d->classes, SIGNAL(selectionChanged(QListViewItem*)),
	     this, SLOT(selectClass(QListViewItem*)) );

    updateDetails(0,0);
}

QDeveloper::~QDeveloper()
{
}

void QDeveloper::addTopLevelWidget(QWidget* tlw)
{
    if ( !d->findTopLevel(tlw) ) {
	QDeveloperObjectItem *lvi = new QDeveloperObjectItem( d->objects, tlw, d );
	lvi->fillExistingTree();
	lvi->setOpen(TRUE);es
    }
}

void QDeveloper::selectObject( QListViewItem* lvi )
{
    if ( lvi->isSelected() ) {
	QDeveloperObjectItem* boi = (QDeveloperObjectItem*)lvi;
	d->selectClass(boi->at()->metaObject());
	updateDetails(boi->at());
    }
}

void QDeveloper::selectClass( QListViewItem* lvi )
{
    if ( lvi->isSelected() ) {
	QDeveloperClassItem* bci = (QDeveloperClassItem*)lvi;
	d->selectClass(bci->at());

	QDeveloperObjectItem* boi = (QDeveloperObjectItem*)
				    d->objects->currentItem();
	if ( boi && bci->at() != boi->at()->metaObject() ) {
	    d->objects->setSelected(boi,FALSE);
	    updateDetails(0,bci->at());
	}
    }
}

void QDeveloper::updateDetails( QObject* object, QMetaObject* cls )
{
    if ( object ) {
	cls = object->metaObject();
	// Show detailed information
    } else {
	// Disable display
    }

    if ( cls ) {
	// Show detailed information
    } else {
	// Disable display
    }
}

void QDeveloper::recordUnknownTranslation( const char *scope, const char *key )
{
    QDeveloperTranslationScope *dts = d->translationScopes.find(scope);
    if ( !dts ) {
	dts = new QDeveloperTranslationScope( d->translations, scope );
	d->translationScopes.insert(scope,dts);
    }
    QDeveloperTranslationKey *dtk = dts->keys.find(key);
    if ( !dtk ) {
	dtk = new QDeveloperTranslationKey( dts, key );
	dts->keys.insert(key,dtk);
    }
}
