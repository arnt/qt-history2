#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

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

    QStringList recentFiles;
    QString curFile;

    QMenu *fileMenu;
    QMenu *recentFilesMenu;
    QMenu *helpMenu;
};

#endif
