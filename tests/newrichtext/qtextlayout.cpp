#include "qtextlayout.h"

#include "scriptengine.h"
#include "scriptenginelatin.h"
#include "scriptenginearabic.h"

#include <stdlib.h>

#include "bidi.cpp"


ScriptItemArray::~ScriptItemArray()
{
    free( d );
}

void ScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (ScriptItemArrayPrivate *)realloc( d, sizeof( ScriptItemArrayPrivate ) +
		 sizeof( ScriptItem ) * alloc );
    d->alloc = alloc;
}


CharAttributesArray::~CharAttributesArray()
{
    free( d );
}


ShapedItem::ShapedItem()
{
    d = 0;
}

ShapedItem::~ShapedItem()
{
    // ####
    if ( d )
	delete ( (QString *)d );
}

ScriptEngine **scriptEngines = 0;


class TextLayoutQt : public TextLayout
{
public:

    void itemize( ScriptItemArray &items, const QRTString & ) const;
    void itemize( ScriptItemArray &items, const QString & ) const;

    void attributes( CharAttributesArray &attributes, const QString &string,
		     const ScriptItemArray &items, int item ) const;

    void shape( ShapedItem &shaped, const QRTString &string,
		const ScriptItemArray &items, int item ) const;
};


void TextLayoutQt::itemize( ScriptItemArray &items, const QRTString &string ) const
{
    if ( !items.d ) {
	int size = 0;//string.formats.numFormats() + 1;
	items.d = (ScriptItemArrayPrivate *)malloc( sizeof( ScriptItemArrayPrivate ) +
					      sizeof( ScriptItem ) * size );
	items.d->alloc = size;
	items.d->size = 0;
    }

    bidiItemize( string.str(), items, QChar::DirON, 0 ); //&string.formats );
}


void TextLayoutQt::itemize( ScriptItemArray &items, const QString &string ) const
{
    if ( !items.d ) {
	int size = 1;
	items.d = (ScriptItemArrayPrivate *)malloc( sizeof( ScriptItemArrayPrivate ) +
						    sizeof( ScriptItem ) * size );
	items.d->alloc = size;
	items.d->size = 0;
    }

    bidiItemize( string, items, QChar::DirON, 0 );
}


void TextLayoutQt::attributes( CharAttributesArray &attrs, const QString &string,
			       const ScriptItemArray &items, int item ) const
{
    const ScriptItem &si = items[item];
    int from = si.position;
    item++;
    int len = ( item < items.size() ? items[item].position : string.length() ) - from;


    attrs.d = (CharAttributesArrayPrivate *)realloc( attrs.d, sizeof(CharAttributes)*len );

    scriptEngines[si.analysis.script]->charAttributes( string, from, len, attrs.d->attributes );
}


void TextLayoutQt::shape( ShapedItem &shaped, const QRTString &string,
	    const ScriptItemArray &items, int item ) const
{
    const ScriptItem &si = items[item];
    int from = si.position;
    item++;
    int len = ( item < items.size() ? items[item].position : string.length() ) - from;

    QFont f;
    scriptEngines[si.analysis.script]->shape( f, string.str(), from, len, si.analysis, &shaped );
}


static TextLayout *_instance = 0;

const TextLayout *TextLayout::instance()
{
    if ( !_instance ) {
	_instance = new TextLayoutQt();

        if ( !scriptEngines ) {
	    scriptEngines = (ScriptEngine **) malloc( QFont::NScripts * sizeof( ScriptEngine * ) );
	    scriptEngines[0] = new ScriptEngineLatin;
	    for ( int i = 1; i < QFont::NScripts; i++ )
		scriptEngines[i] = scriptEngines[0];
	    scriptEngines[QFont::Arabic] = new ScriptEngineArabic;
	}
    }
    return _instance;
}


void TextLayout::bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder ) const
{
    ::bidiReorder(numRuns, levels, visualOrder );
}

