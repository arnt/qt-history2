/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OBJECTS_H
#define OBJECTS_H

#include <qwidget.h>

class QVBoxLayout;
class QSubWidget;

class QParentWidget : public QWidget
{
    Q_OBJECT
public:
    QParentWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    QSize sizeHint() const;

public slots:
    void createSubWidget( const QString &name );

    QSubWidget *subWidget( const QString &name );

private:
    QVBoxLayout *vbox;
};

class QSubWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )
public:
    QSubWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    void setLabel( const QString &text );
    QString label() const;

    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent *e );

private:
    QString lbl;
};

#endif // OBJECTS_H
