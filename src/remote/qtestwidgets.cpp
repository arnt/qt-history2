#include "qtestwidgets_p.h"

#include <qstring.h>
#include "qapplication.h"
#include <qobjectlist.h>
#include <private/qremotemessage_p.h>
#include <assert.h>

class QKeyString : public QString
{
public:
    QKeyString();
    QKeyString( const QString &s );

    void addKey( const QString &key, const QString &value );
    bool findKey( const QString &key, QString &keyValue, bool resetIfNotFound = TRUE );

    void addKey( const QString &key, int value );
    bool findKey( const QString &key, int &value, bool resetIfNotFound = TRUE );

    QString removeKey( const QString &key );
};

QKeyString::QKeyString()
{
}

QKeyString::QKeyString( const QString &s ) : QString(s)
{
}

void QKeyString::addKey( const QString &key, const QString &value )
{
    QString S;
    S = "<" + key + ">" + value + "</" + key + ">";
    append( S );
}

void QKeyString::addKey( const QString &key, int value )
{
    QString tmp;
    tmp.sprintf("%d",value);
    addKey(key,tmp);
}

bool QKeyString::findKey( const QString &key, QString &keyValue, bool resetIfNotFound )
{
    int pos1, pos2;
    pos1 = find("<"+key+">");
    if (pos1>=0) {
	pos2 = find("</"+key+">",pos1);
	if (pos2 > pos1) {
	    pos1+= key.length()+2;
	    keyValue = mid(pos1,pos2-pos1);
	    return TRUE;
	}
    }

    if (resetIfNotFound)
	keyValue = "";

    return FALSE;
}

bool QKeyString::findKey( const QString &key, int &value, bool resetIfNotFound )
{
    QString tmp;
    if (findKey(key,tmp,FALSE)) {
	bool ok;
	value = tmp.toInt(&ok);
	if (!ok && resetIfNotFound)
	    value = 0;
	return ok;
    } else {
	if (resetIfNotFound)
	    value = 0;
	return FALSE;
    }
}


QString QKeyString::removeKey( const QString &key )
{
    QString retValue;
    int pos1, pos2;
    pos1 = find("<"+key+">");
    if (pos1 >= 0) {
	pos2 = find("</"+key+">",pos1);
	if (pos2 > pos1) {
	    int len = pos2 - pos1 + key.length()+3;
	    retValue = remove(pos1,len);
	    return retValue;
	}
    }

    return left(length());
}

//***********************************************************

QTestWidgets::QWidgetRec::QWidgetRec( QString signature, int id )
{
    _signature = signature;
    _id = id;
    _ptr = 0;
}

QTestWidgets::QWidgetRec::~QWidgetRec()
{
}


QCString QTestWidgets::QWidgetRec::name()
{
    QKeyString S = _signature;
    QString ret;
    S.findKey("NAME", ret, TRUE );
    return ret;
}

//***********************************************************

#define WIDGETS_INCR 1024

QTestWidgets::QTestWidgets()
{
    next_widget_index = 0;
    nameDelimiterChar = '#';
    socket = 0;

//    widgetName2index_.clear();
    widgets_size = WIDGETS_INCR;

    widgetsBySignature.setAutoDelete( FALSE );
    widgetsBySignature.resize( widgets_size );

    widgetsById.setAutoDelete( TRUE );
    widgetsById.resize( widgets_size );

    widgetsByPtr.setAutoDelete( FALSE );
    widgetsByPtr.resize( widgets_size );
}

QTestWidgets::~QTestWidgets()
{
    clear();
}

void QTestWidgets::setSocket( QSocket *s )
{
    socket = s;
}

void QTestWidgets::clear()
{
    // clear all dictionaries
    widgetsBySignature.clear();
    widgetsById.clear();
    widgetsByPtr.clear();
    next_widget_index = 0;
}

bool QTestWidgets::getWidgetRec( int id, QWidgetRec *&rec )
{
    rec = widgetsById.find( id );
    return rec != 0;
}

bool QTestWidgets::getWidgetRec( QCString signature, QWidgetRec *&rec )
{
    rec = widgetsBySignature.find( signature );
    return rec != 0;
}

bool QTestWidgets::getWidgetRec( QObject *o, QWidgetRec *&rec )
{
    rec = widgetsByPtr.find( o );
    return rec != 0;
}

bool QTestWidgets::appendWidget( const QCString &signature, int widget_id )
{
    QWidgetRec *rec;
    if (getWidgetRec( widget_id, rec )) {
	QRemoteMessage S("WRN","Widget id is already defined!");
	S.send( socket );
	return FALSE;
    }

    if (getWidgetRec( signature, rec )) {
	QRemoteMessage S("WRN","Widget signature is already defined!");
	S.send( socket );
        return FALSE;
    }
	
    // resize the dictionary if they are filled
    if (widgetsById.count()+1 == widgets_size) {
        widgets_size+= WIDGETS_INCR;

        widgetsById.resize( widgets_size );
        widgetsBySignature.resize( widgets_size );
        widgetsByPtr.resize( widgets_size );
    }

    // insert the signature, id and ptr into the dictionaries 
    rec = new QWidgetRec( signature, widget_id );

    widgetsBySignature.insert( signature, rec );
    widgetsById.insert( widget_id, rec );

    if (widget_id >= next_widget_index)
	next_widget_index = widget_id + 1;
    return TRUE;
}

QString QTestWidgets::appendWidget( const QCString &signature )
{
    if (!appendWidget( signature, next_widget_index ))
        return "";

    QKeyString w( signature );
    w.addKey( "ID", next_widget_index );

    return w;
}

bool QTestWidgets::appendWidgetDef( const QString &def )
{
    QKeyString w(def);
    int widget_id;

    if (w.findKey("ID",widget_id,TRUE)) {
        w.removeKey("ID");

        QCString tmp = w;
        if (appendWidget( tmp, widget_id )) {

/*
    for (QTestScalingInfo* si = scalingInfo.first(); si; si = scalingInfo.next())
    {
        if ((si->regExp()) && 
	    (si->regExp()->isValid()) && 
	    (si->regExp()->search(widget_name, 0) >= 0))
        {
            widget_scale_dict_.insert(widget_id, si);
        }
    }
*/
            return TRUE;
        }
    }

    return FALSE;
}

uint QTestWidgets::count()
{
    return widgetsById.count();
}
 
//***************************************************************

QObject* QTestWidgets::findWidget( const QCString &name, QString &missing_name, bool allow_invisible_widgets )
{
    QObject *widget = findAppWidget( name, missing_name, allow_invisible_widgets );

    if (!widget)
	widget = findGlobalWidget(name);

    return widget;
}

QObject* QTestWidgets::findAppWidget(const QCString &name, QString &missing_name, bool allow_invisible_widgets)
{
    // casting a QWidgetList to a QObjectList is safe as long as QWidget inherits QObject
    QObjectList *search_list = (QObjectList*)(QApplication::topLevelWidgets());  
    QObject *widget = findWidgetByName(name, search_list, missing_name, allow_invisible_widgets);
    delete search_list;

    return widget;
}

QObject* QTestWidgets::findGlobalWidget( const QCString &name )
{
    QString missing_name;
    QObjectList *search_list = (QObjectList*)(QApplication::objectTrees());
    QObject *widget = findWidgetByName(name, search_list, missing_name, TRUE);

    return widget;
}

QObject* QTestWidgets::findWidgetByName( const QCString &name, const QObjectList* search_list, QString &missing_name, bool allow_invisible_widgets )
{
    bool last = FALSE;
    QObject* result = 0;
    if (search_list) {

	QCString base_name;
	int pos = name.find(nameDelimiterChar);
	if (pos > 0)
	    base_name = name.left(pos);
	else {
	    base_name = name;
	    last = TRUE;
	}

	QCString cur_name;
        QObjectListIt it (*search_list);
	QObject *o;
        while (it.current() && ! result) {

	    o = it.current();
            cur_name = o->name();
	    if ((base_name == cur_name) &&
		    (allow_invisible_widgets || 
		 !o->isWidgetType() || 
		 ((QWidget*)o)->isVisible())) {

		    QCString nextname = name.mid(pos+1);     // next name starts after delimiter
		    if (nextname != "" && !last) {            // if there is another name...
			result = findWidgetByName(nextname, o->children(), missing_name, allow_invisible_widgets);
		    } else {
			if (o->isWidgetType())
			    result = it.current();
		    }
            }
            ++it;
        }
        if ( ! result && missing_name.isNull())
            missing_name = base_name;
    }
    return result;
}

//***************************************************************************


// getFullName() creates a name that is a concatenation of all parents' names. 
// This is unique as long as you ensure that sibling widgets have unique names.
QCString QTestWidgets::getFullName( const QObject* item )
{
    if (item == 0)
	return "";

    QString name = item->name();
    if (name.isNull())
	name = "null";

    QCString retValue;

//    if (isTopLevelItem(item))
    if (item->parent() == 0)
	retValue = name;
    else
	retValue = getFullName(item->parent()) + nameDelimiterChar + name.latin1();

    if (!hasName(item)) {

	QRemoteMessage S("WRN","Unnamed instance '" + retValue + "' of class '" + item->className() + "'");
	S.send(socket);
    }
//qDebug("getFullName: " + retValue);

    return retValue;
}

bool QTestWidgets::hasName ( const QObject* item )
{
    if ( item == 0 )
	return FALSE;

    if ( !item->name() || strcmp (item->name(), "unnamed") == 0) {
	    return FALSE;
    }

    return TRUE;
}

QCString QTestWidgets::signature( const QObject *item )
{
    QKeyString retValue;
    retValue.addKey( "NAME", getFullName( item ) );
    retValue.addKey( "CLASS", item->className() );

    QObject* parent = item->parent();
    uint count = 0;
    QString pclass;
    if (parent) {

        pclass = parent->className();

        QObjectList *children = parent->queryList(0,item->name(),FALSE,FALSE);
	if (children) {
	    if (children->count() > 1) {

		QWidget *w;
		uint i;
		for (i=0; i < children->count(); i++) {
		    w = (QWidget*)children->at(i);
                    if (w == item) {
                        count = i;
                        break;
                    }
                }
            }
        }
    }
    
    retValue.addKey( "PARENT", pclass );
    retValue.addKey( "CHILDNO", count ); 

    return retValue;
}

// verifyUniqueName() returns the same name as getFullName() but also verifies that there are
// no siblings with exactly the same name.
/*
bool QTestWidgets::verifyUniqueName( const QObject* item, QCString &widgetName )
{
    bool retValue = TRUE;
    widgetName = getFullName(item);

    QObject* parent = item->parent();
    if (parent) {

	QObjectList *children = parent->queryList(0,item->name(),FALSE,FALSE);
	if (children) {
	    if (children->count() > 1) {

		// some workarounds to solve internal name problems...
		QString testName = item->name();
		if (testName == "qt_dockwidget_internal") {

		    bool bad_class = FALSE;
		    int popup_count = 0;
		    int extension_count = 0;
		    int resize_count = 0;
		    int title_count = 0;
		    int handle_count = 0;
		    int alias_count = 0;
		    QWidget *w;
		    uint i;
		    for (i=0; i < children->count(); i++) {
			w = (QWidget*)children->at(i);
			if (w->alias() != "") {
			    alias_count++;
			    continue;
			}
			QString tmp = w->className();
			if (tmp == "QDockWindowTitleBar") {
			    title_count++;
			    w->setAlias("qt_titlebar");
			} else if (tmp == "QDockWindowHandle") {
			    switch (handle_count)
			    {
				case 0: w->setAlias("qt_horz_handle"); break;
				case 1: w->setAlias("qt_vert_handle"); break;
			    }
			    handle_count++;

			} else if (tmp == "QDockWindowResizeHandle") {
			    switch (resize_count)
			    {
				case 0: w->setAlias("qt_resize_top"); break;
				case 1: w->setAlias("qt_resize_left"); break;
				case 2: w->setAlias("qt_resize_right"); break;
				case 3: w->setAlias("qt_resize_bottom"); break;
			    }
			    resize_count++;

			} else if (tmp == "QToolBarExtensionWidget") {
			    extension_count++;
			    w->setAlias("qt_extension_widget");
			} else if (tmp == "QPopupMenu") {
			    popup_count++;
			    w->setAlias("qt_popup_menu");
			} else {
			    bad_class = TRUE;
			}
		    }

		    if (!bad_class) {
			if (alias_count == 9 ||
			   (popup_count == 1 && 
			    extension_count == 1 && 
			    resize_count == 4 && 
			    handle_count == 2 &&
			    title_count == 1
			   ) )
			return TRUE;
		    }

		    // turn back the clock so we get a proper error message from the children
		    for (i=0; i < children->count(); i++) {
			w = (QWidget*)children->at(i);
			w->setAlias("");
		    }
		}


		// check if we have issued a warning for this objectname before.
		// if we have, exit without sending another warning.
		QStringList::Iterator it = multiple_instances.begin();
		while (it != multiple_instances.end()) {
		    if (strcmp(*it,widgetName) == 0) {
			retValue = FALSE;
			break;
		    }
		    it++;
		}

		if (retValue) {
		    // add the name to the list...
		    multiple_instances.append(widgetName);

		    // and send a warning/error...
		    QRemoteMessage S("ERR","Multiple instances named '" + widgetName + "' exist.");
		    S.send(socket);

		    for (uint i=0; i < children->count(); i++) {
			QString tmp = "  of class type '";
			tmp+= children->at(i)->className();
			assert(children->at(i)->parent() == parent);
			tmp+= "'.";
			QRemoteMessage S("ERR",tmp);
			S.send( socket );
		    }

		    retValue = FALSE;
		}
	    }
	    delete children;
	}
    }

    return retValue;
}
*/

bool QTestWidgets::widgetIsVisible( QObject* receiver )
{
    // i guess this body can be improved a lot, but for now it works.
    QCString widget_name = getFullName( receiver );

    widget_name += nameDelimiterChar;
    QString missing_name;
    QObject* event_widget = findAppWidget( widget_name, missing_name, FALSE );
    return (event_widget != 0);
}

bool QTestWidgets::getWidget( int id, QObject *&widget )
{
    QWidgetRec *rec = widgetsById.find( id );
    if (rec != 0) {
        widget = rec->_ptr;
        return widget != 0;
    }

    return FALSE;
}

bool QTestWidgets::getWidget( QCString signature, QObject *&widget )
{
    QWidgetRec *rec = widgetsBySignature.find( signature );
    if (rec != 0) {
        widget = rec->_ptr;
        return widget != 0;
    }

    return FALSE;
}

/*!
   \internal
    Browses recursive through the \a list and assigns all available widgets with an 
    unique ID that will be shared with the test server.
*/

void QTestWidgets::updateWidgetDefs( const QObjectList* list )
{
    QWidgetRec *rec;
    if (list != 0) {

	uint count = list->count();
        QObjectListIt it (*list);
        while (count > 0 && it.current()) {
	    count--;

	    if (it.current()->isWidgetType()) {
//		&& (!((QWidget*)it.current())->isTopLevel())) {

	        QCString sig = signature( it.current() );
		if (!getWidgetRec( sig, rec)) {

		    QString s = appendWidget( sig );
		    QRemoteMessage S( "WidgetDef", s );
		    S.send( socket );
		}
	    }
	    
	    // append all children too
	    updateWidgetDefs( it.current()->children() );

            ++it;
        }
    }
}

bool QTestWidgets::receiverIsAccessible(QObject *receiver)
{
    assert(receiver);

    if (!receiver->parent()) {
	if (isTopLevelItem(receiver)) {

            QString missing_name;
	    QObject* widget = findWidget( (char*)receiver->name(), missing_name );
	    if (widget == 0) {
		return FALSE;
	    }
	}
    }

    return TRUE;
}

bool QTestWidgets::isTopLevelItem(const QObject* item)
{
    return (item->isWidgetType() && ((QWidget*)item)->isTopLevel ());
}

bool QTestWidgets::addPtr( QObject *widget )
{
    if (widget == 0)
        return FALSE;

    QWidgetRec *tmp = widgetsByPtr.find( widget );
    if (tmp != 0)
        return TRUE;

    // try to locate the instance
    QWidgetRec *rec;
    if (getWidgetRec( signature( widget ), rec )) {
        rec->_ptr = widget;
        widgetsByPtr.insert( widget, rec );
        connect( widget, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyed(QObject*)));
        return TRUE;
    } else
        return FALSE;
}

bool QTestWidgets::checkPtr( QWidgetRec *rec )
{
    if (rec == 0)
        return FALSE;

    if (rec->_ptr != 0) {
        QWidgetRec *tmp = widgetsByPtr.find( rec->_ptr );
        if (tmp != 0)
            return TRUE;
    }

    if (rec->_ptr == 0) {
        // try to locate the instance
        QString missing_name;
        QObject *ptr = findWidget( rec->name(), missing_name, TRUE );
        if (ptr != 0)
            rec->_ptr = ptr;
        else
            return FALSE;
    }

    widgetsByPtr.insert( rec->_ptr, rec );
    connect( rec->_ptr, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyed(QObject*)));
    return TRUE;
}

void QTestWidgets::onDestroyed( QObject *object )
{
    QWidgetRec *rec = widgetsByPtr.find( object );
    if (rec != 0) {
        rec->_ptr = 0;
        widgetsByPtr.remove( object );
    }
}
