#ifndef WIDGETDEFS_H
#define WIDGETDEFS_H

#include <qobject.h>
#include <qstring.h>
#include <qintdict.h>
#include <qasciidict.h>

class WidgetDefs : public QObject
{
    Q_OBJECT

public:
    WidgetDefs();
    virtual ~WidgetDefs();

    bool findWidgetIndex( const QCString &name, int &index );
    bool findWidgetName( int index, QCString &name );

    bool appendWidgetDef( const QString &def, QCString &name, int &index );
    bool appendWidget( const QCString &name, int index, bool &indexError );
    QString appendWidget( const QCString &widget_name );

    uint count();
    QString defString( uint index );

    virtual void clear();

private:
    int		    next_widget_index;
    QIntDict<char>  index2widgetName_;
    QAsciiDict<int> widgetName2index_;
};

#endif
