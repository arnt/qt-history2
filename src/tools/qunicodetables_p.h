#ifndef QSTRINGDATA_H
#define QSTRINGDATA_H

#include "qstring.h"

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
    if ( QUnicodeTables::unicode_info[row][cell] != QChar::Letter_Uppercase )
	return c;
    Q_UINT16 lower = *( QUnicodeTables::case_info[row] + cell );
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
    if ( QUnicodeTables::unicode_info[row][cell] != QChar::Letter_Lowercase )
	return c;
    Q_UINT16 upper = *(QUnicodeTables::case_info[row]+cell);
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
    const Q_UINT8 *rowp = QUnicodeTables::direction_info[c.row()];
    return (QChar::Direction) ( *(rowp+c.cell()) & 0x1f );
#else
    Q_UNUSED(c);
    return QChar::DirL;
#endif
}

inline bool mirrored( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    const Q_UINT8 *rowp = QUnicodeTables::direction_info[c.row()];
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
    const Q_UINT8 *rowp = QUnicodeTables::direction_info[ch.row()];
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
    const Q_UINT8 *rowp = QUnicodeTables::combining_info[ch.row()];
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


#endif
