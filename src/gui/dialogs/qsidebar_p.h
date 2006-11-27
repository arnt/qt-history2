/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QSIDEBAR_H
#define QSIDEBAR_H

#include <qlistwidget.h>

class QFileSystemModel;

class Q_AUTOTEST_EXPORT QSidebar : public QListWidget
{
    Q_OBJECT

Q_SIGNALS:
    void goToUrl(const QUrl &url);

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
    };

    QSidebar(QFileSystemModel *model, const QList<QUrl> &newUrls, QWidget *parent = 0);
    ~QSidebar();

    QSize sizeHint() const;

    void setUrls(const QList<QUrl> &list);
    QList<QUrl> urls() const;
    void selectUrl(const QUrl &url);
    void addUrls(const QList<QUrl> &list, int row);

protected:
    void focusInEvent(QFocusEvent *event);

    QStringList mimeTypes() const;
    QMimeData *mimeData ( const QList<QListWidgetItem *> items ) const;
    bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
    void setUrl(const QModelIndex &index, const QUrl &url);

private Q_SLOTS:
    void clicked(const QModelIndex &index);
    void showContextMenu(const QPoint &position);
    void removeEntry();
    void layoutChanged();

private:
    void init();
    QFileSystemModel *fileSystemModel;
    QStringList watching;
};

#endif

