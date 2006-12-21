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

#ifndef VIEW_H
#define VIEW_H

#include <QtGui>
#include <QtSql>

class ImageItem;
class InformationWindow;

class View : public QGraphicsView
{
    Q_OBJECT

public:
    View(const QString &offices, const QString &images, QWidget *parent = 0);

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void updateImage(int id, const QString &fileName);

private:
    void addItems();
    InformationWindow* findWindow(int id);
    void showInformation(ImageItem *image);

    QGraphicsScene *scene;
    QList<InformationWindow *> informationWindows;
    QSqlRelationalTableModel *officeTable;
};

#endif
