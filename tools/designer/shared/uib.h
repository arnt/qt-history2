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

#ifndef UIB_H
#define UIB_H

#include <qdatastream.h>

const Q_UINT32 UibMagic = 0xb77c61d8;

enum BlockTag { Block_End = '$', Block_Actions = 'A', Block_Buddies = 'B',
		Block_Connections = 'C', Block_Functions = 'F',
		Block_Images = 'G', Block_Intro = 'I', Block_Menubar = 'M',
		Block_Slots = 'S', Block_Strings = 'Z', Block_Tabstops = 'T',
		Block_Toolbars = 'O', Block_Variables = 'V',
		Block_Widget = 'W' };

enum ObjectTag { Object_End = '$', Object_Action = 'A',
		 Object_ActionGroup = 'G', Object_ActionRef = 'F',
		 Object_Attribute = 'B', Object_GridCell = 'D',
		 Object_Item = 'I', Object_PaletteProperty = 'P',
		 Object_StringProperty = 'Z', Object_VariantProperty = 'V',
		 Object_Separator = 'S', Object_Spacer = 'Y',
		 Object_SubLayout = 'L', Object_SubWidget = 'W',
		 Object_TableColumn = 'C', Object_TableRow = 'R' };

enum PaletteTag { Palette_End = '$', Palette_Active = 'A',
		  Palette_Inactive = 'I', Palette_Disabled = 'D',
		  Palette_Color = 'C', Palette_Pixmap = 'P' };

enum IntroFlag { Intro_Pixmapinproject = 0x1 };

enum ConnectionFlag { Connection_Language = 0x1, Connection_Sender = 0x2,
		      Connection_Signal = 0x4, Connection_Receiver = 0x8,
		      Connection_Slot = 0x10 };

class UibStrTable
{
public:
    UibStrTable();

    inline int insertCString( const char *cstr );
    inline int insertString( const QString& str );
    inline void readBlock( QDataStream& in, int size );

    inline const char *asCString( int offset ) const;
    inline QString asString( int offset ) const;
    inline QByteArray block() const;

private:
    QCString table;
    QDataStream out;
    int start;
};

/*
  uic uses insertCString(), insertString(), and block();
  QWidgetFactory uses readBlock(), asCString(), and asString(). By
  implementing these functions inline, we ensure that the binaries
  don't contain needless code.
*/

inline int UibStrTable::insertCString( const char *cstr )
{
    if ( cstr == 0 || cstr[0] == 0 ) {
	return 0;
    } else {
	int nextPos = table.size();

	int len = strlen( cstr );
	for ( int i = 0; i < nextPos - len; i++ ) {
	    if ( memcmp(table.data() + i, cstr, len + 1) == 0 )
		return i;
	}
	for ( int i = 0; i < len + 1; i++ )
	    out << (Q_UINT8) cstr[i];
	return nextPos;
    }
}

inline int UibStrTable::insertString( const QString& str )
{
    if ( str.contains('\0') || str[0] == 0x7f ) {
	int nextPos = table.size();
	out << (Q_UINT8) 0x7f;
	out << str;
	return nextPos;
    } else {
	return insertCString( str.utf8() );
    }
}

inline void UibStrTable::readBlock( QDataStream& in, int size )
{
    table.resize( start + size );
    in.readRawBytes( table.data() + start, size );
}

inline QString UibStrTable::asString( int offset ) const
{
    if ( table[offset] == 0x7f ) {
	QDataStream in( table, IO_ReadOnly );
	in.device()->at( offset + 1 );
	QString str;
	in >> str;
	return str;
    } else {
	return QString::fromUtf8( asCString(offset) );
    }
}

inline const char *UibStrTable::asCString( int offset ) const
{
    return table.data() + offset;
}

inline QByteArray UibStrTable::block() const
{
    QByteArray block;
    block.duplicate( table.data() + start, table.size() - start );
    return block;
}

#endif
