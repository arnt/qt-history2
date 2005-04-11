#ifndef IDEAPPLICATION_H
#define IDEAPPLICATION_H

#include <QApplication>

#include <abstractdndmanager.h>
#include <abstractimagecollection.h>
#include <abstractwidgetdatabase.h>
#include <abstractformeditor.h>
#include <abstractmetadatabase.h>
#include <abstractwidgetfactory.h>
#include <abstractformwindowcursor.h>
#include <abstractpropertyeditor.h>
#include <container.h>
#include <abstractformwindow.h>
#include <propertysheet.h>
#include <abstractformwindowmanager.h>
#include <abstractwidgetbox.h>

class IDEApplication: public QApplication
{
    Q_OBJECT
public:
    IDEApplication(int &argc, char *argv[]);
    virtual ~IDEApplication();

    static QDesignerFormEditorInterface *core();
};

#endif // IDEAPPLICATION_H
