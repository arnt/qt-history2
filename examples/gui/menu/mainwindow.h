#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void resizeEvent(QResizeEvent *event);

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
    void editFormatLeftAlign();
    void editFormatRightAlign();
    void editFormatJustify();
    void editFormatCenter();
    void editFormatSetLineSpacing();
    void editFormatSetParagraphSpacing();
    void helpAbout();
    void helpAboutQt();

private:
    QMenu *editMenu;
    QLabel *infoLabel;
    QLabel *usageLabel;
};

#endif
