#include <qapplication.h>
#include <qlistview.h>
#include <qvboxwidget.h>
#include <qstackedwidget.h>
#include <qtoolbar.h>
#include "pimmodel.h"
#include "pimdelegate.h"
#include "pimeditor.h"
#include "pimviewer.h"
#include "pimsignalhub.h"

void addData(PimModel *model)
{
    PimEntry jdoe;
    jdoe.photo = QPixmap(":/images/questionmark.png");
    jdoe.firstName = "John";
    jdoe.lastName = "Doe";
    jdoe.middleName = "Schmo";
    jdoe.company = "Big Gray Corp.";
    jdoe.department = "Central Archive";
    jdoe.jobTitle = "Bureaucrat";
    model->appendEntry(jdoe);

    PimEntry mmonsen;
    mmonsen.photo = QPixmap(":/images/mmonsen.png");
    mmonsen.firstName = "Marius";
    mmonsen.lastName = "Monsen";
    mmonsen.middleName = "Bugge";
    mmonsen.company = "Trolltech";
    mmonsen.department = "Development";
    mmonsen.jobTitle = "Software Developer";
    model->appendEntry(mmonsen);

    PimEntry espenr;
    espenr.photo = QPixmap(":/images/espenr.png");
    espenr.firstName = "Espen";
    espenr.lastName = "Riskedal";
    espenr.middleName = "";
    espenr.company = "Trolltech";
    espenr.department = "Development";
    espenr.jobTitle = "Software Developer";
    model->appendEntry(espenr);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QVBoxWidget w;

    QToolBar *bar = new QToolBar(&w);
    QStackedWidget *stack = new QStackedWidget(&w);
    
    QListView *list = new QListView(stack);
    PimModel *model = new PimModel(stack);
    PimDelegate *delegate = new PimDelegate(stack);

    list->setModel(model);
    list->setItemDelegate(delegate);
    list->setAlternatingRowColors(true);

    PimEditor *editor = new PimEditor(stack);
    editor->setModel(model);

    PimViewer *viewer = new PimViewer(stack);
    viewer->setModel(model);

    PimSignalHub *hub = new PimSignalHub(stack);
    hub->setStack(stack);
    hub->setModel(model);
    hub->setView(list);
    hub->setEditor(editor);
    hub->setViewer(viewer);

    QObject::connect(list, SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
                     hub, SLOT(viewCurrent()));
    QObject::connect(viewer, SIGNAL(done()), hub, SLOT(setDefault()));
    QObject::connect(editor, SIGNAL(done()), hub, SLOT(setDefault()));

    bar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    bar->addAction(QPixmap(":/images/new.png"), "New Entry", hub, SLOT(createNew()));
    bar->addAction(QPixmap(":/images/edit.png"), "Edit Current Entry", hub, SLOT(editCurrent()));
    bar->addAction(QPixmap(":/images/book.png"), "View Current Entry", hub, SLOT(viewCurrent()));
    bar->addAction(QPixmap(":/images/delete.png"), "Delete Current Entry", hub, SLOT(deleteCurrent()));

    addData(model);
    app.setMainWidget(&w);
    w.setWindowIcon(QPixmap(":/images/interview.png"));
    w.show();
    
    return app.exec();
}
