#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#ifndef QT_H
#include "qstring.h"
#include "qcstring.h"
#include "qlist.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

class QWidget;
class QLayout;
class QDomElement;
class QListViewItem;
class QTable;
class QAction;

class QWidgetFactory
{
public:
    QWidgetFactory();
    virtual ~QWidgetFactory() {}

    static QWidget *create( const QString &uiFile, QObject *connector = 0, QWidget *parent = 0, const char *name = 0 );
    static QWidget *create( QIODevice *dev, QObject *connector = 0, QWidget *parent = 0, const char *name = 0 );
    static void addWidgetFactory( QWidgetFactory *factory );
    static bool openDatabaseConnections( const QString &dbFileName );

    virtual QWidget *createWidget( const QString &className, QWidget *parent, const char *name ) const;

private:
    enum LayoutType { HBox, VBox, Grid, NoLayout };
    void loadImageCollection( const QDomElement &e );
    void loadConnections( const QDomElement &e, QObject *connector );
    void loadTabOrder( const QDomElement &e );
    QWidget *createWidgetInternal( const QDomElement &e, QWidget *parent, QLayout* layout, const QString &classNameArg );
    QLayout *createLayout( QWidget *widget, QLayout*  layout, LayoutType type );
    LayoutType layoutType( QLayout *l ) const;
    void setProperty( QObject* widget, const QString &prop, const QDomElement &e );
    void createSpacer( const QDomElement &e, QLayout *layout );
    QImage loadFromCollection( const QString &name );
    QPixmap loadPixmap( const QDomElement &e );
    QColorGroup loadColorGroup( const QDomElement &e );
    void createColumn( const QDomElement &e, QWidget *widget );
    void loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap );
    void createItem( const QDomElement &e, QWidget *widget, QListViewItem *i = 0 );
    void loadActions( const QDomElement &e );

private:
    struct Image {
	QImage img;
	QString name;
	bool operator==(  const Image &i ) const {
	    return ( i.name == name &&
		     i.img == img );
	}
    };

    struct Field
    {
	Field() {}
	Field( const QString &s1, const QPixmap &p, const QString &s2 ) : name( s1 ), pix( p ), field( s2 ) {}
	QString name;
	QPixmap pix;
	QString field;
    };

    QValueList<Image> images;
    QWidget *toplevel;
    QListViewItem *lastItem;
    QMap<QString, QString> dbControls;
    QMap<QString, QStringList> dbTables;
    QString defConnection, defTable;
    QMap<QString, QString> buddies;
    QMap<QTable*, QValueList<Field> > fieldMaps;
    QList<QAction> actionList;

};

#endif
