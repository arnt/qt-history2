#ifndef QSTRINGDATA_H
#define QSTRINGDATA_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifdef QT_NO_UNICODETABLES
# include <ctype.h>
#endif

class QUnicodeTables {
public:
    static const Q_UINT8 * const unicode_info[];
#ifndef QT_NO_UNICODETABLES
    static const Q_UINT16 decomposition_map[];
    static const Q_UINT16 * const decomposition_info[];
    static const Q_UINT16 ligature_map[];
    static const Q_UINT16 * const ligature_info[];
    static const Q_UINT8 * const direction_info[];
    static const Q_UINT8 * const combining_info[];
    static const Q_UINT16 * const case_info[];
    static const Q_INT8 * const decimal_info[];
    static const Q_UINT16 symmetricPairs[];
    static const int symmetricPairsSize;
#endif
    static const unsigned char otherScripts[];
    static const unsigned char indicScripts[];
    static const unsigned char scriptTable[];
    enum { SCRIPTS_INDIC = 0x7e };
};


inline QChar::Category category( const QChar &c )
{
#ifdef QT_NO_UNICODETABLES
    if ( c.unicode() > 0xff ) return QChar::Letter_Uppercase; //#######
#endif // QT_NO_UNICODETABLES
    return (QChar::Category)(QUnicodeTables::unicode_info[c.row()][c.cell()]);
}

inline QChar lower( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    uchar row = c.row();
    uchar cell = c.cell();
    const Q_UINT16 * const ci = QUnicodeTables::case_info[row];
    if ( QUnicodeTables::unicode_info[row][cell] != QChar::Letter_Uppercase || !ci )
	return c;
    Q_UINT16 lower = ci[cell];
    if ( lower == 0 )
	return c;
    return lower;
#else
    if ( c.row() )
	return c;
    return QChar( tolower((uchar) c.latin1()) );
#endif
}

inline QChar upper( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    uchar row = c.row();
    uchar cell = c.cell();
    const Q_UINT16 * const ci = QUnicodeTables::case_info[row];
    if ( QUnicodeTables::unicode_info[row][cell] != QChar::Letter_Lowercase || !ci )
	return c;
    Q_UINT16 upper = ci[cell];
    if ( upper == 0 )
	return c;
    return upper;
#else
    if ( c.row() )
	return c;
    return QChar( toupper((uchar) c.latin1()) );
#endif
}

inline QChar::Direction direction( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT8 * const rowp = QUnicodeTables::direction_info[c.row()];
    return (QChar::Direction) ( *(rowp+c.cell()) & 0x1f );
#else
    Q_UNUSED(c);
    return QChar::DirL;
#endif
}

inline bool mirrored( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT8 * const rowp = QUnicodeTables::direction_info[c.row()];
    return *(rowp+c.cell())>128;
#else
    Q_UNUSED(c);
    return FALSE;
#endif
}


inline QChar mirroredChar( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    if(!::mirrored( ch ))
	return ch;

    int i;
    int c = ch.unicode();
    for (i = 0; i < QUnicodeTables::symmetricPairsSize; i ++) {
	if (QUnicodeTables::symmetricPairs[i] == c)
	    return QUnicodeTables::symmetricPairs[(i%2) ? (i-1) : (i+1)];
    }
#endif
    return ch;
}

inline QChar::Joining joining( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT8 * const rowp = QUnicodeTables::direction_info[ch.row()];
    return (QChar::Joining) ((*(rowp+ch.cell()) >> 5) &0x3);
#else
    Q_UNUSED(ch);
    return QChar::OtherJoining;
#endif
}

inline bool isMark( const QChar &ch )
{
    QChar::Category c = ::category( ch );
    return c >= QChar::Mark_NonSpacing && c <= QChar::Mark_Enclosing;
}

inline unsigned char combiningClass( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT8 * const rowp = QUnicodeTables::combining_info[ch.row()];
    return *(rowp+ch.cell());
#else
    Q_UNUSED(ch);
    return 0;
#endif
}

inline bool isSpace( const QChar &ch )
{
    if( ch.unicode() >= 9 && ch.unicode() <=13 ) return TRUE;
    QChar::Category c = ::category( ch );
    return c >= QChar::Separator_Space && c <= QChar::Separator_Paragraph;
}

inline int scriptForChar( ushort uc )
{
    unsigned char script = QUnicodeTables::scriptTable[(uc>>8)];
    if ( script >= QUnicodeTables::SCRIPTS_INDIC ) {
	if ( script == QUnicodeTables::SCRIPTS_INDIC ) {
	    script = QUnicodeTables::indicScripts[ (uc-0x0900)>>7 ];
	} else {
	    // 0x80 + SCRIPTS_xx
	    unsigned char index = script-0x80;
	    unsigned char cell = uc &0xff;
	    while( QUnicodeTables::otherScripts[index++] < cell )
		index++;
	    script = QUnicodeTables::otherScripts[index];
	}
    }
    return script;
}

#ifdef Q_WS_X11
#define SCRIPT_FOR_CHAR( script, c ) 	\
do { 						\
    unsigned short _uc = (c).unicode(); 		\
    if ( _uc < 0x100 ) {				\
	script = QFont::Latin;		\
    } else { 					\
        script = (QFont::Script)scriptForChar( _uc ); 	\
    } 						\
} while( FALSE )
#else
#define SCRIPT_FOR_CHAR( script, c ) \
    script = (QFont::Script)scriptForChar( (c).unicode() )
#endif

#endif
