#ifndef QTESTWIDGETS_P_H
#define QTESTWIDGETS_P_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qintdict.h>
#include <qasciidict.h>
#include <qsocket.h>
#include <qptrdict.h>

class QTestWidgets : public QObject
{
    Q_OBJECT

public:
    QTestWidgets();
    virtual ~QTestWidgets();

    void setSocket( QSocket *s );

    class QWidgetRec : public QObject
    {
    public:
        QWidgetRec( QString signature, int id );
        virtual ~QWidgetRec();

        QCString _signature;
        int _id;
        QObject *_ptr;

        QCString name();
    };

    bool getWidgetRec( int id, QWidgetRec *&rec );
    bool getWidgetRec( QCString signature, QWidgetRec *&rec );
    bool getWidgetRec( QObject *o, QWidgetRec *&rec );
    
    bool getWidget( int id, QObject *&widget );
    bool getWidget( QCString name, QObject *&widget );

    bool addPtr( QObject *widget );
    bool checkPtr( QWidgetRec *rec );

    bool appendWidgetDef( const QString &def );

    QString appendWidget( const QCString &widgetName );
    bool appendWidget( const QCString &name, int widgetId );

    void updateWidgetDefs( const QObjectList* list );

    uint count();
    virtual void clear();

    QObject* findWidget( const QCString &name, QString &missing_name, bool allow_invisible_widgets = TRUE );
    QObject* findAppWidget( const QCString &name, QString &missing_name, bool allow_invisible_widgets);
    QObject* findGlobalWidget( const QCString &name );

    QCString signature( const QObject* item );

    bool widgetIsVisible( QObject* widget );

protected:
    bool hasName( const QObject* item );
    QCString getFullName( const QObject* item );
    QObject* findWidgetByName( const QCString &name, const QObjectList *search_list, QString& missing_name, bool allow_invisible_widgets);
    bool receiverIsAccessible( QObject *receiver );
    bool isTopLevelItem( const QObject* item );

private:
    int		    next_widget_index;

    char            nameDelimiterChar;
    QSocket         *socket;
    QStringList     multiple_instances;

    QIntDict<QWidgetRec>   widgetsById;
    QAsciiDict<QWidgetRec> widgetsBySignature;
    QPtrDict<QWidgetRec> widgetsByPtr;
    uint            widgets_size;

public slots:
    void onDestroyed( QObject *object );

};

#endif
