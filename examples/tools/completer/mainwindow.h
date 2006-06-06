#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QComboBox;
class QProgressBar;
class QLabel;
class QLineEdit;
class QCompleter;
class QAbstractItemModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void changeMode(int);
    void updateCompletionModel();
    void changeCase(int);
    void about();

private:
    void createMenu();
    QAbstractItemModel *modelFromFile(const QString& fileName);

    QCompleter *completer;
    QLineEdit *lineEdit;
    QComboBox *comboBox;
    QComboBox *caseCombo;
    QComboBox *modelCombo;
    QLabel *progressLabel;
    QLabel *contentsLabel;
    QProgressBar *progressBar;
};

#endif // MAINWINDOW_H

