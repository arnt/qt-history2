/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include <qtabwidget.h>
#include <qstringlist.h>
#include <qlist.h>
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

    void paintCell( QPainter *, const QPalette &pal, int column, int width, int alignment );

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

    void setErrorMessages( const QStringList &errors, const QList<uint> &lines,
			   bool clear, const QStringList &locations,
			   const QObjectList &locationObjects );
    void appendDebug( const QString& );
    void clearErrorMessages();
    void clearDebug();
    void showDebugTab();

    DesignerOutputDock *iFace();

    void shuttingDown();

    static QtMsgHandler oldMsgHandler;

private slots:
    void currentErrorChanged( QListViewItem *i );

private:
    void setupError();
    void setupDebug();

    QTextEdit *debugView;
    QListView *errorView;

    DesignerOutputDock *iface;

};

#endif
