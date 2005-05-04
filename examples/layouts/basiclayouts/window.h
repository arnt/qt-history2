#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QAction;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    void createMenu();
    void createHorizontalGroupBox();
    void createGridGroupBox();

    enum { NumGridRows = 3, NumButtons = 4 };

    QMenuBar *menuBar;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QTextEdit *smallEditor;
    QTextEdit *bigEditor;
    QLabel *labels[NumGridRows];
    QLineEdit *lineEdits[NumGridRows];
    QPushButton *buttons[NumButtons];
    QPushButton *okButton;
    QPushButton *cancelButton;

    QMenu *fileMenu;
    QAction *exitAction;
};

#endif
