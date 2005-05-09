/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QIcon>
#include <QList>
#include <QMainWindow>
#include <QPixmap>

class QAction;
class QActionGroup;
class QGroupBox;
class QMenu;
class QRadioButton;
class QTableWidget;
class IconPreviewArea;
class IconSizeSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void about();
    void changeStyle(bool checked);
    void changeSize();
    void changeIcon();
    void addImage();
    void removeAllImages();

private:
    void createPreviewGroupBox();
    void createImagesGroupBox();
    void createIconSizeGroupBox();
    void createActions();
    void createMenus();
    void createContextMenu();
    void checkCurrentStyle();

    QWidget *centralWidget;

    QGroupBox *previewGroupBox;
    IconPreviewArea *previewArea;

    QGroupBox *imagesGroupBox;
    QTableWidget *imagesTable;

    QGroupBox *iconSizeGroupBox;
    QRadioButton *smallRadioButton;
    QRadioButton *largeRadioButton;
    QRadioButton *toolBarRadioButton;
    QRadioButton *listViewRadioButton;
    QRadioButton *iconViewRadioButton;
    QRadioButton *otherRadioButton;
    IconSizeSpinBox *otherSpinBox;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QAction *addImageAct;
    QAction *removeAllImagesAct;
    QAction *exitAct;
    QAction *guessModeStateAct;
    QActionGroup *styleActionGroup;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
