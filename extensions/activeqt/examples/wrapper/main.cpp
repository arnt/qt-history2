/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qaxfactory.h>

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpixmap.h>

/* XPM */
static const char *fileopen[] = {
"    16    13        5            1",
". c #040404",
"# c #808304",
"a c None",
"b c #f3f704",
"c c #f3f7f3",
"aaaaaaaaa...aaaa",
"aaaaaaaa.aaa.a.a",
"aaaaaaaaaaaaa..a",
"a...aaaaaaaa...a",
".bcb.......aaaaa",
".cbcbcbcbc.aaaaa",
".bcbcbcbcb.aaaaa",
".cbcb...........",
".bcb.#########.a",
".cb.#########.aa",
".b.#########.aaa",
"..#########.aaaa",
"...........aaaaa"
};


class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app ) 
	: QAxFactory( lib, app ) 
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << "QCheckBox";
	list << "QRadioButton";
	list << "QPushButton";
	list << "QToolButton";
	return list;
    }
    QObject *createObject(const QString &key)
    {
	if ( key == "QCheckBox" )
	    return new QCheckBox(0);
	if ( key == "QRadioButton" )
	    return new QRadioButton(0);
	if ( key == "QPushButton" )
	    return new QPushButton(0 );
	if ( key == "QToolButton" ) {
	    QToolButton *tb = new QToolButton(0);
//	    tb->setIcon( QPixmap(fileopen) );
	    return tb;
	}

	return 0;
    }
    const QMetaObject *metaObject( const QString &key ) const
    {
	if ( key == "QCheckBox" )
	    return &QCheckBox::staticMetaObject;
	if ( key == "QRadioButton" )
	    return &QRadioButton::staticMetaObject;
	if ( key == "QPushButton" )
	    return &QPushButton::staticMetaObject;
	if ( key == "QToolButton" )
	    return &QToolButton::staticMetaObject;

	return 0;
    }
    QUuid classID( const QString &key ) const
    {
	if ( key == "QCheckBox" )
	    return "{6E795DE9-872D-43CF-A831-496EF9D86C68}";
	if ( key == "QRadioButton" )
	    return "{AFCF78C8-446C-409A-93B3-BA2959039189}";
	if ( key == "QPushButton" )
	    return "{2B262458-A4B6-468B-B7D4-CF5FEE0A7092}";
	if ( key == "QToolButton" )
	    return "{7c0ffe7a-60c3-4666-bde2-5cf2b54390a1}";

	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QCheckBox" )
	    return "{4FD39DD7-2DE0-43C1-A8C2-27C51A052810}";
	if ( key == "QRadioButton" )
	    return "{7CC8AE30-206C-48A3-A009-B0A088026C2F}";
	if ( key == "QPushButton" )
	    return "{06831CC9-59B6-436A-9578-6D53E5AD03D3}";
	if ( key == "QToolButton" )
	    return "{6726080f-d63d-4950-a366-9bf33e5cdf84}";

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QCheckBox" )
	    return "{FDB6FFBE-56A3-4E90-8F4D-198488418B3A}";
	if ( key == "QRadioButton" )
	    return "{73EE4860-684C-4A66-BF63-9B9EFFA0CBE5}";
	if ( key == "QPushButton" )
	    return "{3CC3F17F-EA59-4B58-BBD3-842D467131DD}";
	if ( key == "QToolButton" )
	    return "{f4d421fd-4ead-4fd9-8a25-440766939639}";

	return QUuid();
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{3B756301-0075-4E40-8BE8-5A81DE2426B7}", "{AB068077-4924-406a-BBAF-42D91C8727DD}" )
