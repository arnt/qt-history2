/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/private/qdesigner_propertysheet_p.h>

#include <QtCore/qplugin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>

#include <ActiveQt/QAxWidget>

#include <olectl.h>
#include "../container/qaxselect.h"
#include "activeqt_extrainfo.h"

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

class QActiveXPluginObject : public QAxWidget
{
public:
    QActiveXPluginObject(QWidget *parent)
        : QAxWidget(parent), m_hasControl(false)
    {
        m_axImage = QPixmap(widgetIcon);
    }

    ~QActiveXPluginObject()
    {
    }

    bool setControl(const QString &clsid)
    {
        m_hasControl = QAxWidget::setControl(clsid);
        return m_hasControl;
    }

    QSize sizeHint() const
    {
        return QSize(80,70);
    }

protected:
    void paintEvent (QPaintEvent *event)
    {
        if (!m_hasControl)
        {
            QPainter p(this);
            QRect r = contentsRect();
            p.drawRect(r.adjusted(0,0,-1,-1));
            p.drawPixmap((r.width()-m_axImage.width())/2,
                (r.height()-m_axImage.height())/2,m_axImage);
        }
        else
        {
            QAxWidget::paintEvent(event);
        }
    }

private:
    QPixmap m_axImage;
    bool m_hasControl;
};

class QActiveXPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    QActiveXPropertySheet(QAxWidget *object, QObject *parent = 0)
        : QDesignerPropertySheet(object, parent)
    {
        int index = indexOf(QLatin1String("control"));
        if (index == -1) {
            createFakeProperty(QLatin1String("control"), QString());
            index = indexOf(QLatin1String("control"));
        }

        setVisible(index, false);
    }

    virtual ~QActiveXPropertySheet()
    {

    }

    void setProperty(int index, const QVariant &value)
    {
        QDesignerPropertySheet::setProperty(index, value);

        if (isAdditionalProperty(index)
            && (propertyName(index) == QLatin1String("control")))
        {
            QString clsid = value.toString();
            if (!clsid.isEmpty()) {
                static_cast<QActiveXPluginObject*>(m_object)->setControl(clsid);

                // don't update the property sheet if the widget isn't in a form (preview)
                if (QDesignerFormWindowInterface::findFormWindow(static_cast<QWidget*>(m_object)))
                {
                    m_currentProperties.clsid = clsid;
                    m_currentProperties.widget = static_cast<QWidget*>(m_object);

                    QTimer::singleShot(100, this, SLOT(updatePropertySheet()));
                }
            }
        }
    }

    int indexOf(const QString &name) const
    {
        int index = QDesignerPropertySheet::indexOf(name);
        if (index == -1) {
            // since metaobject is different, we must store the values in fake properties
            index = count();
            (const_cast<QActiveXPropertySheet *>(this))->m_addIndex.insert(name, index);
            (const_cast<QActiveXPropertySheet *>(this))->m_addProperties.insert(index, QVariant());
        }

        return index;
    }

private slots:
    void updatePropertySheet()
    {
        // compute the changed properties
        for (int i = 0; i<count(); ++i) {
            if (isChanged(i))
                m_currentProperties.changedProperties[propertyName(i)] = property(i);
        }

        // refresh the property sheet (we are deleting m_currentProperties)
        struct SavedProperties tmp = m_currentProperties;
        delete this;
        reloadPropertySheet(tmp);
    }

private:
    struct SavedProperties {
        QMap<QString, QVariant> changedProperties;
        QWidget *widget;
        QString clsid;
    } m_currentProperties;

    static void reloadPropertySheet(struct SavedProperties properties)
    {
        QDesignerFormWindowInterface *formWin = QDesignerFormWindowInterface::findFormWindow(properties.widget);
        Q_ASSERT(formWin != 0);

        QDesignerFormEditorInterface *core = formWin->core();
        QDesignerPropertySheetExtension *sheet = 0;

        sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), properties.widget);

        QMap<QString, QVariant>::const_iterator i = properties.changedProperties.constBegin();
        while (i != properties.changedProperties.constEnd()) {
            int index = sheet->indexOf(i.key());
            sheet->setChanged(index, true);
            sheet->setProperty(index, i.value());
            ++i;
        }

        formWin->clearSelection(true);
        formWin->selectWidget(properties.widget);
    }
};

class QActiveXTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    QActiveXTaskMenu(QAxWidget *object, QObject *parent = 0)
        : QObject(parent), m_axwidget(object) { }
    virtual ~QActiveXTaskMenu() { }

    virtual QList<QAction*> taskActions() const
    {
        if (!m_taskActions.isEmpty())
            return m_taskActions;

        QAction *action = 0;
        QActiveXTaskMenu *that = const_cast<QActiveXTaskMenu*>(this);

        action = new QAction(that);
        action->setText(tr("Set Control"));
        connect(action, SIGNAL(triggered()), this, SLOT(setActiveXControl()));
        m_taskActions.append(action);

        return m_taskActions;
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

                if (key.isEmpty())
                    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString());
                else
                    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString() + ":" + key);
            }
        }
        delete dialog;
    }

private:
    QAxWidget *m_axwidget;
    mutable QList<QAction*> m_taskActions;
};

class QActiveXExtensionFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QActiveXExtensionFactory(QExtensionManager *parent, QDesignerFormEditorInterface *core)
        : QExtensionFactory(parent) { m_core = core; }

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const
    {
        QAxWidget *w = qobject_cast<QAxWidget*>(object);

        if (!w)
            return 0;

        if (iid == Q_TYPEID(QDesignerPropertySheetExtension))
            return new QActiveXPropertySheet(w, parent);

        if (iid == Q_TYPEID(QDesignerTaskMenuExtension))
            return new QActiveXTaskMenu(w, parent);

        return 0;
    }

private:
    QDesignerFormEditorInterface *m_core;
};

class QActiveXPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    inline QActiveXPlugin(QObject *parent = 0)
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
        QActiveXExtensionFactory *axf = new QActiveXExtensionFactory(mgr, core);
        mgr->registerExtensions(axf, Q_TYPEID(QDesignerPropertySheetExtension));
        mgr->registerExtensions(axf, Q_TYPEID(QDesignerTaskMenuExtension));

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

#include "plugin.moc"
