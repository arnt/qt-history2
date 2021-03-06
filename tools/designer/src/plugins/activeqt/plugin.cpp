/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/private/qdesigner_propertysheet_p.h>
#include <QtDesigner/private/qdesigner_taskmenu_p.h>
#include <QtDesigner/private/extensionfactory_p.h>

#include <QtCore/qplugin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>

#include <ActiveQt/QAxWidget>
#include <ActiveQt/qaxselect.h>

#include <QtCore/QMetaProperty>

#include <olectl.h>
#include "activeqt_extrainfo.h"
#include <qaxtypes.h>

QT_BEGIN_NAMESPACE

/* XPM */
static const char *widgetIcon[]={
"22 22 6 1",
"a c #000000",
"# c #808080",
"+ c #aaa5a0",
"b c #dddddd",
"* c #d4d0c8",
". c none",
".........#aa#...#aa#..",
".........abba...abba..",
".........abba...abba..",
".........#aa#...#aa#..",
"..........aa.....aa...",
"..........aa.....aa...",
"..........aa.....aa...",
".......aaaaaaaaaaaaaaa",
".......a*************a",
".......a************#a",
".......a***********+#a",
".......a***********+#a",
".......a***********+#a",
"#aa#...a***********+#a",
"abbaaaaa***********+#a",
"abbaaaaa***********+#a",
"#aa#...a***********+#a",
".......a***********+#a",
".......a***********+#a",
".......a**++++++++++#a",
".......a*############a",
".......aaaaaaaaaaaaaaa"};

class QActiveXPlugin;

static const uint qt_meta_data_QActiveXPluginObject[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_QActiveXPluginObject[] = {
    "QAxWidget\0"
};

const QMetaObject QActiveXPluginObject::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QActiveXPluginObject,
      qt_meta_data_QActiveXPluginObject, 0 }
};

QActiveXPluginObject::QActiveXPluginObject(QWidget *parent)
    : QWidget(parent), m_axobject(0)
{
    m_axImage = QPixmap(widgetIcon);
}

QActiveXPluginObject::~QActiveXPluginObject()
{
    qDeleteAll(m_propValues);
    delete m_axobject;
}

const QMetaObject *QActiveXPluginObject::metaObject() const
{
    if (m_axobject)
        return m_axobject->metaObject();

    return &staticMetaObject;
}

int QActiveXPluginObject::qt_metacall(QMetaObject::Call call, int signal, void **argv)
{
    if (m_axobject) {
        const QMetaObject *mo = metaObject();
        int ioffset = mo->indexOfProperty("control");
        
        if (ioffset == signal)
            return m_axobject->qt_metacall(call,signal,argv);

        ioffset = mo->propertyOffset();
        if (signal < ioffset)
            return QWidget::qt_metacall(call,signal,argv);


        if (call == QMetaObject::WriteProperty) {
            QMetaProperty mprop = mo->property(signal);
            QVariant qVar(mprop.type(), argv[0]);
            m_propValues.insert(signal, new QVariant(qVar));
            return 1;
        } else if(call == QMetaObject::ReadProperty) {
            if (m_propValues.contains(signal)) {
                QMetaProperty mprop = mo->property(signal);
                QVariantToVoidStar(*m_propValues.value(signal), *argv, mprop.typeName(), mprop.type());
                return 1;
            } else {
                return m_axobject->qt_metacall(call,signal,argv);
            }
        } else if(call == QMetaObject::QueryPropertyStored) {
            if (m_propValues.contains(signal))
                if (argv[0]) *reinterpret_cast< bool*>(argv[0]) = true;
        }
    }

    return QWidget::qt_metacall(call,signal,argv);
}

bool QActiveXPluginObject::setControl(const QString &clsid)
{
    m_axobject = new QAxWidget();

    if (!m_axobject->setControl(clsid)) {
        delete m_axobject;
        m_axobject = 0;
        return false;
    }
        
    update();
    return true;
}

QSize QActiveXPluginObject::sizeHint() const
{
    return QSize(80,70);
}

void QActiveXPluginObject::paintEvent (QPaintEvent * /*event */)
{
    static const QString loaded = tr("Control loaded");
    QPainter p(this);
    const QRect r = contentsRect();

    if (m_axobject) {
        p.setBrush(QBrush(Qt::green, Qt::BDiagPattern));
        p.drawRect(r.adjusted(0,0,-1,-1));
        p.setPen(Qt::black);
        p.drawText(5,r.height()-5,loaded);
    } else {
        p.drawRect(r.adjusted(0,0,-1,-1));
    }

    p.drawPixmap((r.width()-m_axImage.width())/2,
        (r.height()-m_axImage.height())/2,m_axImage);
}

class QActiveXPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QActiveXPropertySheet(QActiveXPluginObject *object, QObject *parent = 0)
        : QDesignerPropertySheet(object, parent)
    {
        const int index = indexOf(QLatin1String("control"));
        if (index != -1)
            setVisible(index, false);
    }

    void setProperty(int index, const QVariant &value)
    {
        // take care of all changed properties
        m_currentProperties.changedProperties[propertyName(index)] = value;

        if (propertyName(index) == QLatin1String("control"))
        {            
            const QString clsid = value.toString();            
            if (!clsid.isEmpty()) {
                QActiveXPluginObject *pluginObject = static_cast<QActiveXPluginObject*>(object());
                if (!pluginObject->loaded()) {
                    if (pluginObject->setControl(clsid)) {
                        m_currentProperties.clsid = clsid;
                        m_currentProperties.widget = static_cast<QWidget*>(object());

                        QTimer::singleShot(100, this, SLOT(updatePropertySheet()));
                    }
                }                
            }
        } else {
            QDesignerPropertySheet::setProperty(index, value);
        }
    }

    int indexOf(const QString &name) const
    {
        const int index = QDesignerPropertySheet::indexOf(name);
        if (index == -1) {
            // since metaobject is different, we must store the values in fake properties
            (const_cast<QActiveXPropertySheet *>(this))->createFakeProperty(name, QVariant());
        }

        return index;
    }

private slots:
    void updatePropertySheet()
    {
        // refresh the property sheet (we are deleting m_currentProperties)
        const struct SavedProperties tmp = m_currentProperties;
        delete this; //we delete the property sheet, because this forces a recreation
        reloadPropertySheet(tmp);
    }

private:
    struct SavedProperties {
        typedef QMap<QString, QVariant> NamePropertyMap;
        NamePropertyMap changedProperties;
        QWidget *widget;
        QString clsid;
    } m_currentProperties;

    static void reloadPropertySheet(const struct SavedProperties &properties)
    {
        QDesignerFormWindowInterface *formWin = QDesignerFormWindowInterface::findFormWindow(properties.widget);
        Q_ASSERT(formWin != 0);

        QDesignerFormEditorInterface *core = formWin->core();
        QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), properties.widget);
        
        const SavedProperties::NamePropertyMap::const_iterator cend = properties.changedProperties.constEnd();
        for (SavedProperties::NamePropertyMap::const_iterator i = properties.changedProperties.constBegin(); i != cend; ++i) {
            const int index = sheet->indexOf(i.key());
            sheet->setChanged(index, true);
            sheet->setProperty(index, i.value());
        }

        formWin->clearSelection(true);
        formWin->selectWidget(properties.widget);
    }
};

class QActiveXTaskMenu: public qdesigner_internal::QDesignerTaskMenu
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit QActiveXTaskMenu(QActiveXPluginObject *object, QObject *parent = 0) : 
       qdesigner_internal::QDesignerTaskMenu(object, parent),
       m_axwidget(object), 
       m_action(0),m_separator(0) { }

    virtual ~QActiveXTaskMenu() { }

    virtual QList<QAction*> taskActions() const
    {
        if (m_taskActions.isEmpty()) {
           QActiveXTaskMenu *that = const_cast<QActiveXTaskMenu*>(this);
           m_separator = new QAction(that);
           m_separator->setSeparator(true);
           m_action = new QAction(that);
           m_action->setText(tr("Set Control"));
           m_action->setEnabled(!m_axwidget->loaded());
           
           connect(m_action, SIGNAL(triggered()), this, SLOT(setActiveXControl()));
           m_taskActions.append(m_action);
        }
        QList<QAction*> rc = QDesignerTaskMenu::taskActions();
        if (!rc.empty())
            rc += m_separator; 
        rc += m_taskActions;
        return rc;
    }

private slots:
    void setActiveXControl()
    {
        QAxSelect *dialog = new QAxSelect(m_axwidget->topLevelWidget());

        if (dialog->exec())
        {
            QUuid clsid = dialog->clsid();
            QString key;

            IClassFactory2 *cf2 = 0;
            CoGetClassObject(clsid, CLSCTX_SERVER, 0, IID_IClassFactory2, (void**)&cf2);

            if (cf2)
            {
                BSTR bKey;
                HRESULT hres = cf2->RequestLicKey(0, &bKey);
                if (hres == CLASS_E_NOTLICENSED)
                {
                    QMessageBox::warning(m_axwidget->topLevelWidget(), tr("Licensed Control"),
                        tr("The control requires a design-time license"));
                    clsid = QUuid();
                }
                else
                {
                    key = QString::fromUtf16((ushort *)bKey);
                }

                cf2->Release();
            }

            if (!clsid.isNull())
            {
                QDesignerFormWindowInterface *formWin = QDesignerFormWindowInterface::findFormWindow(m_axwidget);
                Q_ASSERT(formWin != 0);

                formWin->selectWidget(m_axwidget, true);

                m_action->setEnabled(false);
                if (key.isEmpty())
                    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString());
                else
                    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString() + QLatin1String(":") + key);
            }
        }
        delete dialog;
    }

private:
    QActiveXPluginObject *m_axwidget;
    mutable QAction *m_action;
    mutable QAction *m_separator;
    mutable QList<QAction*> m_taskActions;
};

typedef QDesignerPropertySheetFactory<QActiveXPluginObject, QActiveXPropertySheet> ActiveXPropertySheetFactory;
typedef qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, QActiveXPluginObject, QActiveXTaskMenu>  ActiveXTaskMenuFactory;

class QActiveXPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    explicit inline QActiveXPlugin(QObject *parent = 0)
        : QObject(parent), m_core(0) {}

    virtual QString name() const
    { return QLatin1String("QAxWidget"); }

    virtual QString group() const
    { return QLatin1String("Containers"); }

    virtual QString toolTip() const
    { return tr("ActiveX control"); }

    virtual QString whatsThis() const
    { return tr("ActiveX control widget"); }

    virtual QString includeFile() const
    { return QLatin1String("qaxwidget.h"); }

    virtual QIcon icon() const
    { return QIcon(widgetIcon); }

    virtual bool isContainer() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    {
        return new QActiveXPluginObject(parent);
    }

    virtual bool isInitialized() const
    { return (m_core != 0); }

    virtual void initialize(QDesignerFormEditorInterface *core)
    {
        if (m_core != 0)
            return;

        m_core = core;

        QExtensionManager *mgr = core->extensionManager();
        ActiveXPropertySheetFactory::registerExtension(mgr);
        ActiveXTaskMenuFactory::registerExtension(mgr, Q_TYPEID(QDesignerTaskMenuExtension));
        QAxWidgetExtraInfoFactory *extraInfoFactory = new QAxWidgetExtraInfoFactory(core, mgr);
        mgr->registerExtensions(extraInfoFactory, Q_TYPEID(QDesignerExtraInfoExtension));
    }

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"QAxWidget\" name=\"axWidget\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>80</width>\
                    <height>70</height>\
                </rect>\
            </property>\
        </widget>\
      "); }

private:
    QDesignerFormEditorInterface *m_core;
};

Q_EXPORT_PLUGIN(QActiveXPlugin)

QT_END_NAMESPACE

#include "plugin.moc"
