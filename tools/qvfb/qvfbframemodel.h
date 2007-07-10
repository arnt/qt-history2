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

#ifndef QVFBFRAMEMODEL_H
#define QVFBFRAMEMODEL_H

#include <QAbstractListModel>
#include <QListView>

class QVFbFrameView : public QListView
{
    Q_OBJECT
public:
    QVFbFrameView(QWidget *parent = 0);

signals:
    void showImage(const QPixmap &);

private slots:
    void mouseEnter(const QModelIndex &);
};

class QVFbFrameModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QVFbFrameModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex & = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &, int) const;

    QPixmap image(int) const;
    void appendImage(const QImage &, const QString &);

    int maxFrames() const;
    void setMaxFrames(int);

    void clear();

private:
    QList<QPair<QPixmap, QString> > images;
    int mMaxFrames;
};

#endif // QVFBFRAMEMODEL_H
