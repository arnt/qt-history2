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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDomDocument>
#include <QMainWindow>
#include <QModelIndex>

class QComboBox;
class QFile;
class QGroupBox;
class QLabel;
class QListWidget;
class QSqlRelationalTableModel;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &artistTable, const QString &albumTable,
               QFile *albumDetails, QWidget *parent = 0);

private slots:
    void about();
    void addAlbum();
    void changeArtist(int row);
    void deleteAlbum();
    void showAlbumDetails(QModelIndex index);
    void showArtistProfile(QModelIndex index);
    void updateHeader(QModelIndex, int, int);

private:
    void adjustHeader();
    QGroupBox *createAlbumGroupBox();
    QGroupBox *createArtistGroupBox();
    QGroupBox *createDetailsGroupBox();
    void createMenuBar();
    void decreaseAlbumCount(QModelIndex artistIndex);
    void getTrackList(QDomNode album);
    QModelIndex indexOfArtist(const QString &artist);
    void readAlbumData();
    void removeAlbumFromDatabase(QModelIndex album);
    void removeAlbumFromFile(int id);
    void showImageLabel();

    QTableView *albumView;
    QComboBox *artistView;
    QListWidget *trackList;

    QLabel *iconLabel;
    QLabel *imageLabel;
    QLabel *profileLabel;
    QLabel *titleLabel;

    QDomDocument albumData;
    QFile *file;
    QSqlRelationalTableModel *model;
};

#endif
