/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

    bool clicked( const QString &link );
    QWidget *parentWidget() const;

signals:
    void linkClicked( const QString &link );

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
