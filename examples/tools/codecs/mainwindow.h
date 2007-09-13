/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#include <QList>
#include <QMainWindow>

QT_DECLARE_CLASS(QAction)
QT_DECLARE_CLASS(QMenu)
QT_DECLARE_CLASS(QTextCodec)
QT_DECLARE_CLASS(QTextEdit)
class PreviewForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void save();
    void about();
    void aboutToShowSaveAsMenu();

private:
    void findCodecs();
    void createActions();
    void createMenus();

    QTextEdit *textEdit;
    PreviewForm *previewForm;
    QList<QTextCodec *> codecs;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *saveAsMenu;
    QAction *openAct;
    QList<QAction *> saveAsActs;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
