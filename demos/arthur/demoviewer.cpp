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

#include "demoviewer.h"
#include "demowidget.h"
#include "introscreen.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlistwidget.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qstackedwidget.h>
#include <qtextedit.h>
#include <qvboxwidget.h>
#include <qpixmapcache.h>

#include <qdebug.h>


class ItemDelegate : public QAbstractItemDelegate
{
public:
    ItemDelegate(QAbstractItemModel *model)
        : QAbstractItemDelegate(model)
    {
    }

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &options,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &,
                           const QModelIndex &) const
    {
        return QSize(100, 30);
    }
};

void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &options,
                         const QModelIndex &index) const
{
    QColor base = QApplication::palette().color(QPalette::Button);
    QColor pen = QApplication::palette().color(QPalette::ButtonText);

    QColor g1 = base.light();
    QColor g2 = base.dark();

    bool selected = (options.state & QStyle::State_Selected) != 0;

    if (selected) {
         base = QApplication::palette().color(QPalette::Highlight);
         pen = QApplication::palette().color(QPalette::HighlightedText);

        g2 = base.light();
        g1 = base.dark(120);
    }

    QRect r = options.rect;
    painter->fillRect(r, QBrush(r.topLeft(), g1, r.bottomLeft(), g2));

    painter->setPen(pen);
    QRect textRect = r;
    painter->drawText(textRect, Qt::AlignCenter, index.model()->data(index).toString());

    painter->setPen(selected ? Qt::white : Qt::black);
    painter->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
    painter->drawLine(r.right(), r.top()+1, r.right(), r.bottom()-1);

    painter->setPen(selected ? Qt::black : Qt::white);
    painter->drawLine(r.left(), r.top(), r.right(), r.top());
    painter->drawLine(r.left(), r.top()+1, r.left(), r.bottom()-1);
}


DemoViewer::DemoViewer(QWidget *parent)
    : QWidget(parent),
      attributes(new Attributes)
{
    QPixmapCache::setCacheLimit(32*1024);
    setWindowTitle(tr("Qt Paint Engine Demo"));
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);

    QSplitter *horSplit = new QSplitter(Qt::Horizontal, this);

    QVBoxWidget *vbox = new QVBoxWidget(horSplit);
    vbox->setMargin(6);
    vbox->setSpacing(6);
    QGroupBox *categories = new QGroupBox("Categories", vbox);
    QBoxLayout *glayout = new QBoxLayout(QBoxLayout::TopToBottom, categories);
    listWidget = new QListWidget(categories);
    glayout->addWidget(listWidget);

    layout->addWidget(horSplit);

    widgets = new QStackedWidget(horSplit);
    QGroupBox *opts = new QGroupBox("Options", vbox);
    QBoxLayout *props = new QBoxLayout(QBoxLayout::TopToBottom, opts);

    antialias = new QCheckBox(tr("Antialiasing"), opts);
    alpha = new QCheckBox(tr("Alphablended primitives"), opts);

    bgMode = new QComboBox(opts);
    QStringList items;
    items << tr("Solid Fill")
          << tr("Gradient Fill")
          << tr("Tiles")
          << tr("Background image");
    bgMode->insertStringList(items);

    props->addWidget(antialias);
    props->addWidget(alpha);
    props->addWidget(bgMode);
    props->addItem(new QSpacerItem(1, 1));

    viewSourceButton = new QPushButton("View Source", vbox);
    viewSourceButton->setCheckable(true);

    sourceViewer = new QTextEdit(widgets);
    sourceViewer->setReadOnly(true);
    QPalette pal = sourceViewer->palette();

    QColor base = pal.base().color();
    QColor background = pal.background().color();
    QColor betweenBaseAndBackground((base.red() + background.red())/2,
                                    (base.green() + background.green())/2,
                                    (base.blue() + background.blue())/2);
    pal.setBrush(QPalette::All, QPalette::Base, QBrush(QPoint(0, 0), pal.base().color(),
                                                       QPoint(0, 1000), betweenBaseAndBackground));
    sourceViewer->setPalette(pal);

    QApplication::sendPostedEvents();

    QList<int> l;
    l.append(100);
    l.append(700);
    horSplit->setSizes(l);

    // Setting it up...
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setItemDelegate(new ItemDelegate(listWidget->model()));
    connect(viewSourceButton, SIGNAL(toggled(bool)), this, SLOT(openSource(bool)));
    connect(listWidget->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(itemSelected()));
    connect(antialias, SIGNAL(toggled(bool)), this, SLOT(antialiasChanged(bool)));
    connect(alpha, SIGNAL(toggled(bool)), this, SLOT(alphaChanged(bool)));
    connect(bgMode, SIGNAL(activated(int)), this, SLOT(fillModeChanged(int)));

    antialias->setChecked(true);
    alpha->setChecked(true);
    bgMode->setCurrentItem(1);
    attributes->fillMode = Attributes::Gradient;
}

void DemoViewer::addDemoWidget(const QString &name, DemoWidget *widget, const QString &file)
{
    listWidget->appendItem(name);
    widget->setParent(widgets);
    widget->setAttributes(attributes);

    widgetByName[name] = widget;
    fileByName[name] = file;
}

QSize DemoViewer::sizeHint() const
{
    return QSize(800, 600);
}

void DemoViewer::itemSelected()
{
    if(viewSourceButton->isChecked()) {
        openSource(true);
        return;
    }

    QString name = listWidget->model()->data(listWidget->selectionModel()->currentIndex()).toString();

    if (name.isEmpty())
        return;

    DemoWidget *demoWidget = widgetByName[name];

    Q_ASSERT(demoWidget);

    DemoWidget *oldDemoWidget = qt_cast<DemoWidget*>(widgets->currentWidget());
    if (oldDemoWidget)
        oldDemoWidget->stopAnimation();

    widgets->setCurrentIndex(widgets->indexOf(demoWidget));
    demoWidget->startAnimation();
    viewSourceButton->setChecked(false);
}

void DemoViewer::antialiasChanged(bool val)
{
    attributes->antialias = val;
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::alphaChanged(bool val)
{
    attributes->alpha = val;
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::fillModeChanged(int mode)
{
    attributes->fillMode = static_cast<Attributes::BackgroundFill>(mode);
    if (DemoWidget *w = qt_cast<DemoWidget*>(widgets->currentWidget()))
        w->resetState();
}

void DemoViewer::showEvent(QShowEvent *)
{
    if (!listWidget->currentIndex().isValid()) {
        listWidget->selectionModel()->setCurrentIndex(listWidget->model()->index(0, 0, QModelIndex()),
						    QItemSelectionModel::ClearAndSelect);
    }
    itemSelected();
}

void DemoViewer::hideEvent(QHideEvent *)
{
    if (DemoWidget *demoWidget = qt_cast<DemoWidget*>(widgets->currentWidget()))
        demoWidget->stopAnimation();
}

void DemoViewer::openSource(bool on)
{
    if (!on) {
        itemSelected();
    } else {
        QString name =
            listWidget->model()->data(listWidget->selectionModel()->currentIndex()).toString();
        DemoWidget *oldDemoWidget = qt_cast<DemoWidget*>(widgets->currentWidget());
        if (oldDemoWidget)
            oldDemoWidget->stopAnimation();

        Q_ASSERT(!name.isEmpty());
        QString fileName = fileByName[name];
        QString contents;
        if (fileName.isEmpty()) {
            contents = QString("No source for widget: '%1'").arg(name);
        } else {
            QFile f(":/res/" + fileName);
            if (!f.open(QFile::ReadOnly))
                contents = QString("Could not open file: '%1'").arg(fileName);
            else
                contents = f.readAll();
        }

        contents.replace('&', "&amp;");
        contents.replace('<', "&lt;");
        contents.replace('>', "&gt;");

        // add some pretty unsofisticated syntax highlighting
        QStringList keywords;
        keywords << "for " << "if " << "switch " << " int " << "#include " << "const"
                 << "void " << "uint " << "case " << "double " << "#define " << "static"
                 << "#ifndef" << "#else" << "#endif" << "#ifdef" << "break" << "default"
                 << "return";
        for (int i = 0; i < keywords.size(); ++i)
            contents.replace(QRegExp("(\\s)" + keywords.at(i) + "(\\s)"),
                             QLatin1String("\\1<font color=blue><b>")
                             + keywords.at(i) + QLatin1String("</b></font>\\2"));
        contents.replace("(int ", "(<font color=blue><b>int </b></font>");

        QString html = "<pre>" + contents + "</pre>";

        sourceViewer->setHtml(html);
        widgets->setCurrentIndex(widgets->indexOf(sourceViewer));
    }
}
