#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QClipboard;
class QComboBox;
class QLineEdit;
class QWidgetView;
class CharacterView;
class CharacterWidget;

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

    CharacterView *view;
    CharacterWidget *characterWidget;
    QClipboard *clipboard;
    QComboBox *fontCombo;
    QComboBox *styleCombo;
    QLineEdit *lineEdit;
};

#endif
