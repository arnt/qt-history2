#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <qtabwidget.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qlistview.h>

struct DesignerOutputDock;
class QTextEdit;
class QListView;

class ErrorItem : public QListViewItem
{
public:
    enum Type { Error, Warning };

    ErrorItem( QListView *parent, QListViewItem *after, const QString &message, int line,
	       const QString &locationString, QObject *locationObject );

    void paintCell( QPainter *, const QColorGroup & cg,
		    int column, int width, int alignment );

    void setRead( bool b ) { read = b; repaint(); }

    QObject *location() const { return object; }
    int line() const { return text( 2 ).toInt(); }

private:
    QObject *object;
    Type type;
    bool read;

};

class OutputWindow : public QTabWidget
{
    Q_OBJECT

public:
    OutputWindow( QWidget *parent );
    ~OutputWindow();

    void setErrorMessages( const QStringList &errors, const QValueList<int> &lines,
			   bool clear, const QStringList &locations,
			   const QObjectList &locationObjects );
    void appendDebug( const QString& );
    void clearErrorMessages();
    void clearDebug();
    void showDebugTab();

    DesignerOutputDock *iFace();

    void shuttingDown();

private slots:
    void currentErrorChanged( QListViewItem *i );

private:
    void setupError();
    void setupDebug();

    QTextEdit *debugView;
    QListView *errorView;

    DesignerOutputDock *iface;

    QtMsgHandler oldMsgHandler;
};

#endif
