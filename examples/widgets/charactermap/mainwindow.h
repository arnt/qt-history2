#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QClipboard;
class QComboBox;
class QLineEdit;
class QWidgetView;
class CharView;
class CharWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void findStyles();
    void insertCharacter(const QString &character);
    void updateClipboard();

private:
    void findFonts();

    CharWidget *characterWidget;
    QClipboard *clipboard;
    QComboBox *fontCombo;
    QComboBox *styleCombo;
    QLineEdit *lineEdit;
    CharView *view;
};

#endif
