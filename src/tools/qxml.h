/****************************************************************************
** $Id: //depot/qt/main/src/tools/qxml.h#1 $
**
** Definition of QXML classes
*
** Created : 980128
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
#ifndef QXML_H
#define QXML_H

#include "qstring.h"

class QXMLConsumer
{
public:
  QXMLConsumer() { }
  virtual ~QXMLConsumer() { }

  virtual bool tagStart( const QString& name ) = 0;
  virtual bool tagEnd( const QString& name ) = 0;
  virtual bool attrib( const QString& name, const QString& value ) = 0;
  virtual bool text( const QString& text ) = 0;
  virtual bool entity( const QString& name, const QString& value ) = 0;
  virtual void parseError( int pos ) = 0;
  virtual bool finished() = 0;
};

class QXMLSimpleParserPrivate;

class QXMLSimpleParser
{
public:
  QXMLSimpleParser();
  virtual ~QXMLSimpleParser();

  int parse( const QString& _text, QXMLConsumer* = 0 );

private:
  QXMLSimpleParserPrivate* d;
};

class QXMLValidatingParserPrivate;

class QXMLValidatingParser
{
public:
  QXMLValidatingParser() { d = 0; };
  virtual ~QXMLValidatingParser() { };

  int parse( const QString&, QXMLConsumer* ) { ASSERT( 0 ); return 0; }

private:
  QXMLValidatingParserPrivate* d;
};

#endif
