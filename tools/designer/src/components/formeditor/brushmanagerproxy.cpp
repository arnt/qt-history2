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
    QString uniqueBrushFileName(const QString &brushName) const;

    QtBrushManager *theManager;
    QString theBrushFolder;
    QDesignerFormEditorInterface *theCore;
    QMap<QString, QString> theFileToBrush;
    QMap<QString, QString> theBrushToFile;
};

}  // namespace qdesigner_internal

using namespace qdesigner_internal;

void BrushManagerProxyPrivate::brushAdded(const QString &name, const QBrush &brush)
{
    QString filename = uniqueBrushFileName(name);

    QDir designerDir(QDir::homePath() + QDir::separator() + QLatin1String(".designer"));
    if (!designerDir.exists(QLatin1String("brushes")))
        designerDir.mkdir(QLatin1String("brushes"));

    QFile file(theBrushFolder + QDir::separator() +filename);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QSimpleResource resource(theCore);

    DomBrush *dom = resource.saveBrush(brush);
    QDomDocument doc;
    QDomElement elem = doc.createElement(QLatin1String("description"));
    elem.setAttribute(QLatin1String("name"), name);
    elem.appendChild(dom->write(doc));
    doc.appendChild(elem);
    file.write(doc.toString().toUtf8());

    file.close();

    theFileToBrush[filename] = name;
    theBrushToFile[name] = filename;

    delete dom;
}

void BrushManagerProxyPrivate::brushRemoved(const QString &name)
{
    QDir brushDir(theBrushFolder);

    QString filename = theBrushToFile[name];
    brushDir.remove(filename);
    theBrushToFile.remove(name);
    theFileToBrush.remove(filename);
}

QString BrushManagerProxyPrivate::uniqueBrushFileName(const QString &brushName) const
{
    QString filename = brushName.toLower() + QLatin1String(".br");
    int i = 0;
    while (theFileToBrush.contains(filename))
        filename = brushName.toLower() + QString::number(++i) + QLatin1String(".br");
    return filename;
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

            QFile file(fi.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray contents = file.readAll();
                file.close();
                QDomDocument doc;
                if (doc.setContent(contents)) {
                    DomBrush dom;
                    QDomElement domElement = doc.documentElement();

                    QString name = domElement.attribute(QLatin1String("name"));
                    QString filename = fi.fileName();

                    QSimpleResource resource(d_ptr->theCore);

                    QDomElement brushElement = domElement.firstChildElement(QLatin1String("brush"));
                    dom.read(brushElement);
                    QBrush br = resource.setupBrush(&dom);

                    d_ptr->theManager->addBrush(name, br);
                    d_ptr->theFileToBrush[filename] = name;
                    d_ptr->theBrushToFile[name] = filename;
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
        QColor red = QColor::fromRgb(255, 0, 0);
        QColor white = QColor::fromRgb(255, 255, 255);
        QColor darkBlue = QColor::fromRgb(5, 0, 70);
        QColor darkGreen = QColor::fromRgb(60, 160, 00);
        QColor blue = QColor::fromRgb(0, 0, 255);
        QColor black = QColor::fromRgb(0, 0, 0);
        QColor yellow = QColor::fromRgb(255, 255, 0);

        QLinearGradient gr1(0, 0, 1, 0);
        gr1.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr1.setColorAt(0.000, red);
        gr1.setColorAt(0.225, red);
        gr1.setColorAt(0.250, white);
        gr1.setColorAt(0.275, white);
        gr1.setColorAt(0.300, darkBlue);
        gr1.setColorAt(0.400, darkBlue);
        gr1.setColorAt(0.425, white);
        gr1.setColorAt(0.450, white);
        gr1.setColorAt(0.475, red);
        gr1.setColorAt(1.000, red);
        QBrush br1(gr1);

        QLinearGradient gr2(0, 0, 0, 1);
        gr2.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr2.setColorAt(0, white);
        gr2.setColorAt(0.475, white);
        gr2.setColorAt(0.525, red);
        gr2.setColorAt(1, red);
        QBrush br2(gr2);

        QLinearGradient gr3(0, 0, 0, 1);
        gr3.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr3.setColorAt(0, black);
        gr3.setColorAt(0.323, black);
        gr3.setColorAt(0.343, red);
        gr3.setColorAt(0.656, red);
        gr3.setColorAt(0.676, yellow);
        gr3.setColorAt(1, yellow);
        QBrush br3(gr3);

        QLinearGradient gr4(0, 0, 1, 0);
        gr4.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr4.setColorAt(0, blue);
        gr4.setColorAt(0.323, blue);
        gr4.setColorAt(0.343, white);
        gr4.setColorAt(0.656, white);
        gr4.setColorAt(0.676, red);
        gr4.setColorAt(1, red);
        QBrush br4(gr4);

        QLinearGradient gr5(0, 0, 1, 0);
        gr5.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr5.setColorAt(0, darkGreen);
        gr5.setColorAt(0.323, darkGreen);
        gr5.setColorAt(0.343, white);
        gr5.setColorAt(0.656, white);
        gr5.setColorAt(0.676, red);
        gr5.setColorAt(1, red);
        QBrush br5(gr5);

        QLinearGradient gr6(0, 0, 0, 1);
        gr6.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr6.setColorAt(0, red);
        gr6.setColorAt(0.24, red);
        gr6.setColorAt(0.26, yellow);
        gr6.setColorAt(0.74, yellow);
        gr6.setColorAt(0.76, red);
        gr6.setColorAt(1, red);
        QBrush br6(gr6);

        QRadialGradient gr7(0.5, 0.5, 0.5, 0.5, 0.5);
        gr7.setCoordinateMode(QGradient::StretchToDeviceMode);
        gr7.setColorAt(0, red);
        gr7.setColorAt(0.49, red);
        gr7.setColorAt(0.51, white);
        gr7.setColorAt(1, white);
        QBrush br7(gr7);

        manager->addBrush(QLatin1String("Norwegian"), br1);
        manager->addBrush(QLatin1String("Polish"), br2);
        manager->addBrush(QLatin1String("German"), br3);
        manager->addBrush(QLatin1String("French"), br4);
        manager->addBrush(QLatin1String("Italian"), br5);
        manager->addBrush(QLatin1String("Spanish"), br6);
        manager->addBrush(QLatin1String("Japanese"), br7);

        manager->addBrush(QLatin1String("red"), QBrush(Qt::red));
        manager->addBrush(QLatin1String("green"), QBrush(Qt::green));
        manager->addBrush(QLatin1String("blue"), QBrush(Qt::blue));
        manager->addBrush(QLatin1String("yellow"), QBrush(Qt::yellow));
        manager->addBrush(QLatin1String("magenta"), QBrush(Qt::magenta));
        manager->addBrush(QLatin1String("cyan"), QBrush(Qt::cyan));
        manager->addBrush(QLatin1String("black"), QBrush(Qt::black));
        manager->addBrush(QLatin1String("white"), QBrush(Qt::white));
    }
}

#include "moc_brushmanagerproxy.cpp"
