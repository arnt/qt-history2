#ifndef EDITOR_H
#define EDITOR_H

#include <qmainwindow.h>

class QTextEdit;
class QAction;

class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor();
    ~Editor();

private:
    QTextEdit * editor;
    QAction * setredfont;

private slots:
    void setFontColor( QAction * );

};

#endif

