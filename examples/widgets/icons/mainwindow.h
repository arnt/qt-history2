/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#include <QIcon>
#include <QList>
#include <QMainWindow>
#include <QPixmap>

QT_DECLARE_CLASS(QAction)
QT_DECLARE_CLASS(QActionGroup)
QT_DECLARE_CLASS(QGroupBox)
QT_DECLARE_CLASS(QMenu)
QT_DECLARE_CLASS(QRadioButton)
QT_DECLARE_CLASS(QTableWidget)
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
    void changeSize(bool checked = true);
    void changeIcon();
    void addImages();
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
    QRadioButton *tabBarRadioButton;
    QRadioButton *otherRadioButton;
    IconSizeSpinBox *otherSpinBox;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QAction *addImagesAct;
    QAction *removeAllImagesAct;
    QAction *exitAct;
    QAction *guessModeStateAct;
    QActionGroup *styleActionGroup;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
