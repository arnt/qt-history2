/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qmessagebox.h>
#include <qaxfactory.h>

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app ) 
	: QAxFactory( lib, app ) 
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << "QButton";
	list << "QCheckBox";
	list << "QRadioButton";
	list << "QPushButton";
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( key == "QButton" )
	    return new QButton( parent, name );
	if ( key == "QCheckBox" )
	    return new QCheckBox( parent, name );
	if ( key == "QRadioButton" )
	    return new QRadioButton( parent, name );
	if ( key == "QPushButton" )
	    return new QPushButton( parent, name );

	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( key == "QButton" )
	    return QButton::staticMetaObject();
	if ( key == "QCheckBox" )
	    return QCheckBox::staticMetaObject();
	if ( key == "QRadioButton" )
	    return QRadioButton::staticMetaObject();
	if ( key == "QPushButton" )
	    return QPushButton::staticMetaObject();

	return 0;
    }
    QUuid classID( const QString &key ) const
    {
	if ( key == "QButton" )
	    return "{23F5012A-7333-43D3-BCA8-836AABC61B4A}";
	if ( key == "QCheckBox" )
	    return "{6E795DE9-872D-43CF-A831-496EF9D86C68}";
	if ( key == "QRadioButton" )
	    return "{AFCF78C8-446C-409A-93B3-BA2959039189}";
	if ( key == "QPushButton" )
	    return "{2B262458-A4B6-468B-B7D4-CF5FEE0A7092}";

	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QButton" )
	    return "{6DA689FB-928F-423C-8632-678C3D3606DB}";
	if ( key == "QCheckBox" )
	    return "{4FD39DD7-2DE0-43C1-A8C2-27C51A052810}";
	if ( key == "QRadioButton" )
	    return "{7CC8AE30-206C-48A3-A009-B0A088026C2F}";
	if ( key == "QPushButton" )
	    return "{06831CC9-59B6-436A-9578-6D53E5AD03D3}";

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QButton" )
	    return "{73A5D03F-8ADE-4D84-9AE0-A93B4F85A130}";
	if ( key == "QCheckBox" )
	    return "{FDB6FFBE-56A3-4E90-8F4D-198488418B3A}";
	if ( key == "QRadioButton" )
	    return "{73EE4860-684C-4A66-BF63-9B9EFFA0CBE5}";
	if ( key == "QPushButton" )
	    return "{3CC3F17F-EA59-4B58-BBD3-842D467131DD}";

	return QUuid();
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{3B756301-0075-4E40-8BE8-5A81DE2426B7}", "{AB068077-4924-406a-BBAF-42D91C8727DD}" )

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    if ( !QAxFactory::isServer() ) {
	QMessageBox::critical( 0, "Cannot Run stand alone!", "This executable is a server for ActiveX controls.\nIt cannot be run stand alone." );
	return -1;
    }

    return app.exec();
}
