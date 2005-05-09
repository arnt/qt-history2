#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmapcache.h>
#include <qevent.h>
#include <qtextdocument.h>
#include <qabstracttextdocumentlayout.h>
#include <qfile.h>
#include <qtextbrowser.h>
#include <qboxlayout.h>

#include "arthurwidgets.h"

extern QPixmap cached(const QString &img);

ArthurFrame::ArthurFrame(QWidget *parent)
    : QWidget(parent)
{
    m_document = 0;
    m_show_doc = false;

    m_tile = QPixmap(100, 100);
    m_tile.fill(Qt::white);
    QPainter pt(&m_tile);
    QColor color(240, 240, 240);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();

//     QPalette pal = palette();
//     pal.setBrush(backgroundRole(), m_tile);
//     setPalette(pal);
}

// #define IMAGE
void ArthurFrame::paintEvent(QPaintEvent *e)
{

#ifdef IMAGE
    static QImage image;
    if (image.size() != size())
        image = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&image);
#else
    QPainter painter(this);
#endif

    painter.setClipRect(e->rect());

    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath clipPath;

    QRect r = rect();
    double left = r.x() + 1;
    double top = r.y() + 1;
    double right = r.right();
    double bottom = r.bottom();
    double radius2 = 8 * 2;

    clipPath.moveTo(right - radius2, top);
    clipPath.arcTo(right - radius2, top, radius2, radius2, 90, -90);
    clipPath.arcTo(right - radius2, bottom - radius2, radius2, radius2, 0, -90);
    clipPath.arcTo(left, bottom - radius2, radius2, radius2, 270, -90);
    clipPath.arcTo(left, top, radius2, radius2, 180, -90);
    clipPath.closeSubpath();

    painter.save();
    painter.setClipPath(clipPath, Qt::IntersectClip);

    painter.drawTiledPixmap(rect(), m_tile);

    // client painting

    paint(&painter);

    painter.restore();

    painter.save();
    if (m_show_doc)
        paintDescription(&painter);
    painter.restore();

    int level = 180;
    painter.setPen(QPen(QColor(level, level, level), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(clipPath);

#ifdef IMAGE
    painter.end();
    painter.begin(this);
    painter.drawImage(e->rect(), image, e->rect());
#endif
    return;

    // draw frame
    {
        QPixmap topLeft = cached(":res/images/frame_topleft.png");
        QPixmap topRight = cached(":res/images/frame_topright.png");
        QPixmap bottomLeft = cached(":res/images/frame_bottomleft.png");
        QPixmap bottomRight = cached(":res/images/frame_bottomright.png");
        QPixmap leftStretch = cached(":res/images/frame_left.png");
        QPixmap topStretch = cached(":res/images/frame_top.png");
        QPixmap rightStretch = cached(":res/images/frame_right.png");
        QPixmap bottomStretch = cached(":res/images/frame_bottom.png");

        int topFrameOffset = 0;//titleStretch.height()/2 - 2;
        painter.drawPixmap(r.topLeft() + QPoint(0, topFrameOffset), topLeft);
        painter.drawPixmap(r.topRight() - QPoint(topRight.width()-1, 0)  + QPoint(0, topFrameOffset), topRight);
        painter.drawPixmap(r.bottomLeft() - QPoint(0, bottomLeft.height()-1), bottomLeft);
        painter.drawPixmap(r.bottomRight() - QPoint(bottomRight.width()-1, bottomRight.height()-1), bottomRight);

        QRect left = r;
        left.setY(r.y() + topLeft.height() + topFrameOffset);
        left.setWidth(leftStretch.width());
        left.setHeight(r.height() - topLeft.height() - bottomLeft.height() - topFrameOffset);
        painter.drawTiledPixmap(left, leftStretch);

        QRect top = r;
        top.setX(r.x() + topLeft.width());
        top.setY(r.y() + topFrameOffset);
        top.setWidth(r.width() - topLeft.width() - topRight.width());
        top.setHeight(topLeft.height());
        painter.drawTiledPixmap(top, topStretch);

        QRect right = r;
        right.setX(r.right() - rightStretch.width()+1);
        right.setY(r.y() + topRight.height() + topFrameOffset);
        right.setWidth(rightStretch.width());
        right.setHeight(r.height() - topRight.height() - bottomRight.height() - topFrameOffset);
        painter.drawTiledPixmap(right, rightStretch);

        QRect bottom = r;
        bottom.setX(r.x() + bottomLeft.width());
        bottom.setY(r.bottom() - bottomStretch.height()+1);
        bottom.setWidth(r.width() - bottomLeft.width() - bottomRight.width());
        bottom.setHeight(bottomLeft.height());
        painter.drawTiledPixmap(bottom, bottomStretch);
    }
}

void ArthurFrame::setDescriptionEnabled(bool enabled)
{
    if (m_show_doc != enabled) {
        m_show_doc = enabled;
        emit descriptionEnabledChanged(m_show_doc);
        update();
    }
}

void ArthurFrame::loadDescription(const QString &fileName)
{
    QFile textFile(fileName);
    QString text;
    if (!textFile.open(QFile::ReadOnly))
        text = QString("Unable to load resource file: '%1'").arg(fileName);
    else
        text = textFile.readAll();
    setDescription(text);
}


void ArthurFrame::setDescription(const QString &text)
{
    m_document = new QTextDocument(this);
    m_document->setHtml(text);
}

void ArthurFrame::paintDescription(QPainter *painter)
{
    if (!m_document)
        return;

    int pageWidth = qMax(width() - 100, 100);
    int pageHeight = qMax(height() - 100, 100);
    if (pageWidth != m_document->pageSize().width()) {
        m_document->setPageSize(QSize(pageWidth, pageHeight));
    }

    QRect textRect(width() / 2 - pageWidth / 2,
                   height() / 2 - pageHeight / 2,
                   pageWidth,
                   pageHeight);
    int pad = 10;
    QRect clearRect = textRect.adjusted(-pad, -pad, pad, pad);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 63));
    int shade = 10;
    painter->drawRect(clearRect.x() + clearRect.width() + 1,
                      clearRect.y() + shade,
                      shade,
                      clearRect.height() + 1);
    painter->drawRect(clearRect.x() + shade,
                      clearRect.y() + clearRect.height() + 1,
                      clearRect.width() - shade + 1,
                      shade);

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(QColor(255, 255, 255, 220));
    painter->setPen(Qt::black);
    painter->drawRect(clearRect);

    painter->setClipRegion(textRect, Qt::IntersectClip);
    painter->translate(textRect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;

    QLinearGradient g(0, 0, 0, textRect.height());
    g.setColorAt(0, Qt::black);
    g.setColorAt(0.9, Qt::black);
    g.setColorAt(1, Qt::transparent);

    QPalette pal = palette();
    pal.setBrush(QPalette::Text, g);

    ctx.palette = pal;
    ctx.clip = QRect(0, 0, textRect.width(), textRect.height());
    m_document->documentLayout()->draw(painter, ctx);
}

void ArthurFrame::loadSourceFile(const QString &sourceFile)
{
    m_sourceFileName = sourceFile;
}

void ArthurFrame::showSource()
{
    // Check for existing source
    QObjectList c = children();
    for (int i=0; i<c.size(); ++i)
        if (qobject_cast<QTextBrowser *>(c.at(i)))
            return;

    QString contents;
    if (m_sourceFileName.isEmpty()) {
        contents = QString("No source for widget: '%1'").arg(objectName());
    } else {
        QFile f(m_sourceFileName);
        if (!f.open(QFile::ReadOnly))
            contents = QString("Could not open file: '%1'").arg(m_sourceFileName);
        else
            contents = f.readAll();
    }

    contents.replace('&', "&amp;");
    contents.replace('<', "&lt;");
    contents.replace('>', "&gt;");

    QString html = "<html><pre>" + contents + "</pre></html>";

    QTextBrowser *sourceViewer = new QTextBrowser(0);
    sourceViewer->setWindowTitle("Source: " + m_sourceFileName.mid(5));
    sourceViewer->setParent(this, Qt::Dialog);
    sourceViewer->setAttribute(Qt::WA_DeleteOnClose);
    sourceViewer->setHtml(html);
    sourceViewer->resize(600, 600);
    sourceViewer->show();
}
