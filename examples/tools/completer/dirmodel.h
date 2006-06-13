/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DIRMODEL_H
#define DIRMODEL_H

#include <QDirModel>

// With a QDirModel, set on a view, you will see "Program Files" in the view
// But with this model, you will see "C:\Program Files" in the view.
// We acheive this, by having the data() return the entire file path for
// the display role. Note that the Qt::EditRole over which the QCompleter
// looks for matches is left unchanged
class DirModel : public QDirModel
{
public:
    DirModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif
