#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void about();

private:
    void createActions();
    void createMenus();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    void saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void updateRecentFileActions();

    QStringList recentFiles;
    QString curFile;

    QTextEdit *textEdit;
    QMenu *fileMenu;
    QMenu *recentFilesMenu;
    QMenu *helpMenu;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
};

#endif
