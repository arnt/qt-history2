/****************************************************************************
** $Id: //depot/qt/main/src/tools/qxml.cpp#1 $
**
** Implementation of QXML classes
**
** Created : 990128
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qxml.h"

#include <ctype.h>

/*********************************************
 *
 * QXMLSimpleParser
 *
 *********************************************/

QXMLSimpleParser::QXMLSimpleParser()
{
}

QXMLSimpleParser::~QXMLSimpleParser()
{
}

/*!
  Returns -1 if everything went fine or the position in the text
  where the error was detected. If no consumer is passed to this
  function then the parser checks just wether it can scan
  the text.
*/

int QXMLSimpleParser::parse( const QString &text, QXMLConsumer* consumer )
{
  int len = text.length();
  int pos = 0;
  int start = 0;
  QString attrib;
  bool preclosed;
  QString tag;

 Node1: // accepts
  if ( pos == len )
    goto Ok;
  else if ( text[pos] == '<' )
  {
    pos++;
    goto Node2;
  }
  else
  {
    start = pos++;
    goto Node20;
  }
 Node2: // Tag
  if ( pos + 3 <= len && text[pos] == '!' && text[pos+1] == '-' && text[pos+2] == '-' )
  {
    pos += 3;
    goto Node30;
  }
  if ( pos == len )
    goto Failed;

  preclosed = false;
  tag = QString::null;

  if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node2;
  }
  if ( text[pos] == '/' )
  {
    preclosed = true;
    pos++;
    goto Node3;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node4;
  }
  else
    goto Failed;
 Node3:
  if ( pos == len )
    goto Failed;
  if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node3;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node4;
  }
  else
    goto Failed;
 Node4:
  if ( pos == len )
    goto Failed;
  else if ( isalpha( text[pos] ) )
  {
    ++pos;
    goto Node4;
  }
  else if ( text[pos] == '>' )
  {
    if ( consumer )
    {
      if ( preclosed )
      {
	if ( !consumer->tagEnd( text.mid( start, pos - start ) ) )
	  return pos;
      }
      else if ( !consumer->tagStart( text.mid( start, pos - start ) ) )
	return pos;
    }
    pos++;
    goto Node15;
  }
  else if ( text[pos] == '/' )
  {
    if ( preclosed )
      goto Failed;

    if ( consumer )
    {
      if ( !consumer->tagStart( text.mid( start, pos - start ) ) )
	return pos;
      if ( !consumer->tagEnd( text.mid( start, pos - start ) ) )
	return pos;
    }

    pos++;
    goto Node14;
  }
  else if ( text[pos].isSpace() )
  {
    tag = text.mid( start, pos - start );
    pos++;
    goto Node5;
  }
  else
    goto Failed;
 Node5:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node5;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node6;
  }
  else if ( text[pos] == '>' )
  {
    if ( consumer )
    {
      if ( preclosed )
      {
	if ( !consumer->tagEnd( tag ) )
	  return pos;
      }
      else if ( !consumer->tagStart( tag ) )
	return pos;
    }

    pos++;
    goto Node15;
  }
  else if ( text[pos] == '/' )
  {
    if ( preclosed )
      goto Failed;

    if ( consumer )
    {
      if ( !consumer->tagStart( tag ) )
	return pos;
      if ( !consumer->tagEnd( tag ) )
	return pos;
    }
    pos++;
    goto Node14;
  }
  else
    goto Failed;
 Node6:
  if ( pos == len )
    goto Failed;

  if ( consumer )
    if ( !consumer->tagStart( tag ) )
      return pos;

  else if ( text[pos].isSpace() )
  {
    attrib = text.mid( start, pos - start );
    ++pos;
    goto Node7;
  }
  else if ( isalpha( text[pos] ) )
  {
    ++pos;
    goto Node6;
  }
  else if ( text[pos] == '=' )
  {
    attrib = text.mid( start, pos - start );
    ++pos;
    goto Node8;
  }
  else
    goto Failed;
 Node7:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node7;
  }
  else if ( text[pos] == '=' )
  {
    ++pos;
    goto Node8;
  }
  else
    goto Failed;
 Node8:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node10;
  }
  else if ( text[pos] == '"' )
  {
    ++pos;
    goto Node9;
  }
  else
    goto Failed;
 Node9:
  if ( pos == len )
    goto Failed;
  start = pos;
  if ( text[pos] == '"' )
  {
    if ( consumer )
      if ( !consumer->attrib( attrib, text.mid( start, pos - start ) ) )
	return pos;

    pos++;
    goto Node12;
  }
  else
  {
    ++pos;
    goto Node11;
  }
 Node10:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node10;
  }
  else if ( text[pos] == '"' )
  {
    ++pos;
    goto Node9;
  }
  else
    goto Failed;
 Node11:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '"' )
  {
    if ( consumer )
      if ( !consumer->attrib( attrib, text.mid( start, pos - start ) ) )
	return pos;

    pos++;
    goto Node12;
  }
  else
  {
    ++pos;
    goto Node11;
  }
 Node12:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node13;
  }
  else if ( text[pos] == '/' )
  {
    if ( preclosed )
      goto Failed;

    if ( consumer )
      if ( !consumer->tagEnd( tag ) )
	return pos;

    ++pos;
    goto Node14;
  }
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else
    goto Failed;
 Node13:
  if ( pos == len )
    goto Failed;
  else if ( text[pos].isSpace() )
  {
    ++pos;
    goto Node13;
  }
  else if ( text[pos] == '/' )
  {
    if ( preclosed )
      goto Failed;

    if ( consumer )
      if ( !consumer->tagEnd( tag ) )
	return pos;

    ++pos;
    goto Node14;
  }
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else if ( isalpha( text[pos] ) )
  {
    start = pos++;
    goto Node6;
  }
  else
    goto Failed;
 Node14:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node15;
  }
  else
    goto Failed;
 Node15: // accepts
  if ( pos == len )
    goto Ok;
  goto Node1;
 Node20: // Text
  if ( pos == len )
  {
    if ( pos != start && consumer )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	if ( !consumer->text( tmp ) )
	  return pos;
    }
    goto Ok;
  }
  else if ( pos + 7 < len && text[pos] == '[' && text[pos+1] == 'C' && text[pos+2] == 'D' && text[pos+3] == 'A' &&
	    text[pos+4] == 'T' && text[pos+5] == 'A' && text[pos+6] == '[' )
  {
    if ( pos != start && consumer )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	if ( !consumer->text( tmp ) )
	  return pos;
    }

    pos += 7;
    start = pos;
    while( pos + 2 <= len && ( text[pos] != ']' || text[pos+1] != ']' ) )
      ++pos;
    if ( pos + 2 > len )
      goto Failed;

    if ( pos != start && consumer )
    {
      QString tmp = text.mid( start, pos - start );
      if ( !tmp.isEmpty() )
	if ( !consumer->text( tmp ) )
	  return pos;
    }

    pos += 2;
    start = pos;
    goto Node20;
  }
  else if ( text[pos] == '&' )
  {
    if ( pos != start && consumer )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	if ( !consumer->text( tmp ) )
	  return pos;
    }

    ++pos;
    start = pos;

    while( pos < len && text[pos] != ';' )
      ++pos;
    if ( pos == len )
      goto Failed;

    if ( consumer )
    {
      QString entity = text.mid( start, pos - start );
      if ( entity.isEmpty() )
	goto Failed;
      QString value;
      if ( entity == "amp" )
	value = "&";
      else if ( entity == "lt" )
	value = "<";
      else if ( entity == "gt" )
	value = ">";
      else if ( entity == "quot" )
	value = "\"";
      else if ( entity == "apos" )
	value = "'";
      else if ( entity.length() >= 2 && entity[pos] == '#' && entity[pos+1] == 'x' )
      {
	if ( entity.length() > 6 )
	  goto Failed;
	QString tmp = entity.mid( 2 );
	bool ok;
	uint i = tmp.toUInt( &ok, 16 );
	if ( !ok )
	  goto Failed;
	value = QString( QChar( i ) );
      }
      else if ( entity[pos] == '#' )
      {
	if ( entity.length() > 6 )
	  goto Failed;
	QString tmp = entity.mid( 1 );
	bool ok;
	uint i = tmp.toUInt( &ok );
	if ( !ok )
	  goto Failed;
	value = QString( QChar( i ) );
      }

      if ( !consumer->entity( entity, value ) )
	return pos;
    }
    ++pos;
    start = pos;
    goto Node20;
  }
  else if ( text[pos] == '<' )
  {
    if ( pos != start && consumer )
    {
      QString tmp = text.mid( start, pos - start );
      tmp = tmp.simplifyWhiteSpace();
      if ( !tmp.isEmpty() )
	if ( !consumer->text( tmp ) )
	  return pos;
    }
    ++pos;
    goto Node2;
  }
  ++pos;
  goto Node20;
 Node30: // Comments
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node31;
  }
  else 
  {
    ++pos;
    goto Node30;
  }
 Node31:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node32;
  }
  else 
  {
    ++pos;
    goto Node30;
  }
 Node32:
  if ( pos == len )
    goto Failed;
  else if ( text[pos] == '>' )
  {
    ++pos;
    goto Node1;
  }
  else if ( text[pos] == '-' )
  {
    ++pos;
    goto Node32;
  }
  ++pos;
  goto Node30;

 Ok:
  if ( consumer )
    if ( !consumer->finished() )
      return pos;
  return -1;
 Failed:
  if ( consumer )
    consumer->parseError( pos );
  return pos;
}
