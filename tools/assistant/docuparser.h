/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Assistant.
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
#ifndef DOCUPARSER_H
#define DOCUPARSER_H

#include <qxml.h>
#include <qptrlist.h>

class QString;

struct ContentItem {
    ContentItem( const QString &t, const QString &r, int d )
	: title( t ), reference( r ), depth( d ) {}
    QString title;
    QString reference;
    int depth;
};

struct IndexItem {
    IndexItem( const QString &k, const QString &r )
	: keyword( k ), reference( r ) {}
    QString keyword;
    QString reference;
};

enum States{
    StateInit,
    StateContent,
    StateSect,
    StateKeyword
};

class DocuParser : public QXmlDefaultHandler
{
public:
    DocuParser();
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool characters( const QString & );
    bool fatalError( const QXmlParseException& exception );
    QString errorProtocol() const;

    QPtrList<ContentItem>& getContentItems();
    QPtrList<IndexItem>& getIndexItems();
    QString getCategory() const;
    QString getDocumentationTitle() const;

private:
    QString category, contentRef, indexRef, errorProt;
    QString docTitle, title;
    int depth;
    States state;
    QPtrList<ContentItem> contentList;
    QPtrList<IndexItem> indexList;
};

#endif //DOCUPARSER_H
