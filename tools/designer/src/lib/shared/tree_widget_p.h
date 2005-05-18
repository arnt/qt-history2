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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include "shared_global_p.h"
#include <QTreeWidget>

class QT_SHARED_EXPORT TreeWidget: public QTreeWidget
{
    Q_OBJECT
public:
    TreeWidget(QWidget *parent = 0);
    virtual ~TreeWidget();

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
};

#endif // TREEWIDGET_H
