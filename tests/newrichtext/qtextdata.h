#ifndef QTEXTDATA_H
#define QTEXTDATA_H

#include <qstring.h>
#include <qfont.h>

class QTextData {

    static const Q_UINT8 * const unicode_info[];
    static const Q_UINT8 * const direction_info[];
    static const Q_UINT8 * const combining_info[];
    static const Q_UINT16 symmetricPairs[];
    static const int symmetricPairsSize;

    static const unsigned char otherScripts[];
    static const unsigned char indicScripts[];
    static const unsigned char scriptTable[];
    enum { SCRIPTS_INDIC = 0x7e };

public:
    static QChar::Category category( const QChar &c )
    {
	return (QChar::Category)(unicode_info[c.row()][c.cell()]);
    }


    static QChar::Direction direction( const QChar &c )
    {
	const Q_UINT8 *rowp = direction_info[c.row()];
	if(!rowp) return QChar::DirL;
	return (QChar::Direction) ( *(rowp+c.cell()) & 0x1f );
    }

    static QChar::Joining joining( const QChar &c )
    {
	const Q_UINT8 *rowp = direction_info[c.row()];
	if ( !rowp )
	    return QChar::OtherJoining;
	return (QChar::Joining) ((*(rowp+c.cell()) >> 5) &0x3);
    }

    static bool mirrored( const QChar &c )
    {
	const Q_UINT8 *rowp = direction_info[c.row()];
	if ( !rowp )
	    return FALSE;
	return *(rowp+c.cell())>128;
    }

    static QChar mirroredChar( const QChar &ch )
    {
	if(!QTextData::mirrored( ch ))
	    return ch;

	int i;
	unsigned short c = ch.unicode();
	for (i = 0; i < symmetricPairsSize; i ++) {
	    if (symmetricPairs[i] == c)
		return symmetricPairs[(i%2) ? (i-1) : (i+1)];
	}
	return ch;
    }


    static QFont::Script scriptForChar( ushort uc )
    {
	unsigned char script = scriptTable[(uc>>8)];
	if ( script >= SCRIPTS_INDIC ) {
	    if ( script == SCRIPTS_INDIC ) {
		script = indicScripts[ (uc-0x0900)>>7 ];
	    } else {
		// 0x80 + SCRIPTS_xx
		unsigned char index = script-0x80;
		unsigned char cell = uc &0xff;
		while( otherScripts[index] < cell )
		    index += 2;
		script = otherScripts[index+1];
	    }
	}
	return (QFont::Script)script;
    }

#ifdef Q_WS_X11
#define SCRIPT_FOR_CHAR( script, c ) 	\
do { 						\
    unsigned short _uc = (c).unicode(); 		\
    if ( _uc < 0x100 ) {				\
	script = QFont::Latin;		\
    } else { 					\
        script = QTextData::scriptForChar( _uc ); 	\
    } 						\
} while( FALSE )

#else
#define SCRIPT_FOR_CHAR( script, c ) script = QTextData::scriptForChar( c )
#endif



};


#endif
