#include "widgetdefs_p.h"

#include <qstring.h>

class QKeyString : public QString
{
public:
    QKeyString();
    QKeyString( const QString &s );

    void addKey( const QString &key, const QString &value );
    bool findKey( const QString &key, QString &keyValue, bool resetIfNotFound = TRUE );

    void addKey( const QString &key, int value );
    bool findKey( const QString &key, int &value, bool resetIfNotFound = TRUE );
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

//***********************************************************

WidgetDefs::WidgetDefs()
{
    index2widgetName_.clear();
    index2widgetName_.resize(9973);
    widgetName2index_.clear();
    widgetName2index_.resize(9973);
    next_widget_index = 0;
}

WidgetDefs::~WidgetDefs()
{
    clear();
}

void WidgetDefs::clear()
{
    // clear all dictionaries
    index2widgetName_.clear();
    widgetName2index_.clear();
    next_widget_index = 0;
}

/*!
    Searches for a widget \a name and returns an \a index to it.
    Returns TRUE if the widget was found.
*/

bool WidgetDefs::findWidgetIndex( const QCString &name, int &index )
{
    QKeyString w;
    w.addKey("NAME",(QString)name);

    int *i = widgetName2index_.find(w);
    if (i) {
	index = *i;
	return TRUE;
    } 

    return FALSE;
}

/*!
    Searches for the \a name of a widget based on an \a index.
    Returns TRUE if the \a index was valid.
*/

bool WidgetDefs::findWidgetName( int index, QCString &name )
{
    QKeyString w = QString( index2widgetName_.find(index) );
    if (!w.isNull()) {
	QString tmp;
	w.findKey("NAME",tmp,TRUE);
	name = tmp;
	return TRUE;
    }
    return FALSE;
}

bool WidgetDefs::appendWidget( const QCString &widget_name, int widget_index, bool &indexError )
{
    int tmp_index = -1;
    if (findWidgetIndex( widget_name, tmp_index )) {
	indexError = TRUE;
	return FALSE;
    }

    QCString tmpName;
    if (findWidgetName( widget_index, tmpName )) {
	indexError = FALSE;
        return FALSE;
    }
	
    // insert the name and index into two qdict's 
    QKeyString w;
    w.addKey("NAME",(QString)widget_name);
    widgetName2index_.insert( w, new int( widget_index ) );
    index2widgetName_.insert( widget_index, ::qstrdup( w ) );
    if (widget_index >= next_widget_index)
	next_widget_index = widget_index + 1;
    return TRUE;
}

QString WidgetDefs::defString( uint list_index )
{
    QString S;
    uint count = 0;
    QIntDictIterator<char> it(index2widgetName_);
    while (it.current()) {
	if (count == list_index) {
	    QKeyString S(it.current());
	    S.addKey("ID",it.currentKey());
	    return S;
	}

	count++;
	++it;
    }

    return S;
}

QString WidgetDefs::appendWidget( const QCString &name )
{
    QKeyString w;
    w.addKey("NAME",(QString)name);

    widgetName2index_.insert( w, new int(next_widget_index) );
    index2widgetName_.insert( next_widget_index, ::qstrdup(w) );

    w.addKey( "ID", next_widget_index++ );

    return w;
}

bool WidgetDefs::appendWidgetDef(const QString &def, QCString &name, int &widget_index)
{
    QKeyString w(def);
    QString wName, wAlias;
    if (!w.findKey("NAME",wName,TRUE) ||
	!w.findKey("ID",widget_index,TRUE))
	return FALSE;

    bool error;
    name = wName;
    return appendWidget(name, widget_index, error);
}

uint WidgetDefs::count()
{
    return widgetName2index_.count();
}
 
