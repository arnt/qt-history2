#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void filePrint();
    void fileQuit();
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editFormat();
    void editFormatBold();
    void editFormatItalic();
    void editFormatLeftAlign();
    void editFormatRightAlign();
    void editFormatJustify();
    void editFormatCenter();
    void editFormatSetLineSpacing();
    void editFormatSetParagraphSpacing();
    void helpAbout();
    void helpAboutQt();

private:
    void createMenus();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *helpMenu;
    QLabel *infoLabel;
    QLabel *usageLabel;
};

#endif
