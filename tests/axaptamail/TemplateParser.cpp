// TemplateParser.cpp : Implementation of CTemplateParser
#include "stdafx.h"
#include "Axaptamail.h"
#include "TemplateParser.h"

#include "qregexp.h"
#include <windows.h>

/////////////////////////////////////////////////////////////////////////////
// CTemplateParser


static QString BSTR2QString( BSTR src )
{
    QString tmp;

    for( unsigned int i = 0; i < SysStringLen( src ); i++ ) {
	QChar c( src[ i ] );
	tmp += c;
    }
    return tmp;
}

STDMETHODIMP CTemplateParser::addVariable(BSTR name, BSTR value)
{
    dictionary[ BSTR2QString( name ) ] = BSTR2QString( value );

    return S_OK;
}

STDMETHODIMP CTemplateParser::parseTemplate(BSTR inStream, BSTR *outStream)
{
    QString out;
    QString in = BSTR2QString( inStream );
    QString varName;
    int index( in.find( "%" ) );
    int lastIndex( 0 ), index2;
    int width( 0 );
    int widthPos;

    while( index != -1 ) {
	out += in.mid( lastIndex, index - lastIndex );
	index2 = in.find( "%", index + 1 );
	varName = in.mid( index + 1, index2 - index - 1 );

	if( ( index2 - index ) == 1 )
	    out += "%";
	else {
	    widthPos = varName.find( QRegExp( "\\.\\d+" ) );
	    if( widthPos != -1 ) {
		width = varName.mid( widthPos + 1 ).toInt();
		varName.truncate( widthPos );
	    }
	    out += dictionary[ varName ];
	    if( dictionary[ varName ].length() < width ) {
		for( int i = dictionary[ varName ].length(); i < width; i++ )
		    out += " ";
	    }
	}

	index = index2;
	lastIndex = index + 1;
	index = in.find( "%", lastIndex );
    }

    if( lastIndex < in.length() )
	out += in.mid( lastIndex );

    *outStream = ::SysAllocString( (TCHAR*)qt_winTchar( out, TRUE ) );
    return S_OK;
}

STDMETHODIMP CTemplateParser::dropDictionary()
{
    dictionary.clear();
    return S_OK;
}
