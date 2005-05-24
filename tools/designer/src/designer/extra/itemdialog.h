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

#ifndef ITEMDIALOG_H
#define ITEMDIALOG_H

#include <QtGui/QDialog>
class Item;

class ItemDialog : public QDialog
{
    Q_OBJECT
public:
    ItemDialog(QWidget *parent, const Item *item);

};

#endif
