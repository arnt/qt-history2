
#include <qpropertyeditor.h>
#include <qapplication.h>
#include <qtextedit.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qmetaobject.h>
#include <qdebug.h>

void createPropertySheet(PropertyCollection *root, QObject *object, const QMetaObject *meta = 0)
{
    if (!meta) {
        createPropertySheet(root, object, object->metaObject());
        return;
    }

    QString name = QLatin1String("[") + QString::fromUtf8(meta->className()) + QLatin1String("]");

    PropertyCollection *coll = 0;
    bool created = false;

    for (int propertyIndex = meta->propertyOffset(); propertyIndex < meta->propertyCount(); ++propertyIndex) {

        if (!created) {
            coll = new PropertyCollection(name);
            root->addProperty(coll);
            created = true;
        }

        QMetaProperty property = meta->property(propertyIndex);

        if (!property.isDesignable(object))
            continue;

        QString pname = QString::fromUtf8(property.name());
        QVariant value = property.read(object);

        I::Property *p = 0;
        switch (property.type()) {
            case QVariant::Int:
                p = new IntProperty(value.toInt(), pname);
                break;
            case QVariant::UInt:
                p = new IntProperty(value.toUInt(), pname);
                break;
            case QVariant::Bool:
                p = new BoolProperty(value.toBool(), pname);
                break;
            case QVariant::String:
                p = new StringProperty(value.toString(), pname);
                break;
            case QVariant::Size:
                p = new SizeProperty(value.toSize(), pname);
                break;
            case QVariant::Point:
                p = new PointProperty(value.toPoint(), pname);
                break;
            case QVariant::Font:
                p = new FontProperty(value.toFont(), pname);
                break;
            case QVariant::Color:
                p = new ColorProperty(value.toColor(), pname);
                break;
            default:
                if (property.isEnumType()) {
                    QMetaEnum e = property.enumerator();
                    if (e.isFlag()) {
                        qWarning("skip property %s. flags not supported yet!",
                            pname.latin1());
                        break;
                    }

                    QMap<QString, QVariant> items;
                    for (int i=0; i<e.keyCount(); ++i)
                        items.insert(QLatin1String(e.key(i)), e.value(i));

                    p = new MapProperty(items, value, pname);
                }
                break;
        }

        if (p)
            coll->addProperty(p);
    }

    if (meta->superClass())
        createPropertySheet(root, object, meta->superClass());
}

class TestApplication: public QApplication
{
    Q_OBJECT
public:
    TestApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
        QSplitter *splitter = new QSplitter();
        splitter->setOrientation(Qt::Vertical);
        propertyEditor = new QPropertyEditor::View(splitter);

        widget = createWidget(splitter);

        connect(propertyEditor, SIGNAL(propertyChanged(I::Property*)),
            this, SLOT(updateProperty(I::Property*)));

        setMainWidget(splitter);

        PropertyCollection *root = new PropertyCollection("root"); // ### delete me
        createPropertySheet(root, widget);

        propertyEditor->setInitialInput(root);
        propertyEditor->setInitialInput(0);
        propertyEditor->setInitialInput(root);
    }

    QWidget *createWidget(QWidget *parent) {
        QPushButton *btn = new QPushButton(parent);
        btn->setText(QLatin1String("Push me"));
        return btn;
    }


public slots:
    void updateProperty(I::Property *property)
    {
        widget->setProperty(property->propertyName(), property->value());
        property->setDirty(false);
    }

private:
    QPropertyEditor::View *propertyEditor;
    QWidget *widget;
};

int main(int argc, char *argv[])
{
    TestApplication app(argc, argv);
    app.mainWidget()->show();

    return app.exec();
}

#include "main.moc"
