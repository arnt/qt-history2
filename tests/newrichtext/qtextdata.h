#ifndef QTEXTDATA_H
#define QTEXTDATA_H

#include <qstring.h>
#include "qfont.h"

#include <private/qunicodetables_p.h>

class QTextData {

    static const unsigned char otherScripts[];
    static const unsigned char indicScripts[];
    static const unsigned char scriptTable[];
    enum { SCRIPTS_INDIC = 0x7e };

public:

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
		while( otherScripts[index++] < cell )
		    index++;
		script = otherScripts[index];
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
