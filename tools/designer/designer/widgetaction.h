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

#ifndef WIDGETACTION_H
#define WIDGETACTION_H

#include <qaction.h>

class WidgetAction : public QAction
{
    Q_OBJECT

public:
    WidgetAction( const QString &grp, QObject* parent,
		  const char* name = 0, bool toggle = FALSE  )
	: QAction( parent, name, toggle ) { init( grp ); }
    WidgetAction( const QString &grp, const QString& text,
		  const QIconSet& icon, const QString& menuText, QKeySequence accel,
		  QObject* parent, const char* name = 0, bool toggle = FALSE )
	: QAction( text, icon, menuText, accel, parent, name, toggle ) { init( grp ); }
    WidgetAction( const QString &grp, const QString& text,
		  const QString& menuText, QKeySequence accel, QObject* parent,
		  const char* name = 0, bool toggle = FALSE )
	: QAction( text, menuText, accel, parent, name, toggle ) { init( grp ); }
    ~WidgetAction();

#if !defined(Q_NO_USING_KEYWORD)
    using QAction::addedTo;
#endif
    void addedTo( QWidget *, QWidget * );

    QString group() const { return grp; }

private:
    void init( const QString &g );

private:
    QString grp;

};

#endif
