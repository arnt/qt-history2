#ifndef WINDOW_H
#define WINDOW_H

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

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();

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
