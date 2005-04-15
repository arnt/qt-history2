#ifndef WINDOW_H
#define WINDOW_H

#include <QIcon>
#include <QList>
#include <QMainWindow>
#include <QPixmap>

class QActionGroup;
class QGroupBox;
class QMenu;
class QPushButton;
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
    void resetImages();

private:
    void createMenus();
    void createPreviewGroupBox();
    void createImagesGroupBox();
    void createIconSizeGroupBox();
    void checkCurrentStyle();

    enum { PixmapRole = Qt::UserRole + 1 };

    QWidget *centralWidget;

    QGroupBox *previewGroupBox;
    IconPreviewArea *previewArea;

    QGroupBox *imagesGroupBox;
    QTableWidget *imagesTable;
    QPushButton *addButton;
    QPushButton *resetButton;

    QGroupBox *iconSizeGroupBox;
    QRadioButton *smallRadioButton;
    QRadioButton *largeRadioButton;
    QRadioButton *toolBarRadioButton;
    QRadioButton *listViewRadioButton;
    QRadioButton *iconViewRadioButton;
    QRadioButton *otherRadioButton;
    IconSizeSpinBox *otherSpinBox;

    QMenu *styleMenu;
    QMenu *helpMenu;
    QActionGroup *styleActionGroup;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
