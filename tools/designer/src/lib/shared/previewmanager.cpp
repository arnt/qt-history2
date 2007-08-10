/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "previewmanager_p.h"
#include "qdesigner_formbuilder_p.h"
#include <deviceskin.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>

#include <QtGui/QWidget>
#include <QtGui/qevent.h>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>
#include <QtGui/QDialog>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCursor>
#include <QtGui/QStyleFactory>

#include <QtCore/QMap>
#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <private/qobject_p.h>

static inline int compare(const qdesigner_internal::PreviewConfiguration &pc1, const qdesigner_internal::PreviewConfiguration &pc2)
{
    int rc = pc1.style.compare(pc2.style);
    if (rc)
        return rc;
    rc = pc1.applicationStyleSheet.compare(pc2.applicationStyleSheet);
    if (rc)
        return rc;
    return pc1.deviceSkin.compare(pc2.deviceSkin);
}

namespace {
    // ------ PreviewData
    struct PreviewData {
        PreviewData(const QPointer<QWidget> &widget, const  QDesignerFormWindowInterface *formWindow, const qdesigner_internal::PreviewConfiguration &pc);
        QPointer<QWidget> m_widget;
        const QDesignerFormWindowInterface *m_formWindow;
        qdesigner_internal::PreviewConfiguration m_configuration;
    };

    PreviewData::PreviewData(const QPointer<QWidget>& widget,
                             const QDesignerFormWindowInterface *formWindow,
                             const qdesigner_internal::PreviewConfiguration &pc) :
        m_widget(widget),
        m_formWindow(formWindow),
        m_configuration(pc)
    {
    }
}

namespace qdesigner_internal {
// --------- Widget Preview skin: Forward the key events to the window
class PreviewDeviceSkin : public  DeviceSkin
{
    Q_OBJECT
public:
    explicit PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *p );

private slots:
    void slotSkinKeyPressEvent(int code, const QString& text, bool autorep);
    void slotSkinKeyReleaseEvent(int code, const QString& text, bool autorep);
    void slotPopupMenu();
};

PreviewDeviceSkin::PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *p) :
    DeviceSkin(parameters, p)
{
    connect(this, SIGNAL(skinKeyPressEvent(int,QString,bool)),
            this, SLOT(slotSkinKeyPressEvent(int,QString,bool)));
    connect(this, SIGNAL(skinKeyReleaseEvent(int,QString,bool)),
            this, SLOT(slotSkinKeyReleaseEvent(int,QString,bool)));
    connect(this, SIGNAL(popupMenu()), this, SLOT(slotPopupMenu()));
}

void PreviewDeviceSkin::slotSkinKeyPressEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyPress,code,0,text,autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}

void PreviewDeviceSkin::slotSkinKeyReleaseEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyRelease,code,0,text,autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}

void PreviewDeviceSkin::slotPopupMenu()
{
    QMenu menu(this);
    connect(menu.addAction(tr("&Close")), SIGNAL(triggered()), parentWidget(), SLOT(close()));
    menu.exec(QCursor::pos());
}

// ------------- PreviewConfiguration

static const char *styleKey = "Style";
static const char *appStyleSheetKey = "AppStyleSheet";
static const char *skinKey = "Skin";

PreviewConfiguration::PreviewConfiguration()
{
}

PreviewConfiguration::PreviewConfiguration(const QString &sty, const QString &applicationSheet, const QString &skin) :
    style(sty),
    applicationStyleSheet(applicationSheet),
    deviceSkin(skin)
{
}

void PreviewConfiguration::clear()
{
    style.clear();
    applicationStyleSheet.clear();
    deviceSkin.clear();
}

void PreviewConfiguration::toSettings(const QString &prefix, QSettings &settings) const
{
    settings.beginGroup(prefix);
    settings.setValue(QLatin1String(styleKey),  style);
    settings.setValue(QLatin1String(appStyleSheetKey), applicationStyleSheet);
    settings.setValue(QLatin1String(skinKey), deviceSkin);
    settings.endGroup();
}

void PreviewConfiguration::fromSettings(const QString &prefix, const QSettings &settings)
{
    clear();
    QString key = prefix;
    key += QLatin1Char('/');
    const int prefixSize = key.size();

    const QVariant emptyString = QVariant(QString());

    key += QLatin1String(styleKey);
    style = settings.value(key, emptyString).toString();

    key.replace(prefixSize, key.size() - prefixSize, QLatin1String(appStyleSheetKey));
    applicationStyleSheet = settings.value(key, emptyString).toString();

    key.replace(prefixSize, key.size() - prefixSize, QLatin1String(skinKey));
    deviceSkin = settings.value(key, emptyString).toString();
}


QDESIGNER_SHARED_EXPORT bool operator<(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) < 0;
}

QDESIGNER_SHARED_EXPORT bool operator==(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) == 0;
}

QDESIGNER_SHARED_EXPORT bool operator!=(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
    return compare(pc1, pc2) != 0;
}

// ------------- PreviewManagerPrivate
class PreviewManagerPrivate :  public  QObjectPrivate {
public:
    PreviewManagerPrivate(PreviewManager::PreviewMode mode);

    const PreviewManager::PreviewMode m_mode;

    QPointer<QWidget> m_activePreview;

    typedef QList<PreviewData> PreviewDataList;

    PreviewDataList m_previews;

    typedef QMap<QString, DeviceSkinParameters> DeviceSkinConfigCache;
    DeviceSkinConfigCache m_deviceSkinConfigCache;
    bool m_updateBlocked;




};

PreviewManagerPrivate::PreviewManagerPrivate(PreviewManager::PreviewMode mode) :
    m_mode(mode),
    m_updateBlocked(false)
{
}

// ------------- PreviewManager

PreviewManager::PreviewManager(PreviewMode mode, QObject *parent) :
   QObject(*(new PreviewManagerPrivate(mode)), parent)
{
}

Qt::WindowFlags PreviewManager::previewWindowFlags(const QWidget *widget) const
{
#ifdef Q_WS_WIN
    Qt::WindowFlags windowFlags = (widget->windowType() == Qt::Window) ? Qt::Window | Qt::WindowMaximizeButtonHint : Qt::WindowFlags(Qt::Dialog);
#else
    Q_UNUSED(widget)
    // Only Dialogs have close buttons on Mac.
    // On Linux, we don't want an additional task bar item and we don't want a minimize button;
    // we want the preview to be on top.
    Qt::WindowFlags windowFlags = Qt::Dialog;
#endif
    return windowFlags;
}

QWidget *PreviewManager::createDeviceSkinContainer(const QDesignerFormWindowInterface *fw) const
{
    return new QDialog(fw->window());
}

QWidget *PreviewManager::createPreview(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc, QString *errorMessage)
{
    Q_D(PreviewManager);
    QWidget *widget = QDesignerFormBuilder::createPreview(fw, pc.style, pc.applicationStyleSheet, errorMessage);
    if (!widget)
        return widget;

    if (pc.deviceSkin.isEmpty()) {
        widget->setParent(fw->window(), previewWindowFlags(widget));
    } else {
        // Embed into skin.
        widget->setWindowModality(Qt::NonModal); // Otherwise, dialogs will block

        // find config in cache
        PreviewManagerPrivate::DeviceSkinConfigCache::iterator it = d->m_deviceSkinConfigCache.find(pc.deviceSkin);
        if (it == d->m_deviceSkinConfigCache.end()) {
            DeviceSkinParameters parameters;
            if (!parameters.read(pc.deviceSkin, DeviceSkinParameters::ReadAll, errorMessage)) {
                widget->deleteLater();
                return 0;
            }
            it = d->m_deviceSkinConfigCache.insert(pc.deviceSkin, parameters);
        }

        QWidget *skinContainer = createDeviceSkinContainer(fw);
        DeviceSkin *skin = new PreviewDeviceSkin(it.value(), skinContainer);
        // Setup widget for skin
        widget->setFixedSize(it.value().screenSize());
        widget->setParent(skin, Qt::SubWindow);
        widget->setAutoFillBackground(true);
        skin->setView(widget);
        widget = skinContainer;
    }
    return widget;
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc, QString *errorMessage)
{
    Q_D(PreviewManager);
    if (QWidget *existingPreviewWidget = raise(fw, pc))
        return existingPreviewWidget;

    QWidget *widget = createPreview(fw, pc, errorMessage);
    if (!widget)
        return 0;
    // Install filter for Escape key
    widget->setAttribute(Qt::WA_DeleteOnClose, true);
    widget->installEventFilter(this);

    switch (d->m_mode) {
    case ApplicationModalPreview:
        // Cannot do this on the Mac as the dialog would have no close button
        widget->setWindowModality(Qt::ApplicationModal);
        break;
    case SingleFormNonModalPreview:
    case MultipleFormNonModalPreview:
        widget->setWindowModality(Qt::NonModal);
        connect(fw, SIGNAL(changed()), widget, SLOT(close()));
        connect(fw, SIGNAL(destroyed()), widget, SLOT(close()));
        if (d->m_mode == SingleFormNonModalPreview)
            connect(fw->core()->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)), widget, SLOT(close()));
        break;
    }
    // Semi-smart algorithm to position previews:
    // If its the first one, position relative to form.
    // 2nd, attempt to tile right (for comparing styles) or give up
    // Todo: Implement more sophisticated DWIM.
    const QSize size = widget->size();
    const bool firstPreview = d->m_previews.empty();
    if (firstPreview) {
        widget->move(fw->mapToGlobal(QPoint(10, 10)));
    } else {
        if (QWidget *lastPreview = d->m_previews.back().m_widget) {
            QDesktopWidget *desktop = qApp->desktop();
            const QRect lastPreviewGeometry = lastPreview->frameGeometry();
            const QRect availGeometry = desktop->availableGeometry(desktop->screenNumber(lastPreview));
            const QPoint newPos = lastPreviewGeometry.topRight() + QPoint(10, 0);
            if (newPos.x() +  size.width() < availGeometry.right())
                widget->move(newPos);
        }

    }
    d->m_previews.push_back(PreviewData(widget, fw, pc));
    widget->show();
    if (firstPreview)
        emit firstPreviewOpened();
    return widget;
}

QWidget *PreviewManager::raise(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc)
{
    typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;
    Q_D(PreviewManager);
    if (d->m_previews.empty())
        return false;

    // find matching window
    const PreviewDataList::const_iterator cend =  d->m_previews.constEnd();
    for (PreviewDataList::const_iterator it = d->m_previews.constBegin(); it !=  cend ;++it) {
        QWidget * w = it->m_widget;
        if (w && it->m_formWindow == fw && it->m_configuration == pc) {
            w->raise();
            w->activateWindow();
            return w;
        }
    }
    return 0;
}

void PreviewManager::closeAllPreviews()
{
    typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;
    Q_D(PreviewManager);
    if (!d->m_previews.empty()) {
        d->m_updateBlocked = true;
        d->m_activePreview = 0;
        const PreviewDataList::iterator cend =  d->m_previews.end();
        for (PreviewDataList::iterator it = d->m_previews.begin(); it !=  cend ;++it) {
            if (it->m_widget)
                it->m_widget->close();
        }
        d->m_previews.clear();
        d->m_updateBlocked = false;
        emit lastPreviewClosed();
    }
}

void PreviewManager::updatePreviewClosed(QWidget *w)
{
    typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;
    Q_D(PreviewManager);
    if (d->m_updateBlocked)
        return;

    const PreviewDataList::iterator cend =  d->m_previews.end();
    for (PreviewDataList::iterator it = d->m_previews.begin(); it !=  cend ;++it) {
        if (it->m_widget == w) {
            d->m_previews.erase(it);
            if (d->m_previews.empty())
                emit lastPreviewClosed();
            return;
        }
    }
}

bool PreviewManager::eventFilter(QObject *watched, QEvent *event)
{
    // Courtesy of designer
    do {
        if (!watched->isWidgetType())
            break;
        QWidget *previewWindow = qobject_cast<QWidget *>(watched);
        if (!previewWindow || !previewWindow->isWindow())
            break;

        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::ShortcutOverride:        {
            const  QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
            const int key = keyEvent->key();
            if ((key == Qt::Key_Escape
#ifdef Q_WS_MAC
                 || (keyEvent->modifiers() == Qt::ControlModifier && key == Qt::Key_Period)
#endif
                 )) {
                previewWindow->close();
                return true;
            }
        }
            break;
        case QEvent::WindowActivate: {
            Q_D(PreviewManager);
            d->m_activePreview = previewWindow;
        }
            break;
        case  QEvent::Close:
            updatePreviewClosed(previewWindow);
            previewWindow->removeEventFilter (this);
            break;
        default:
            break;
        }
    } while(false);
    return QObject::eventFilter(watched, event);
}

int PreviewManager::previewCount() const
{
    Q_D(const PreviewManager);
    return  d->m_previews.size();
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc, QString *errorMessage)
{
    QWidget *widget = createPreview(fw, pc, errorMessage);
    if (!widget)
        return QPixmap();

    return QDesignerFormBuilder::grabPreviewPixmap(widget);
}

QActionGroup *PreviewManager::createStyleActionGroup(QObject *parent, QMenu *subMenu)
{
    QActionGroup * rc = new QActionGroup(parent);
    rc->setExclusive(true);

    const QStringList styles = QStyleFactory::keys();
    const QStringList::const_iterator cend = styles.constEnd();

    // Make sure ObjectName  is unique in case toolbar solution is used.
    const QString objNamePrefix = QLatin1String("__qt_designer_style_");
    const QString objNamePostfix = QLatin1String("_action");
    for (QStringList::const_iterator it = styles.constBegin(); it !=  cend ;++it) {
        QAction *a = new QAction(tr("%1 Style").arg(*it), parent);
        QString objName = objNamePrefix;
        objName += *it;
        objName += objNamePostfix;
        a->setObjectName(objName);
        a->setData(*it);
        rc->addAction(a);
        if (subMenu)
            subMenu->addAction(a);
    }
    return rc;
}
}

#include "previewmanager.moc"
