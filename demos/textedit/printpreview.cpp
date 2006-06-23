/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "printpreview.h"

#include <QAbstractScrollArea>
#include <QPrintDialog>
#include <QPrinter>
#include <QToolBar>
#include <QAction>
#include <QTextFormat>
#include <QMouseEvent>
#include <QTextFrame>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QPageSetupDialog>

#ifdef Q_WS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

static inline int inchesToPixels(float inches, QPaintDevice *device)
{
    return qRound(inches * device->logicalDpiY());
}

static inline qreal mmToInches(double mm)
{
    return mm*0.039370147;
}

#define Q_MM(n) int((n * 720 + 127) / 254)
#define Q_IN(n) int(n * 72)

static const struct Size {
   int width;
   int height;
} paperSizes[QPrinter::NPageSize] =
{
    {  Q_MM(210), Q_MM(297) },      // A4
    {  Q_MM(176), Q_MM(250) },      // B5
    {  Q_IN(8.5), Q_IN(11) },       // Letter
    {  Q_IN(8.5), Q_IN(14) },       // Legal
    {  Q_IN(7.5), Q_IN(10) },       // Executive
    {  Q_MM(841), Q_MM(1189) },     // A0
    {  Q_MM(594), Q_MM(841) },      // A1
    {  Q_MM(420), Q_MM(594) },      // A2
    {  Q_MM(297), Q_MM(420) },      // A3
    {  Q_MM(148), Q_MM(210) },      // A5
    {  Q_MM(105), Q_MM(148) },      // A6
    {  Q_MM(74), Q_MM(105)},        // A7
    {  Q_MM(52), Q_MM(74) },        // A8
    {  Q_MM(37), Q_MM(52) },        // A9
    {  Q_MM(1000), Q_MM(1414) },    // B0
    {  Q_MM(707), Q_MM(1000) },     // B1
    {  Q_MM(31), Q_MM(44) },        // B10
    {  Q_MM(500), Q_MM(707) },      // B2
    {  Q_MM(353), Q_MM(500) },      // B3
    {  Q_MM(250), Q_MM(353) },      // B4
    {  Q_MM(125), Q_MM(176) },      // B6
    {  Q_MM(88), Q_MM(125) },       // B7
    {  Q_MM(62), Q_MM(88) },        // B8
    {  Q_MM(44), Q_MM(62) },        // B9
    {  Q_MM(162),    Q_MM(229) },   // C5E
    {  Q_IN(4.125),  Q_IN(9.5) },   // Comm10E
    {  Q_MM(110),    Q_MM(220) },   // DLE
    {  Q_IN(8.5),    Q_IN(13) },    // Folio
    {  Q_IN(17),     Q_IN(11) },    // Ledger
    {  Q_IN(11),     Q_IN(17) }     // Tabloid
};

class PreviewView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    PreviewView(QTextDocument *document);

    inline void updateLayout() { resizeEvent(0); }

public slots:
    void zoomIn();
    void zoomOut();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private:
    void paintPage(QPainter *painter, int page);
    QTextDocument *doc;
    qreal scale;
    int interPageSpacing;
    QPoint mousePressPos;
    QPoint scrollBarValuesOnMousePress;
};

PreviewView::PreviewView(QTextDocument *document)
{
    verticalScrollBar()->setSingleStep(20);
    horizontalScrollBar()->setSingleStep(20);

    viewport()->setBackgroundRole(QPalette::Dark);

    doc = document;
    scale = 1.0;
    interPageSpacing = 30;
}

void PreviewView::zoomIn()
{
    scale += 0.2;
    resizeEvent(0);
    viewport()->update();
}

void PreviewView::zoomOut()
{
    scale -= 0.2;
    resizeEvent(0);
    viewport()->update();
}

void PreviewView::paintEvent(QPaintEvent *)
{
    QPainter p(viewport());

    p.translate(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
    p.translate(interPageSpacing, interPageSpacing);

    const int pages = doc->pageCount();
    for (int i = 0; i < pages; ++i) {
        p.save();
        p.scale(scale, scale);

        paintPage(&p, i);

        p.restore();
        p.translate(0, interPageSpacing + doc->pageSize().height() * scale);
    }
}

void PreviewView::paintPage(QPainter *painter, int page)
{
    const QSizeF pgSize = doc->pageSize();

    QColor col(Qt::black);

    painter->setPen(col);
    painter->setBrush(Qt::white);
    painter->drawRect(QRectF(QPointF(0, 0), pgSize));
    painter->setBrush(Qt::NoBrush);

    col = col.light();
    painter->drawLine(QLineF(pgSize.width(), 1, pgSize.width(), pgSize.height() - 1));

    col = col.light();
    painter->drawLine(QLineF(pgSize.width(), 2, pgSize.width(), pgSize.height() - 2));

    QRectF docRect(QPointF(0, page * pgSize.height()), pgSize);
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = docRect;
    painter->translate(0, - page * pgSize.height());
    painter->setClipRect(docRect);
    doc->documentLayout()->draw(painter, ctx);
}

void PreviewView::resizeEvent(QResizeEvent *)
{
    const QSize viewportSize = viewport()->size();

    QSize docSize;
    docSize.setWidth(qRound(doc->pageSize().width() * scale + 2 * interPageSpacing));
    const int pageCount = doc->pageCount();
    docSize.setHeight(qRound(pageCount * doc->pageSize().height() * scale + (pageCount + 1) * interPageSpacing));

    horizontalScrollBar()->setRange(0, docSize.width() - viewportSize.width());
    horizontalScrollBar()->setPageStep(viewportSize.width());

    verticalScrollBar()->setRange(0, docSize.height() - viewportSize.height());
    verticalScrollBar()->setPageStep(viewportSize.height());
}

void PreviewView::mousePressEvent(QMouseEvent *e)
{
    mousePressPos = e->pos();
    scrollBarValuesOnMousePress.rx() = horizontalScrollBar()->value();
    scrollBarValuesOnMousePress.ry() = verticalScrollBar()->value();
    e->accept();
}

void PreviewView::mouseMoveEvent(QMouseEvent *e)
{
    if (mousePressPos.isNull()) {
        e->ignore();
        return;
    }

    horizontalScrollBar()->setValue(scrollBarValuesOnMousePress.x() - e->pos().x() + mousePressPos.x());
    verticalScrollBar()->setValue(scrollBarValuesOnMousePress.y() - e->pos().y() + mousePressPos.y());
    horizontalScrollBar()->update();
    verticalScrollBar()->update();
    e->accept();
}

void PreviewView::mouseReleaseEvent(QMouseEvent *e)
{
    mousePressPos = QPoint();
    e->accept();
}

PrintPreview::PrintPreview(const QTextDocument *document, QWidget *parent)
    : QMainWindow(parent), printer(QPrinter::HighResolution)
{
    printer.setFullPage(true);
    doc = document->clone();

    view = new PreviewView(doc);
    setCentralWidget(view);
    resize(800, 600);

    doc->setUseDesignMetrics(true);
    doc->documentLayout()->setPaintDevice(view->viewport());

    // add a nice 2 cm margin
    const qreal margin = inchesToPixels(mmToInches(20), this);
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(margin);
    doc->rootFrame()->setFrameFormat(fmt);

    setup();

    QFont f(doc->defaultFont());
    f.setPointSize(10);
    doc->setDefaultFont(f);

    QToolBar *tb = addToolBar(tr("Print"));
    tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QAction *a;
    a = new QAction(/*QIcon(rsrcPath + "/fileprint.png"),*/ tr("&Print..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(a, SIGNAL(triggered()), this, SLOT(print()));
    tb->addAction(a);

    a = new QAction(this);
    a->setText(tr("Page Setup..."));
    connect(a, SIGNAL(triggered()), this, SLOT(pageSetup()));
    tb->addAction(a);

    tb->addSeparator();

    a = new QAction(QIcon(rsrcPath + "/zoomin.png"), tr("Zoom In"), this);
    connect(a, SIGNAL(triggered()), view, SLOT(zoomIn()));
    tb->addAction(a);

    a = new QAction(QIcon(rsrcPath + "/zoomout.png"), tr("Zoom Out"), this);
    connect(a, SIGNAL(triggered()), view, SLOT(zoomOut()));
    tb->addAction(a);

    tb->addSeparator();

    a = new QAction(this);
    a->setText(tr("&Close"));
    connect(a, SIGNAL(triggered()), this, SLOT(close()));
    tb->addAction(a);
}

void PrintPreview::setup()
{
    QSizeF page(paperSizes[printer.pageSize()].width,
                paperSizes[printer.pageSize()].height);
    page.setWidth(page.width() * view->logicalDpiX() / 72.);
    page.setHeight(page.height() * view->logicalDpiY() / 72.);

    if (printer.orientation() == QPrinter::Landscape) {
        qSwap(page.rwidth(), page.rheight());
    }

    // add a nice 2 cm margin
    const qreal margin = inchesToPixels(mmToInches(20), this);
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(margin);
    doc->rootFrame()->setFrameFormat(fmt);

    doc->setPageSize(page);
}

PrintPreview::~PrintPreview()
{
    delete doc;
}

void PrintPreview::print()
{
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
        doc->print(&printer);
    }
    delete dlg;
}

void PrintPreview::pageSetup()
{
    QPageSetupDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted) {
        setup();
        view->updateLayout();
    }
}

#include "printpreview.moc"
