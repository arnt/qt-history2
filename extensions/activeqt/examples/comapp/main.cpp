#include <qapplication.h>
#include <qaxfactory.h>
#include <qwidget.h>
#include <qtimer.h>

class Application;
class DocumentList;

class Window : public QWidget
{
public:
    Window()
    {
    }
};

class Document : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID", "{2b5775cd-72c2-43da-bc3b-b0e8d1e1c4f7}")
    Q_CLASSINFO("InterfaceID", "{2ce1761e-07a3-415c-bd11-0eab2c7283de}")

    Q_PROPERTY(int value READ value WRITE setValue)

public:
    Document(DocumentList *list);

    int value() const;
    void setValue(int value);

private:
    int v;
};

class DocumentList : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID", "{496b761d-924b-4554-a18a-8f3704d2a9a6}")
    Q_CLASSINFO("InterfaceID", "{6c9e30e8-3ff6-4e6a-9edc-d219d074a148}")

    Q_PROPERTY(int count READ count)
    Q_PROPERTY(Application* application READ application)

public:
    DocumentList(Application *application);

    int count() const;
    Application *application() const;

public slots:
    Document *addDocument();
    Document *item(int index) const;

public:
    QList<Document*> list;
};

class Application : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID", "{b50a71db-c4a7-4551-8d14-49983566afee}")
    Q_CLASSINFO("InterfaceID", "{4a427759-16ef-4ed8-be79-59ffe5789042}")
    Q_CLASSINFO("RegisterObject", "yes")

    Q_PROPERTY(DocumentList* documents READ documents)
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    Application(QObject *parent = 0);
    DocumentList *documents() const;

    QString id() const { return objectName(); }

    void setVisible(bool on);
    bool isVisible() const;

    Window *window() const { return ui; }

public slots:
    void quit();

private:
    DocumentList *docs;

    Window *ui;
};

Document::Document(DocumentList *list)
: QObject(list), v(0)
{
}

int Document::value() const
{
    return v;
}

void Document::setValue(int value)
{
    v = value;
}

DocumentList::DocumentList(Application *application)
: QObject(application)
{
}

Application *DocumentList::application() const
{
    return qt_cast<Application*>(parent());
}

int DocumentList::count() const
{
    return list.count();
}

Document *DocumentList::item(int index) const
{
    if (index >= list.count())
        return 0;

    return list.at(index);
}

Document *DocumentList::addDocument()
{
    Document *document = new Document(this);
    list.append(document);
    document->setValue(list.count());

    return document;
}


Application::Application(QObject *parent)
: QObject(parent), ui(0)
{
    setObjectName("From QAxFactory");
    docs = new DocumentList(this);
}

DocumentList *Application::documents() const
{
    return docs;
}

void Application::setVisible(bool on)
{
    if (on) {
        ui = new Window;
        ui->show();
    } else {
        delete ui;
    }
}

bool Application::isVisible() const
{
    return ui && ui->isVisible();
}

void Application::quit()
{
    QTimer::singleShot(0, qApp, SLOT(quit()));
}

#include "main.moc"


QAXFACTORY_BEGIN("{edd3e836-f537-4c6f-be7d-6014c155cc7a}", "{b7da3de8-83bb-4bbe-9ab7-99a05819e201}")
   QAXCLASS(Application)
   QAXTYPE(Document)
   QAXTYPE(DocumentList)
QAXFACTORY_END()

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // started by COM - don't do anything
    if (QAxFactory::isServer())
        return app.exec();

    // started by user
    Application appobject(0);
    appobject.setObjectName("From Application");

    QAxFactory::startServer();
    QAxFactory::registerActiveObject(&appobject);

    appobject.setVisible(true);
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    app.exec();

    return 0;
}
