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

#ifndef LISTBOX_H
#define LISTBOX_H


class QSpinBox;
class QListBox;
class QButtonGroup;

#include <qwidget.h>


class ListBoxDemo: public QWidget
{
    Q_OBJECT
public:
    ListBoxDemo();
    ~ListBoxDemo();

private slots:
    void setNumRows();
    void setNumCols();
    void setRowsByHeight();
    void setColsByWidth();
    void setVariableWidth( bool );
    void setVariableHeight( bool );
    void setMultiSelection( bool );
    void sortAscending();
    void sortDescending();
    
private:
    QListBox * l;
    QSpinBox * columns;
    QSpinBox * rows;
    QButtonGroup * bg;
};


#endif
