/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#ifndef QT_H
#include <qstring.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qmap.h>
#include <qaction.h>
#include <qcstring.h>
#include <qvariant.h>
#endif // QT_H

class QDomDocument;
class QDomElement;
class QLayout;
class QListView;
class QListViewItem;
class QMenuBar;
class QTable;
class QWidget;
class QWidgetFactoryPrivate;
class UibStrTable;

class QWidgetFactory
{
public:
    QWidgetFactory();
    virtual ~QWidgetFactory();

    static QWidget *create( const QString &uiFile, QObject *connector = 0, QWidget *parent = 0, const char *name = 0 );
    static QWidget *create( QIODevice *dev, QObject *connector = 0, QWidget *parent = 0, const char *name = 0 );
    static void addWidgetFactory( QWidgetFactory *factory );
    static void loadImages( const QString &dir );

    virtual QWidget *createWidget( const QString &className, QWidget *parent, const char *name ) const;
    static QStringList widgets();
    static bool supportsWidget( const QString &widget );

private:
    enum LayoutType { HBox, VBox, Grid, NoLayout };
    void loadImageCollection( const QDomElement &e );
    void loadConnections( const QDomElement &e, QObject *connector );
    void loadTabOrder( const QDomElement &e );
    QWidget *createWidgetInternal( const QDomElement &e, QWidget *parent, QLayout* layout, const QString &classNameArg );
    QLayout *createLayout( QWidget *widget, QLayout*  layout, LayoutType type, bool isQLayoutWidget = FALSE );
    LayoutType layoutType( QLayout *l ) const;
    void setProperty( QObject* widget, const QString &prop, QVariant value );
    void setProperty( QObject* widget, const QString &prop, const QDomElement &e );
    void createSpacer( const QDomElement &e, QLayout *layout );
    QImage loadFromCollection( const QString &name );
    QPixmap loadPixmap( const QString &name );
    QPixmap loadPixmap( const QDomElement &e );
    QColorGroup loadColorGroup( const QDomElement &e );
    void createListViewColumn( QListView *lv, const QString& txt,
			       const QPixmap& pix, bool clickable,
			       bool resizable );
#ifndef QT_NO_TABLE
    void createTableColumnOrRow( QTable *table, const QString& txt,
				 const QPixmap& pix, const QString& field,
				 bool isRow );
#endif
    void createColumn( const QDomElement &e, QWidget *widget );
    void loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap );
    void createItem( const QDomElement &e, QWidget *widget, QListViewItem *i = 0 );
    void loadChildAction( QObject *parent, const QDomElement &e );
    void loadActions( const QDomElement &e );
    void loadToolBars( const QDomElement &e );
    void loadMenuBar( const QDomElement &e );
    void loadPopupMenu( QPopupMenu *p, const QDomElement &e );
    void loadFunctions( const QDomElement &e );
    QAction *findAction( const QString &name );
    void loadExtraSource();
    QString translate( const char *sourceText, const char *comment = "" );
    QString translate( const QString& sourceText, const QString& comment = QString::null );

    void unpackUInt16( QDataStream& in, Q_UINT16& n );
    void unpackUInt32( QDataStream& in, Q_UINT32& n );
    void unpackByteArray( QDataStream& in, QByteArray& array );
    void unpackCString( const UibStrTable& strings, QDataStream& in,
			QByteArray& cstr );
    void unpackString( const UibStrTable& strings, QDataStream& in,
		       QString& str );
    void unpackStringSplit( const UibStrTable& strings, QDataStream& in,
			    QString& str );
    void unpackVariant( const UibStrTable& strings, QDataStream& in,
			QVariant& value );
    void inputSpacer( const UibStrTable& strings, QDataStream& in,
		      QLayout *parent );
    void inputColumnOrRow( const UibStrTable& strings, QDataStream& in,
			   QObject *parent, bool isRow );
    void inputItem( const UibStrTable& strings, QDataStream& in,
		    QObject *parent, QListViewItem *parentItem = 0 );
    void inputMenuItem( QObject **objects, const UibStrTable& strings,
			QDataStream& in, QMenuBar *menuBar );
    QObject *inputObject( QObject **objects, int& numObjects,
			  const UibStrTable& strings, QDataStream& in,
			  QWidget *ancestorWidget, QObject *parent,
			  QCString className = "" );
    QWidget *createFromUiFile( QDomDocument doc, QObject *connector,
			       QWidget *parent, const char *name );
    QWidget *createFromUibFile( QDataStream& in, QObject *connector,
				QWidget *parent, const char *name );

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
	Q_DUMMY_COMPARISON_OPERATOR( Field )
    };

    struct SqlWidgetConnection
    {
	SqlWidgetConnection() {}
	SqlWidgetConnection( const QString &c, const QString &t )
	    : conn( c ), table( t ), dbControls( new QMap<QString, QString>() ) {}
	QString conn;
	QString table;
	QMap<QString, QString> *dbControls;
	Q_DUMMY_COMPARISON_OPERATOR( SqlWidgetConnection )
    };

    QList<Image> images;
    QWidget *toplevel;
    QWidgetFactoryPrivate *d;
    QMap<QString, QString> *dbControls;
    QMap<QString, QStringList> dbTables;
    QMap<QWidget*, SqlWidgetConnection> sqlWidgetConnections;
    QMap<QString, QString> buddies;
    QMap<QTable*, QList<Field> > fieldMaps;
    QList<QAction*> actionList;
    QMap<QString, QString> languageSlots;
    QStringList noDatabaseWidgets;
    bool usePixmapCollection;
    int defMargin;
    int defSpacing;
    QString code;
    QString uiFileVersion;
};

#endif
