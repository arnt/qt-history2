/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qdom.h>
#include <qfile.h>
#include <qobject.h>
#include <qrect.h>
#include <qsizepolicy.h>

#include <domtool.h>

#include "ui2uib.h"
#include "uib.h"

#define COMPILE_STATS

#ifdef COMPILE_STATS
static QMap<int, int> totalPerBlock;
static int total;
#endif

class UibHack : public QObject
{
public:
    static QString normalize( const QString& member ) {
	return QString::fromUtf8( QObject::normalizeSignalSlot(member.utf8()) );
    }
};

class UibIndexMap
{
public:
    UibIndexMap() : next( 0 ) { }

    void insert( const QString& name ) { setName( insert(), name ); }
    int insert() { return next++; }
    void setName( int no, const QString& name );

    int find( const QString& name, int deflt = -1 ) const;
    int count() const { return next; }

private:
    QMap<QString, int> nameMap;
    int next;
};

void UibIndexMap::setName( int no, const QString& name )
{
    nameMap.insert( name, no );
}

int UibIndexMap::find( const QString & name, int deflt ) const
{
    QMap<QString, int>::ConstIterator no = nameMap.find( name );
    if ( no == nameMap.end() ) {
	return deflt;
    } else {
	return *no;
    }
}

static void packUInt16( QDataStream& out, Q_UINT16 n )
{
    if ( n < 255 ) {
	out << (Q_UINT8) n;
    } else {
	out << (Q_UINT8) 255;
	out << n;
    }
}

static void packUInt32( QDataStream& out, Q_UINT32 n )
{
    if ( n < 65535 ) {
	out << (Q_UINT16) n;
    } else {
	out << (Q_UINT16) 65535;
	out << n;
    }
}

static void packByteArray( QDataStream& out, const QByteArray& array )
{
    packUInt32( out, array.size() );
    out.writeRawBytes( array.data(), array.size() );
}

static void packCString( UibStrTable& strings, QDataStream& out,
			 const char *cstr )
{
    packUInt32( out, strings.insertCString(cstr) );
}

static void packString( UibStrTable& strings, QDataStream& out,
			const QString& str )
{
    packUInt32( out, strings.insertString(str) );
}

static void packStringSplit( UibStrTable& strings, QDataStream& out,
			     const QString& str, QChar sep )
{
    int pos = str.find( sep );
    if ( pos == -1 )
	pos = str.length();
    packString( strings, out, str.left(pos) );
    packString( strings, out, str.mid(pos) );
}

static void packVariant( UibStrTable& strings, QDataStream& out,
			 QVariant value )
{
    QStringList::ConstIterator s;
    Q_UINT8 type = value.type();

    out << type;

    switch ( type ) {
    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::IconSet:
	packString( strings, out, value.asString() );
	break;
    case QVariant::StringList:
	packUInt16( out, value.asStringList().count() );
	s = value.asStringList().begin();
	while ( s != value.asStringList().end() ) {
	    packString( strings, out, *s );
	    ++s;
	}
	break;
    case QVariant::Font:
	out << value.asFont();
	break;
    case QVariant::Rect:
	packUInt16( out, value.asRect().x() );
	packUInt16( out, value.asRect().y() );
	packUInt16( out, value.asRect().width() );
	packUInt16( out, value.asRect().height() );
	break;
    case QVariant::Size:
	packUInt16( out, value.asSize().width() );
	packUInt16( out, value.asSize().height() );
	break;
    case QVariant::Color:
	out << value.asColor();
	break;
    case QVariant::Point:
	packUInt16( out, value.asPoint().x() );
	packUInt16( out, value.asPoint().y() );
	break;
    case QVariant::Int:
	packUInt32( out, value.asInt() );
	break;
    case QVariant::Bool:
	out << (Q_UINT8) value.asBool();
	break;
    case QVariant::Double:
	out << value.asDouble();
	break;
    case QVariant::CString:
	packCString( strings, out, value.asCString() );
	break;
    case QVariant::Cursor:
	out << value.asCursor();
	break;
    case QVariant::Date:
	out << value.asDate();
	break;
    case QVariant::Time:
	out << value.asTime();
	break;
    case QVariant::DateTime:
	out << value.asDateTime();
	break;
    default:
	out << value;
    }
}

static void outputProperty( QMap<int, QStringList>& buddies, int objectNo,
			    UibStrTable& strings, QDataStream& out,
			    QDomElement elem )
{
    QCString name = elem.attribute( "name" ).latin1();
    QDomElement f = elem.firstChild().toElement();
    QString tag = f.tagName();
    QString comment;
    QVariant value;

    if ( tag == "palette" ) {
	out << (Q_UINT8) Object_PaletteProperty;
	QDomElement g = f.firstChild().toElement();
	while ( !g.isNull() ) {
	    if ( g.tagName() == "active" ) {
		out << (Q_UINT8) Palette_Active;
	    } else if ( g.tagName() == "inactive" ) {
		out << (Q_UINT8) Palette_Inactive;
	    } else {
		out << (Q_UINT8) Palette_Disabled;
	    }
	    QDomElement h = g.firstChild().toElement();
	    while ( !h.isNull() ) {
		value = DomTool::elementToVariant( h, Qt::gray );
		if ( h.tagName() == "color" ) {
		    out << (Q_UINT8) Palette_Color;
		    out << value.asColor();
		} else if ( h.tagName() == "pixmap" ) {
		    out << (Q_UINT8) Palette_Pixmap;
		    packString( strings, out, value.asString() );
		}
		h = h.nextSibling().toElement();
	    }
	    g = g.nextSibling().toElement();
	}
	out << (Q_UINT8) Palette_End;
    } else {
	value = DomTool::elementToVariant( f, value, comment );
	if ( value.isValid() ) {
	    if ( name == "buddy" ) {
		buddies[objectNo] += value.asString();
	    } else {
		if ( tag == "string" ) {
		    out << (Q_UINT8) Object_StringProperty;
		    packCString( strings, out, name );
		    packCString( strings, out, value.asString().utf8() );
		    packCString( strings, out, comment.utf8() );
		} else {
		    out << (Q_UINT8) Object_VariantProperty;
		    packCString( strings, out, name );
		    packVariant( strings, out, value );
		}
	    }
	}
    }
}

// ### QLayoutWidget
static int outputObject( QMap<int, QStringList>& buddies,
			 UibIndexMap& objects, UibStrTable& strings,
			 QDataStream& out, QDomElement elem,
			 QCString className = "" )
{
    bool isQObject = !className.isEmpty();
    if ( className == "QWidget" )
	className = elem.attribute( "class", className ).latin1();

    int objectNo = -1;
    if ( isQObject ) {
	packCString( strings, out, className );
	objectNo = objects.insert();
    }

    int column = elem.attribute( "column", "0" ).toInt();
    int row = elem.attribute( "row", "0" ).toInt();
    int colspan = elem.attribute( "colspan", "1" ).toInt();
    int rowspan = elem.attribute( "rowspan", "1" ).toInt();
    if ( colspan < 1 )
	colspan = 1;
    if ( rowspan < 1 )
	rowspan = 1;

    if ( column != 0 || row != 0 || colspan != 1 || rowspan != 1 ) {
	out << (Q_UINT8) Object_GridCell;
	packUInt16( out, column );
	packUInt16( out, row );
	packUInt16( out, colspan );
	packUInt16( out, rowspan );
    }

    // optimization: insert '&Foo' into string-table before 'Foo'
    if ( className == "QAction" ) {
	QVariant value = DomTool::readProperty( elem, "menuText", QVariant() );
	if ( value.asString().startsWith("&") ) {
	    QDataStream out2;
	    packCString( strings, out2, value.asString().utf8() );
	}
    }

    QDomElement f = elem.firstChild().toElement();
    while ( !f.isNull() ) {
	QString tag = f.tagName();
	if ( tag == "action" ) {
	    if ( f.hasAttribute("name") ) {
		QString actionName = f.attribute( "name" );
		int no = objects.find( actionName );
		if ( no != -1 ) {
		    out << (Q_UINT8) Object_ActionRef;
		    packUInt16( out, no );
		}
	    } else {
		out << (Q_UINT8) Object_Action;
		outputObject( buddies, objects, strings, out, f );
	    }
	} else if ( tag == "actiongroup" ) {
	    out << (Q_UINT8) Object_ActionGroup;
	    outputObject( buddies, objects, strings, out, f );
	} else if ( tag == "attribute" ) {
	    out << (Q_UINT8) Object_Attribute;
	    outputProperty( buddies, objectNo, strings, out, f );
	} else if ( tag == "column" ) {
	    out << (Q_UINT8) Object_TableColumn;
	    outputObject( buddies, objects, strings, out, f );
	} else if ( tag == "grid" ) {
	    out << (Q_UINT8) Object_SubLayout;
	    outputObject( buddies, objects, strings, out, f, "QGridLayout" );
	} else if ( tag == "hbox" ) {
	    out << (Q_UINT8) Object_SubLayout;
	    outputObject( buddies, objects, strings, out, f, "QHBoxLayout" );
	} else if ( tag == "item" ) {
	    out << (Q_UINT8) Object_Item;
	    outputObject( buddies, objects, strings, out, f );
	} else if ( tag == "property" ) {
	    outputProperty( buddies, objectNo, strings, out, f );
	} else if ( tag == "row" ) {
	    out << (Q_UINT8) Object_TableRow;
	    outputObject( buddies, objects, strings, out, f );
	} else if ( tag == "separator" ) {
	    out << (Q_UINT8) Object_Separator;
	} else if ( tag == "spacer" ) {
	    out << (Q_UINT8) Object_Spacer;
	    outputObject( buddies, objects, strings, out, f );
	} else if ( tag == "vbox" ) {
	    out << (Q_UINT8) Object_SubLayout;
	    outputObject( buddies, objects, strings, out, f, "QVBoxLayout" );
	} else if ( tag == "widget" ) {
	    out << (Q_UINT8) Object_SubWidget;
	    outputObject( buddies, objects, strings, out, f, "QWidget" );
	}
	f = f.nextSibling().toElement();
    }
    out << (Q_UINT8) Object_End;
    if ( isQObject )
	objects.setName( objectNo,
			 DomTool::readProperty(elem, "name", "unnamed")
			 .asString() );
}

static void outputBlock( QDataStream& out, BlockTag tag,
			 const QByteArray& data )
{
    if ( !data.isEmpty() ) {
	out << (Q_UINT8) tag;
	packByteArray( out, data );
    }
#ifdef COMPILE_STATS
    *totalPerBlock.insert( tag, 0, FALSE ) += data.size();
    total += data.size();
#endif
}

void convertUiToUib( QDomDocument& doc, QDataStream& out )
{
    QByteArray introBlock;
    QByteArray actionsBlock;
    QByteArray buddiesBlock;
    QByteArray connectionsBlock;
    QByteArray functionsBlock;
    QByteArray imagesBlock;
    QByteArray menubarBlock;
    QByteArray slotsBlock;
    QByteArray tabstopsBlock;
    QByteArray toolbarsBlock;
    QByteArray variablesBlock;
    QByteArray widgetBlock;

    QDomElement connectionsElem;
    QDomElement imagesElem;
    QDomElement menubarElem;
    QDomElement tabstopsElem;
    QDomElement toolbarsElem;
    QDomElement widgetElem;

    QMap<int, QStringList> buddies;
    UibStrTable strings;
    UibIndexMap objects;
    int widgetNo = -1;
    QCString className;
    Q_INT16 defaultMargin = -32768;
    Q_INT16 defaultSpacing = -32768;
    Q_UINT8 introFlags = 0;

    QDomElement elem = doc.firstChild().toElement().firstChild().toElement();
    while ( !elem.isNull() ) {
	QString tag = elem.tagName();

	switch ( tag[0].latin1() ) {
	case 'a':
	    if ( tag == "actions" ) {
		QDataStream out2( actionsBlock, IO_WriteOnly );
		outputObject( buddies, objects, strings, out2, elem );
	    }
	    break;
	case 'c':
	    if ( tag == "class" ) {
		className = elem.firstChild().toText().data().latin1();
	    } else if ( tag == "connections" ) {
		connectionsElem = elem;
	    }
	    break;
	case 'f':
	    if ( tag == "functions" ) {
		QDataStream out2( functionsBlock, IO_WriteOnly );
		QDomElement f = elem.firstChild().toElement();
		while ( !f.isNull() ) {
		    if ( f.tagName() == "function" ) {
			packStringSplit( strings, out2,
					 f.attribute("name").latin1(), '(' );
			packString( strings, out2,
				    f.firstChild().toText().data() );
		    }
		    f = f.nextSibling().toElement();
		}
	    }
	    break;
	case 'i':
	    if ( tag == "images" ) {
		QDataStream out2( imagesBlock, IO_WriteOnly );
		QDomElement f = elem.firstChild().toElement();
		while ( !f.isNull() ) {
		    if ( f.tagName() == "image" ) {
			QString name = f.attribute( "name" );
			QDomElement g = f.firstChild().toElement();
			if ( g.tagName() == "data" ) {
			    QString format = g.attribute( "format", "PNG" );
			    QString hex = g.firstChild().toText().data();
			    int n = hex.length() / 2;
			    QByteArray data( n );
			    for ( int i = 0; i < n; i++ )
				data[i] = (char) hex.mid( 2 * i, 2 )
						    .toUInt( 0, 16 );

			    packString( strings, out2, name );
			    packString( strings, out2, format );
			    packUInt32( out2, g.attribute("length").toInt() );
			    packByteArray( out2, data );
			}
		    }
		    f = f.nextSibling().toElement();
		}
	    }
	    break;
	case 'l':
	    if ( tag == "layoutdefaults" ) {
		QString margin = elem.attribute( "margin" );
		if ( !margin.isEmpty() )
		    defaultMargin = margin.toInt();
		QString spacing = elem.attribute( "spacing" );
		if ( !spacing.isEmpty() )
		    defaultSpacing = spacing.toInt();
	    }
	    break;
	case 'm':
	    if ( tag == "menubar" )
		menubarElem = elem;
	    break;
	case 'p':
	    if ( tag == "pixmapinproject" )
		introFlags |= Intro_Pixmapinproject;
	    break;
	case 's':
	    if ( tag == "slots" ) {
		QDataStream out2( slotsBlock, IO_WriteOnly );
		QDomElement f = elem.firstChild().toElement();
		while ( !f.isNull() ) {
		    if ( f.tagName() == "slot" ) {
			QString language = f.attribute( "language", "C++" );
			QString slot = UibHack::normalize(
				f.firstChild().toText().data() );
			packString( strings, out2, language );
			packStringSplit( strings, out2, slot, '(' );
		    }
		    f = f.nextSibling().toElement();
		}
	    }
	    break;
	case 't':
	    if ( tag == "tabstops" ) {
		tabstopsElem = elem;
	    } else if ( tag == "toolbars" ) {
		toolbarsElem = elem;
	    }
	    break;
	case 'v':
	    if ( tag == "variable" ) {
		QDataStream out2( variablesBlock, IO_WriteOnly | IO_Append );
		packString( strings, out, elem.firstChild().toText().data() );
	    } else if ( tag == "variables" ) {
		QDataStream out2( variablesBlock, IO_WriteOnly );
		QDomElement f = elem.firstChild().toElement();
		while ( !f.isNull() ) {
		    if ( f.tagName() == "variable" )
			packString( strings, out,
				    f.firstChild().toText().data() );
		    f = f.nextSibling().toElement();
		}
	    }
	    break;
	case 'w':
	    if ( tag == "widget" )
		widgetElem = elem;
	}
	elem = elem.nextSibling().toElement();
    }

    {
	QDataStream out2( widgetBlock, IO_WriteOnly );
	widgetNo = outputObject( buddies, objects, strings, out2, widgetElem,
				 "QWidget" );
    }

    if ( !tabstopsElem.isNull() ) {
	QDataStream out2( tabstopsBlock, IO_WriteOnly );
	QDomElement f = tabstopsElem.firstChild().toElement();
	while ( !f.isNull() ) {
	    if ( f.tagName() == "tabstop" ) {
		QString widgetName = f.firstChild().toText().data();
		int no = objects.find( widgetName );
		if ( no != -1 )
		    packUInt16( out2, no );
	    }
	    f = f.nextSibling().toElement();
	}
    }

    if ( !menubarElem.isNull() ) {
	QDataStream out2( menubarBlock, IO_WriteOnly );
	outputObject( buddies, objects, strings, out2, menubarElem,
		      "QMenuBar" );
    }

    if ( !toolbarsElem.isNull() ) {
	QDataStream out2( toolbarsBlock, IO_WriteOnly );
	QDomElement f = toolbarsElem.firstChild().toElement();
	while ( !f.isNull() ) {
	    if ( f.tagName() == "toolbar" ) {
		out2 << (Q_UINT8) f.attribute( "dock", "0" ).toInt();
		outputObject( buddies, objects, strings, out2, f, "QToolBar" );
	    }
	    f = f.nextSibling().toElement();
	}
    }

    if ( !buddies.isEmpty() ) {
	QDataStream out2( buddiesBlock, IO_WriteOnly );
	QMap<int, QStringList>::ConstIterator a = buddies.begin();
	while ( a != buddies.end() ) {
	    QStringList::ConstIterator b = (*a).begin();
	    while ( b != (*a).end() ) {
		int no = objects.find( *b );
		if ( no != -1 ) {
		    packUInt16( out2, a.key() );
		    packUInt16( out2, no );
		}
		++b;
	    }
	    ++a;
	}
    }

    if ( !connectionsElem.isNull() ) {
	QString prevLanguage = "C++";
	int prevSenderNo = 0;
	QString prevSignal = "clicked()";
	int prevReceiverNo = 0;
	QString prevSlot = "accept()";

	QDataStream out2( connectionsBlock, IO_WriteOnly );
	QDomElement f = connectionsElem.firstChild().toElement();
	while ( !f.isNull() ) {
	    if ( f.tagName() == "connection" ) {
		QMap<QString, QString> argMap;

		QDomElement g = f.firstChild().toElement();
		while ( !g.isNull() ) {
		    argMap[g.tagName()] = g.firstChild().toText().data();
		    g = g.nextSibling().toElement();
		}

		QString language = f.attribute( "language", "C++" );
		int senderNo = objects.find( argMap["sender"], widgetNo );
		int receiverNo = objects.find( argMap["receiver"], widgetNo );
		QString signal = UibHack::normalize( argMap["signal"] );
		QString slot = UibHack::normalize( argMap["slot"] );

		Q_UINT8 connectionFlags = 0;
		if ( language != prevLanguage )
		    connectionFlags |= Connection_Language;
		if ( senderNo != prevSenderNo )
		    connectionFlags |= Connection_Sender;
		if ( signal != prevSignal )
		    connectionFlags |= Connection_Signal;
		if ( receiverNo != prevReceiverNo )
		    connectionFlags |= Connection_Receiver;
		if ( slot != prevSlot )
		    connectionFlags |= Connection_Slot;
		out2 << connectionFlags;

		if ( connectionFlags & Connection_Language )
		    packString( strings, out2, language );
		if ( connectionFlags & Connection_Sender )
		    packUInt16( out2, senderNo );
		if ( connectionFlags & Connection_Signal )
		    packStringSplit( strings, out2, signal, '(' );
		if ( connectionFlags & Connection_Receiver )
		    packUInt16( out2, receiverNo );
		if ( connectionFlags & Connection_Slot )
		    packStringSplit( strings, out2, slot, '(' );

		prevLanguage = language;
		prevSenderNo = senderNo;
		prevSignal = signal;
		prevReceiverNo = receiverNo;
		prevSlot = slot;
	    } else if ( f.tagName() == "slot" ) {
		// ###
	    }
	    f = f.nextSibling().toElement();
	}
    }

    {
	QDataStream out2( introBlock, IO_WriteOnly );
	out2 << introFlags;
	out2 << defaultMargin;
	out2 << defaultSpacing;
	packUInt16( out2, objects.count() );
	packCString( strings, out2, className );
    }

    out << UibMagic;
    out << (Q_UINT8) '\n';
    out << (Q_UINT8) '\r';
    outputBlock( out, Block_Strings, strings.block() );
    outputBlock( out, Block_Intro, introBlock );
    outputBlock( out, Block_Images, imagesBlock );
    outputBlock( out, Block_Widget, widgetBlock );
    outputBlock( out, Block_Slots, slotsBlock );
    outputBlock( out, Block_Tabstops, tabstopsBlock );
    outputBlock( out, Block_Actions, actionsBlock );
    outputBlock( out, Block_Menubar, menubarBlock );
    outputBlock( out, Block_Toolbars, toolbarsBlock );
    outputBlock( out, Block_Variables, variablesBlock );
    outputBlock( out, Block_Functions, functionsBlock );
    outputBlock( out, Block_Buddies, buddiesBlock );
    outputBlock( out, Block_Connections, connectionsBlock );
    out << (Q_UINT8) Block_End;

#ifdef COMPILE_STATS
#define X(block) \
	*totalPerBlock.insert( block, 0, FALSE ), \
	100.0 * totalPerBlock[block] / total

    qDebug( "Strings\t\t%6d\t%5.2f%%", X(Block_Strings) );
    qDebug( "Intro\t\t%6d\t%5.2f%%", X(Block_Intro) );
    qDebug( "Images\t\t%6d\t%5.2f%%", X(Block_Images) );
    qDebug( "Widget\t\t%6d\t%5.2f%%", X(Block_Widget) );
    qDebug( "Slots\t\t%6d\t%5.2f%%", X(Block_Slots) );
    qDebug( "Tabstops\t%6d\t%5.2f%%", X(Block_Tabstops) );
    qDebug( "Actions\t\t%6d\t%5.2f%%", X(Block_Actions) );
    qDebug( "Menubar\t\t%6d\t%5.2f%%", X(Block_Menubar) );
    qDebug( "Toolbars\t%6d\t%5.2f%%", X(Block_Toolbars) );
    qDebug( "Variables\t%6d\t%5.2f%%", X(Block_Variables) );
    qDebug( "Functions\t%6d\t%5.2f%%", X(Block_Functions) );
    qDebug( "Buddies\t%6d\t%5.2f%%", X(Block_Buddies) );
    qDebug( "Connections\t%6d\t%5.2f%%", X(Block_Connections) );
    qDebug( "\nTotal\t\t%6d", total );
#endif
}
