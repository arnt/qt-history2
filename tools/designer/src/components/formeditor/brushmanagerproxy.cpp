#include "qtbrushmanager.h"
#include "brushmanagerproxy.h"
#include "qsimpleresource_p.h"
#include <QDomDocument>

namespace qdesigner_internal {

class BrushManagerProxyPrivate
{
    BrushManagerProxy *q_ptr;
    Q_DECLARE_PUBLIC(BrushManagerProxy)
public:
    void brushAdded(const QString &name, const QBrush &brush);
    void brushRemoved(const QString &name);
    QString brushFileName(const QString &brushName) const;

    QtBrushManager *theManager;
    QString theBrushFolder;
    QDesignerFormEditorInterface *theCore;
};

}  // namespace qdesigner_internal

using namespace qdesigner_internal;

void BrushManagerProxyPrivate::brushAdded(const QString &name, const QBrush &brush)
{
    QString filename = brushFileName(name);

    QDir designerDir(QDir::homePath() + QDir::separator() + QLatin1String(".designer"));
    if (!designerDir.exists(QLatin1String("brushes")))
        designerDir.mkdir(QLatin1String("brushes"));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QSimpleResource resource(theCore);

    DomBrush *dom = resource.saveBrush(brush);
    QDomDocument doc;
    doc.appendChild(dom->write(doc));
    file.write(doc.toString().toUtf8());

    file.close();

    delete dom;
}

void BrushManagerProxyPrivate::brushRemoved(const QString &name)
{
    QDir brushDir(theBrushFolder);

    brushDir.remove(name + QLatin1String(".br"));
}

QString BrushManagerProxyPrivate::brushFileName(const QString &brushName) const
{
    return theBrushFolder + QDir::separator() + brushName + QLatin1String(".br");
}


BrushManagerProxy::BrushManagerProxy(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent)
{
    d_ptr = new BrushManagerProxyPrivate;
    d_ptr->q_ptr = this;

    d_ptr->theManager = 0;
    d_ptr->theBrushFolder = QDir::homePath()
                    + QDir::separator()
                    + QLatin1String(".designer")
                    + QDir::separator()
                    + QLatin1String("brushes");
    d_ptr->theCore = core;
}

BrushManagerProxy::~BrushManagerProxy()
{
    delete d_ptr;
}

void BrushManagerProxy::setBrushManager(QtBrushManager *manager)
{
    if (d_ptr->theManager == manager)
        return;

    if (d_ptr->theManager) {
        disconnect(d_ptr->theManager, SIGNAL(brushAdded(const QString &, const QBrush &)),
                    this, SLOT(brushAdded(const QString &, const QBrush &)));
        disconnect(d_ptr->theManager, SIGNAL(brushRemoved(const QString &)),
                    this, SLOT(brushRemoved(const QString &)));
    }

    d_ptr->theManager = manager;

    if (!d_ptr->theManager)
        return;

    // clear the manager
    QMap<QString, QBrush> brushes = d_ptr->theManager->brushes();
    QMap<QString, QBrush>::ConstIterator it = brushes.constBegin();
    while (it != brushes.constEnd()) {
        QString name = it.key();
        d_ptr->theManager->removeBrush(name);

        it++;
    }

    // fill up the manager from compiled resources or from brush folder here
    QDir brushDir(d_ptr->theBrushFolder);
    bool customBrushesExist = brushDir.exists();
    if (customBrushesExist) {
        // load brushes from brush folder
        QStringList nameFilters;
        nameFilters.append(QLatin1String("*.br"));

        QFileInfoList infos = brushDir.entryInfoList(nameFilters);
        QListIterator<QFileInfo> it(infos);
        while (it.hasNext()) {
            QFileInfo fi = it.next();

            QString name = fi.baseName();

            QFile file(fi.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray contents = file.readAll();
                file.close();
                QDomDocument doc;
                if (doc.setContent(contents)) {
                    DomBrush dom;
                    dom.read(doc.documentElement());

                    QSimpleResource resource(d_ptr->theCore);

                    QBrush br = resource.setupBrush(&dom);

                    d_ptr->theManager->addBrush(name, br);
                }
            }
        }
    }

    connect(d_ptr->theManager, SIGNAL(brushAdded(const QString &, const QBrush &)),
            this, SLOT(brushAdded(const QString &, const QBrush &)));
    connect(d_ptr->theManager, SIGNAL(brushRemoved(const QString &)),
            this, SLOT(brushRemoved(const QString &)));

    if (!customBrushesExist) {
        // load brushes from resources
    }
}

#include "moc_brushmanagerproxy.cpp"
