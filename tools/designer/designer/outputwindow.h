#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <qtabwidget.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class DesignerOutputDock;
class QTextEdit;
class QListView;

class OutputWindow : public QTabWidget
{
    Q_OBJECT

public:
    OutputWindow( QWidget *parent );

    void setErrorMessages( const QStringList &errors, const QValueList<int> &lines, bool clear = TRUE );

    static QTextEdit *debugView;
    static QListView *errorView;

    DesignerOutputDock *iFace();

private:
    void setupError();
    void setupDebug();

};

#endif
