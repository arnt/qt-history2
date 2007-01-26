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

#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include <QtSql>
#include <QtXml>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QSqlRelationalTableModel *albums, QDomDocument details,
           QFile *output, QWidget *parent = 0);

private slots:
    void revert();
    void submit();

private:
    int addNewAlbum(const QString &title, int artistId);
    int addNewArtist(const QString &name);
    void addTracks(int albumId, QStringList tracks);
    QDialogButtonBox *createButtons();
    QGroupBox *createInputWidgets();
    int findArtistId(const QString &artist);
    int generateAlbumId();
    int generateArtistId();
    void increaseAlbumCount(QModelIndex artistIndex);
    QModelIndex indexOfArtist(const QString &artist);

    QSqlRelationalTableModel *model;
    QDomDocument albumDetails;
    QFile *outputFile;

    QLineEdit *artistEditor;
    QLineEdit *titleEditor;
    QSpinBox *yearEditor;
    QLineEdit *tracksEditor;
};

#endif
