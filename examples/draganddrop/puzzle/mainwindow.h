#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPixmap>
#include <QMainWindow>

class PiecesList;
class PuzzleWidget;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

public slots:
    void openImage(const QString &path = QString());
    void setupPuzzle();

private slots:
    void setCompleted();

private:
    void setupMenus();
    void setupWidgets();

    QPixmap puzzleImage;
    PiecesList *piecesList;
    PuzzleWidget *puzzleWidget;
};

#endif
