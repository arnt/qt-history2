/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   phraselv.h
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef PHRASELV_H
#define PHRASELV_H

#include <qlistview.h>

#include "phrase.h"

class QWhatsThis;

class PhraseLV;

#define NewPhrase PhraseLV::tr( "(New Phrase)" )

class PhraseLVI : public QListViewItem
{
public:
    enum { SourceTextShown, TargetTextShown, DefinitionText, SourceTextOriginal,
           TargetTextOriginal };

    PhraseLVI( PhraseLV *parent, const Phrase& phrase, int accelKey = 0 );

    virtual void setText( int column, const QString& text );
    virtual QString key( int column, bool ascending ) const;

    void setPhrase( const Phrase& phrase );
    Phrase phrase() const;
    int accelKey() const { return akey; }

private:
    QString makeKey( const QString& text ) const;

    int akey;
    QString sourceTextKey;
    QString targetTextKey;
};

class PhraseLV : public QListView
{
    Q_OBJECT
public:
    PhraseLV( QWidget *parent, const char *name );
    ~PhraseLV();

    virtual QSize sizeHint() const;

private:
    QWhatsThis *what;
};

#endif
