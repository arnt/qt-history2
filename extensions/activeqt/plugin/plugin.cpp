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

#include <customwidget.h>
#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>
#include <abstractformwindowcursor.h>
#include <qextensionmanager.h>
#include <qdesigner_propertysheet.h>
#include <taskmenu.h>

#include <qplugin.h>

#include <QObject>
#include <QTimer>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>

#include <QUuid>
#include <QAxWidget>
#include <olectl.h>
#include "../container/qaxselect.h"

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
    Q_INTERFACES(IPropertySheet)
public:
    QActiveXPropertySheet(QAxWidget *object, QObject *parent = 0)
        : QDesignerPropertySheet(object, parent)
    { 
        createFakeProperty("control", QString());
        int index = indexOf("control");
        setVisible(index, false);
    }

    void setProperty(int index, const QVariant &value)
    {
        QDesignerPropertySheet::setProperty(index, value);

        if (isAdditionalProperty(index) 
            && (propertyName(index) == "control"))
        {
            if (!value.toString().isEmpty())
                static_cast<QActiveXPluginObject*>(m_object)->setControl(value.toString());
        }
    }

    virtual ~QActiveXPropertySheet() { }
};

class QActiveXTaskMenu: public QObject, public ITaskMenu
{
    Q_OBJECT
    Q_INTERFACES(ITaskMenu)
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

    static void setActiveXControl(QActiveXPluginObject *widget)
    {
       QAxSelect *dialog = new QAxSelect(widget->topLevelWidget());

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
	        if (hres == CLASS_E_NOTLICENSED) {
		    QMessageBox::warning(widget->topLevelWidget(), tr("Licensed Control"),
		        tr("The control requires a design-time license"));
		    clsid = QUuid();
	        }
                else
                {
		    key = QString::fromUtf16(bKey);
	        }
                cf2->Release();
            }

	    if (!clsid.isNull())
            {
                AbstractFormWindow *formWin = AbstractFormWindow::findFormWindow(widget);
                formWin->selectWidget(widget, true);

	        if (key.isEmpty())
                    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString());
	        else
		    formWin->cursor()->setProperty(QLatin1String("control"), clsid.toString() + ":" + key);
            }
        }

        delete dialog;
    }

private slots:
    void setActiveXControl()
    { QTimer::singleShot(100, this, SLOT(openLater())); }

    void openLater()
    { setActiveXControl(static_cast<QActiveXPluginObject*>(m_axwidget)); }

private:
    QAxWidget *m_axwidget;
    mutable QList<QAction*> m_taskActions;
};

class QActiveXExtensionFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    QActiveXExtensionFactory(QExtensionManager *parent = 0)
        : DefaultExtensionFactory(parent) { }

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const
    {
        QAxWidget *w = qobject_cast<QAxWidget*>(object);

        if (!w)
            return 0;

        if (iid == Q_TYPEID(IPropertySheet))
            return new QActiveXPropertySheet(w, parent);

        if (iid == Q_TYPEID(ITaskMenu))
            return new QActiveXTaskMenu(w, parent);
        
        return 0;
    }
};

class QActiveXPlugin : public QObject, public ICustomWidget
{
    Q_OBJECT
    Q_INTERFACES(ICustomWidget)
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
    
    virtual bool isForm() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    {
        return new QActiveXPluginObject(parent);
    }
    
    virtual bool isInitialized() const 
    { return (m_core != 0); }
    
    virtual void initialize(AbstractFormEditor *core) 
    { 
        if (m_core != 0)
            return;

        m_core = core;

        QExtensionManager *mgr = core->extensionManager();
        QActiveXExtensionFactory *axf = new QActiveXExtensionFactory(mgr);
        mgr->registerExtensions(axf, Q_TYPEID(IPropertySheet));
        mgr->registerExtensions(axf, Q_TYPEID(ITaskMenu));
    }

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"QAxWidget\" name=\"QAxWidget\">\
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
    
    virtual QString codeTemplate() const
    { return QString(); }

private:
    AbstractFormEditor *m_core;
};

Q_EXPORT_PLUGIN(QActiveXPlugin)

#include "plugin.moc"
