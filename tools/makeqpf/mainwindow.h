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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "qpf2.h"

class QListWidgetItem;

class MainWindow : public QMainWindow, Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString &customFont);

private Q_SLOTS:
    void on_actionAdd_Custom_Font_triggered();
    void on_selectAll_clicked();
    void on_deselectAll_clicked();
    void on_invertSelection_clicked();
    void fontChanged();
    void on_browsePath_clicked();
    void on_browseSampleFile_clicked();
    void on_generate_clicked();
    void on_sampleFile_editingFinished();

private:
    void populateCharacterRanges();
    void addCustomFont(const QString &fontFile);

private:
    QList<QPF::CharacterRange> sampleFileRanges;
};

#endif // MAINWINDOW_H
