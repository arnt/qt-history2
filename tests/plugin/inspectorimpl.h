#ifndef INSPECTOR_H
#define INSPECTOR_H
#include "inspector.h"

#include <qdict.h>
#include <qlibrary.h>
#include <qlistview.h>

class QUnknownInterface;

class LibraryInspector : public LibraryInspectorBase
{ 
    Q_OBJECT

public:
    LibraryInspector( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~LibraryInspector();

public slots:
    void showLibrary( QListViewItem* );
    void selectPath();

private:
    void addInterface( QListViewItem *parent, QUnknownInterface *iface );

    QDict<QListViewItem> itemDict;
    QDict<QLibrary> libDict;
    QString path;
};

#endif // INSPECTOR_H
