/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qwidget.h>
class QStyle;
class QListBox;
class QListBoxItem;
class QWidgetStack;

class GraphWidgetPrivate;

class GraphWidget : public QWidget 
{
    Q_OBJECT
public:
    GraphWidget( QWidget *parent=0, const char *name=0 );
    ~GraphWidget();

private slots:
    void shuffle();
    void setSpeed(int);
    
private:
    GraphWidgetPrivate* d;
};
