/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef METADATABASE_H
#define METADATABASE_H

#include <qvariant.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qlist.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qwidgetlist.h>
#include <qcursor.h>

#include "pixmapchooser.h"
#include <widgetdatabase.h>

class QObject;

class MetaDataBase
{
public:
    struct Connection
    {
	QObject *sender, *receiver;
	QCString signal, slot;
	bool operator==( const Connection &c ) const {
	    return sender == c.sender && receiver == c.receiver &&
		   signal == c.signal && slot == c.slot ;
	}
    };

    struct Slot
    {
	QCString slot;
	QString access;
	bool operator==( const Slot &s ) const {
	    return slot == s.slot &&
		 access == s.access;
	}
    };

    struct Property
    {
	QCString property;
	QString type;
	bool operator==( const Property &p ) const {
	    return property == p.property &&
		 type == p.type;
	}
    };

    struct CustomWidget
    {
	CustomWidget();
	CustomWidget( const CustomWidget &w );
	~CustomWidget();	
	bool operator==( const CustomWidget &w ) const;	
	CustomWidget &operator=( const CustomWidget &w );

	bool hasSignal( const QCString &signal ) const;
	bool hasSlot( const QCString &slot ) const;
	bool hasProperty( const QCString &prop ) const;
	
	enum IncludePolicy { Global, Local };
	QString className;
	QString includeFile;
	IncludePolicy includePolicy;
	QSize sizeHint;
	QSizePolicy sizePolicy;
	QPixmap *pixmap;
	QValueList<QCString> lstSignals;
	QValueList<Slot> lstSlots;
	QValueList<Property> lstProperties;
	int id;
	bool isContainer;
    };

    struct Include
    {
	QString header;
	QString location;
	bool operator==( const Include & ) const;
    };

    struct MetaInfo
    {
	QString className;
	bool classNameChanged;
	QString comment;
	QString author;
    };

    MetaDataBase();
    static void setupDataBase();

    static void addEntry( QObject *o );
    static void removeEntry( QObject *o );
    static void setPropertyChanged( QObject *o, const QString &property, bool changed );
    static bool isPropertyChanged( QObject *o, const QString &property );
    static void setPropertyComment( QObject *o, const QString &property, const QString &comment );
    static QString propertyComment( QObject *o, const QString &property );
    static QStringList changedProperties( QObject *o );

    static void setFakeProperty( QObject *o, const QString &property, const QVariant& value );
    static QVariant fakeProperty( QObject * o, const QString &property );
    static QMap<QString,QVariant>* fakeProperties( QObject* o );

    static void setSpacing( QObject *o, int spacing );
    static int spacing( QObject *o );
    static void setMargin( QObject *o, int margin );
    static int margin( QObject *o );

    static void addConnection( QObject *o, QObject *sender, const QCString &signal,
			       QObject *receiver, const QCString &slot );
    static void removeConnection( QObject *o, QObject *sender, const QCString &signal,
				  QObject *receiver, const QCString &slot );
    static QValueList<Connection> connections( QObject *o );
    static QValueList<Connection> connections( QObject *o, QObject *sender, QObject *receiver );
    static QValueList<Connection> connections( QObject *o, QObject *object );
    static void doConnections( QObject *o );

    static void addSlot( QObject *o, const QCString &slot, const QString &access );
    static void removeSlot( QObject *o, const QCString &slot, const QString &access );
    static QValueList<Slot> slotList( QObject *o );
    static bool isSlotUsed( QObject *o, const QCString &slot );
    static bool hasSlot( QObject *o, const QCString &slot );

    static bool addCustomWidget( CustomWidget *w );
    static void removeCustomWidget( CustomWidget *w );
    static QList<CustomWidget> *customWidgets();
    static CustomWidget *customWidget( int id );
    static bool isWidgetNameUsed( CustomWidget *w );
    static bool hasCustomWidget( const QString &className );

    static void setTabOrder( QWidget *w, const QWidgetList &order );
    static QWidgetList tabOrder( QWidget *w );

    static void setIncludes( QObject *o, const QValueList<Include> &incs );
    static QValueList<Include> includes( QObject *o );

    static void setMetaInfo( QObject *o, MetaInfo mi );
    static MetaInfo metaInfo( QObject *o );

    static void setCursor( QWidget *w, const QCursor &c );
    static QCursor cursor( QWidget *w );

    static void setPixmapArgument( QObject *o, int pixmap, const QString &arg );
    static QString pixmapArgument( QObject *o, int pixmap );
    static void clearPixmapArguments( QObject *o );

};

#endif
