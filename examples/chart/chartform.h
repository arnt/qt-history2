#ifndef CHARTFORM_H
#define CHARTFORM_H

#include "element.h"

#include <qmainwindow.h>
#include <qstringlist.h>


class CanvasView;

class QAction;
class QCanvas;
class QFont;
class QPrinter;
class QString;


class ChartForm: public QMainWindow
{
    Q_OBJECT
public:
    enum { MAX_ELEMENTS = 100 };
    enum { MAX_RECENTFILES = 9 }; // Must not exceed 9
    enum ChartType { PIE, VERTICAL_BAR, HORIZONTAL_BAR };
    enum AddValuesType { NO, YES, AS_PERCENTAGE };

    ChartForm( const QString& filename );
    ~ChartForm();

    int getChartType() { return chartType; }
    void setChanged( bool change = true ) { changed = change; }
    void drawElements();

    QPopupMenu *optionsMenu; // Why public? See canvasview.cpp

protected:
    void closeEvent( QCloseEvent *ce );

private slots:
    void fileNew();
    void fileOpen();
    void fileOpenRecent( int index );
    void fileSave();
    void fileSaveAs();
    void fileSaveAsPixmap();
    void filePrint();
    void fileQuit();
    void optionsSetData();
    void setChartType( QAction *action );
    void optionsSetFont();
    void optionsSetOptions();
    void helpHelp();
    void helpAbout();
    void helpAboutQt();
    void saveOptions();

private:
    void init();
    void load( const QString& filename );
    bool okToClear();
    void drawPieChart( const double scales[], double total, int count );
    void drawVerticalBarChart( const double scales[], double total, int count );
    void drawHorizontalBarChart( const double scales[], double total, int count );

    QString valueLabel( const QString& label, double value, double total );
    void updateRecentFiles( const QString& filename );
    void updateRecentFilesMenu();

    QPopupMenu *fileMenu;
    QAction *optionsPieChartAction;
    QAction *optionsHorizontalBarChartAction;
    QAction *optionsVerticalBarChartAction;

    QString fileName;
    QStringList recentFiles;
    QCanvas *canvas;
    CanvasView *canvasView;
    bool changed;
    ElementVector elements;
    QPrinter *printer;
    ChartType chartType;
    AddValuesType addValues;
    int decimalPlaces;
    QFont font;
};

#endif
