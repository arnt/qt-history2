/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef WHATSTHIS_H
#define WHATSTHIS_H

#include <qwhatsthis.h> 

class QHeader;
class QTable;

class WhatsThis : public QObject, public QWhatsThis
{
    Q_OBJECT
public:
    WhatsThis( QWidget *w, QWidget *watch = 0 );

    bool clicked( const QString &href );
    QWidget *parentWidget() const;

signals:
    void linkClicked( const QString &href );

private:
    QWidget *widget;
};

class HeaderWhatsThis : public WhatsThis
{
public: 
    HeaderWhatsThis( QHeader *h );

    QString text( const QPoint &p );
};

class TableWhatsThis : public WhatsThis
{
public: 
    TableWhatsThis( QTable *t );

    QString text( const QPoint &p );
};

#endif
