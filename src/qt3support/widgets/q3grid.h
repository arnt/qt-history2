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


#ifndef Q3GRID_H
#define Q3GRID_H

#include "q3frame.h"

class QGridLayout;

class Q_COMPAT_EXPORT Q3Grid : public Q3Frame
{
    Q_OBJECT
public:
    Q3Grid(int n, QWidget* parent=0, const char* name=0, Qt::WFlags f = 0);
    Q3Grid(int n, Qt::Orientation orient, QWidget* parent=0, const char* name=0,
	   Qt::WFlags f = 0);

    void setSpacing(int);
    QSize sizeHint() const;

    typedef Qt::Orientation Direction;

protected:
    void frameChanged();

private:
    Q_DISABLE_COPY(Q3Grid)
};


#endif // Q3GRID_H
