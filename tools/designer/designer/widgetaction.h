#ifndef WIDGETACTION_H
#define WIDGETACTION_H

#include <qaction.h>

class WidgetAction : public QAction
{
    Q_OBJECT

public:
    WidgetAction( QObject* parent, const char* name = 0, bool toggle = FALSE  )
	: QAction( parent, name, toggle ) {}
    WidgetAction( const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel,
		  QObject* parent, const char* name = 0, bool toggle = FALSE )
	: QAction( text, icon, menuText, accel, parent, name, toggle ) {}
    WidgetAction( const QString& text, const QString& menuText, QKeySequence accel, QObject* parent,
		  const char* name = 0, bool toggle = FALSE )
	: QAction( text, menuText, accel, parent, name, toggle ) {}

    void addedTo( QWidget *, QWidget * );

};

#endif
