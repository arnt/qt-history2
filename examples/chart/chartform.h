#ifndef CHARTFORM_H
#define CHARTFORM_H

#include "globals.h"
#include "element.h"

#include <qmainwindow.h>

class QAction;
class QString;
class QCanvas;
class QCanvasView;
class QPrinter;


class ChartForm: public QMainWindow
{
    Q_OBJECT
public:
    ChartForm( const QString& filename );
    ~ChartForm();

protected:
    void closeEvent( QCloseEvent* );

private:
    void load( const QString& filename );
    bool okToQuit();
    void paintElements();

private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void fileQuit();
    void optionsSetData();
    void optionsSetFont();
    void optionsSetOptions();
    void helpHelp();
    void helpAbout();
    void helpAboutQt();

private:
    QString fileName;
    QCanvas *canvas;
    QCanvasView *canvasView;
    bool changed;
    ElementVector elements;
    QPrinter *printer;
};

#endif
