#include "interactivewidget.h"

InteractiveWidget::InteractiveWidget()
{
    QHBoxLayout *hlayout = new QHBoxLayout(this);

    QSplitter *hsplit = new QSplitter(this);
    QSplitter *vsplit = new QSplitter(hsplit);
    vsplit->setOrientation(Qt::Vertical);

    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addWidget(hsplit);

    osw = new OnScreenWidget<QWidget>(vsplit);
    osw->setMinimumSize(320, 240);

    te = new QTextEdit(vsplit);
    te->installEventFilter(this);

    QWidget *panel = new QWidget(hsplit);
    QVBoxLayout *vlayout = new QVBoxLayout(panel);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    cmds = new QListWidget(panel);
    cmds->setMinimumSize(150, 150);
    cmds->addItem("clearRenderHint");
    cmds->addItem("drawArc");
    cmds->addItem("drawChord");
    cmds->addItem("drawEllipse");
    cmds->addItem("drawLine");
    cmds->addItem("drawPath");
    cmds->addItem("drawPie");
    cmds->addItem("drawPolygon");
    cmds->addItem("drawPolyline");
    cmds->addItem("drawRect");
    cmds->addItem("drawRoundRect");
    cmds->addItem("drawText");
    cmds->addItem("path_addEllipse");
    cmds->addItem("path_addPolygon");
    cmds->addItem("path_addRect");
    cmds->addItem("path_addText");
    cmds->addItem("path_arcTo");
    cmds->addItem("path_closeSubPath");
    cmds->addItem("path_createOutline");
    cmds->addItem("path_cubicTo");
    cmds->addItem("path_lineTo");
    cmds->addItem("path_moveTo");
    cmds->addItem("path_setFillRule");
    cmds->addItem("qt3_drawArc");
    cmds->addItem("qt3_drawChord");
    cmds->addItem("qt3_drawEllipse");
    cmds->addItem("qt3_drawPie");
    cmds->addItem("qt3_drawRect");
    cmds->addItem("qt3_drawRoundRect");
    cmds->addItem("region_addRect");
    cmds->addItem("region_getClipRegion");
    cmds->addItem("resetMatrix");
    cmds->addItem("restore");
    cmds->addItem("rotate");
    cmds->addItem("save");
    cmds->addItem("scale");
    cmds->addItem("mapQuadToQuad");
    cmds->addItem("setMatrix");
    cmds->addItem("setBackground");
    cmds->addItem("setBgMode");
    cmds->addItem("setBrush");
    cmds->addItem("setBrushOrigin");
    cmds->addItem("setClipPath");
    cmds->addItem("setClipRect");
    cmds->addItem("setClipRegion");
    cmds->addItem("setClipping");
    cmds->addItem("setFont");
    cmds->addItem("setPen");
    cmds->addItem("setRenderHint");
    cmds->addItem("translate");

    cmdMap.insert("clearRenderHint", "clearRenderHint");
    cmdMap.insert("drawArc", "drawArc 10 10 100 100 0 1500");
    cmdMap.insert("drawChord", "drawChord 10 10 100 100 0 1500");
    cmdMap.insert("drawEllipse", "drawEllipse 0 0 100 100");
    cmdMap.insert("drawLine", "drawLine 0 0 100 100");
    cmdMap.insert("drawPath", "drawPath path");
    cmdMap.insert("drawPie", "drawPie 10 10 100 100 0 1500");
    cmdMap.insert("drawPolygon", "drawPolygon [0 0 0 100 100 100]");
    cmdMap.insert("drawPolyline", "drawPolyline [0 0 0 100 100 100]");
    cmdMap.insert("drawRect", "drawRect 0 0 100 100");
    cmdMap.insert("drawRoundRect","drawRoundRect 0 0 100 100");
    cmdMap.insert("drawText", "drawText 0 20 \"Hello World\"");
    cmdMap.insert("path_addEllipse", "path_addEllipse path 20 20 60 60");
    cmdMap.insert("path_addPolygon","path_addPolygon path [ 0 0 0 100 100 100]");
    cmdMap.insert("path_addRect", "path_addRect path 0 0 100 100");
    cmdMap.insert("path_addText", "path_addText path 10 20 \"Hello World\"");
    cmdMap.insert("path_arcTo", "path_arcTo path 0 0 100 100 0 360");
    cmdMap.insert("path_closeSubpath", "path_closeSubpath path ");
    cmdMap.insert("path_createOutline", "path_createOutline path outline");
    cmdMap.insert("path_cubicTo", "path_cubicTo path 100 0 0 100 100 100");
    cmdMap.insert("path_lineTo", "path_lineTo path 100 100");
    cmdMap.insert("path_moveTo", "path_moveTo path 10 10");
    cmdMap.insert("path_setFillRule", "path_setFillRule path winding");
    cmdMap.insert("qt3_drawArc", "qt3_drawArc 10 10 100 100 0 1500");
    cmdMap.insert("qt3_drawChord", "qt3_drawChord 10 10 100 100 0 1500");
    cmdMap.insert("qt3_drawEllipse", "qt3_drawEllipse 0 0 100 100");
    cmdMap.insert("qt3_drawPie", "qt3_drawPie 10 10 100 100 0 1500");
    cmdMap.insert("qt3_drawRect", "qt3_drawRect 0 0 100 100");
    cmdMap.insert("qt3_drawRoundRect","qt3_drawRoundRect 0 0 100 100");
    cmdMap.insert("region_addRect", "region_addRect region 0 0 100 100");
    cmdMap.insert("region_getClipRegion", "region_getClipRegion region");
    cmdMap.insert("resetMatrix", "resetMatrix");
    cmdMap.insert("restore", "restore");
    cmdMap.insert("rotate", "rotate 10");
    cmdMap.insert("save", "save");
    cmdMap.insert("scale", "scale 2 2");
    cmdMap.insert("setBackground", "setBackground");
    cmdMap.insert("setBgMode", "setBgMode");
    cmdMap.insert("setBrush", "setBrush red");
    cmdMap.insert("setBrushOrigin", "setBrushOrigin 10 10");
    cmdMap.insert("setClipPath", "setClipPath path");
    cmdMap.insert("setClipRect", "setClipRect 0 0 100 100");
    cmdMap.insert("setClipRegion", "setClipRegion region");
    cmdMap.insert("setClipping", "setClipping ");
    cmdMap.insert("setFont", "setFont \"times\"");
    cmdMap.insert("setPen", "setPen black");
    cmdMap.insert("setRenderHint", "setRenderHint ");
    cmdMap.insert("translate", "translate 10 10");

    vlayout->addWidget(cmds);

    hlayout->setStretchFactor(panel, 0);
    hlayout->setStretchFactor(vsplit, 1);
    QPushButton *run = new QPushButton("&Run", panel);
    QPushButton *load = new QPushButton("&Load", panel);
    QPushButton *save = new QPushButton("&Save", panel);

    vlayout->addWidget(run);
    vlayout->addWidget(load);
    vlayout->addWidget(save);

    connect(cmds, SIGNAL(doubleClicked(QListWidgetItem *, Qt::MouseButton, Qt::KeyboardModifiers)),
            this, SLOT(cmdSelected(QListWidgetItem *)));
    connect(cmds, SIGNAL(returnPressed(QListWidgetItem *)),
            this, SLOT(cmdSelected(QListWidgetItem *)));
    connect(run, SIGNAL(clicked()), this, SLOT(run()));
    connect(load, SIGNAL(clicked()), this, SLOT(load()));
    connect(save, SIGNAL(clicked()), this, SLOT(save()));
}

void InteractiveWidget::run()
{
    osw->cmds.clear();
    QString script = te->toPlainText();
    QStringList lines = script.split("\n");
    for (int i = 0; i < lines.size(); ++i)
        osw->cmds.append(lines.at(i));
    osw->repaint();
}

void InteractiveWidget::cmdSelected(QListWidgetItem *item)
{
    te->insertPlainText(cmdMap[item->text()]);
    te->append(QString());
    te->setFocus();
}

void InteractiveWidget::load()
{
    QString fname = QFileDialog::getOpenFileName(
        this,
        QString("Load QPaintEngine Script"),
        QFileInfo(filename).absoluteFilePath(),
        QString("QPaintEngine Script (*.qps);;All files (*.*)"));
    
    load(fname);
}

void InteractiveWidget::load(const QString &fname)
{
    if (!fname.isEmpty()) {
        filename = fname;
        te->clear();
        QFile file(fname);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream textFile(&file);
        QString script = textFile.readAll();
        te->setPlainText(script);
        osw->filename = fname;
    }
}

void InteractiveWidget::save()
{
    QString script = te->toPlainText();
    if (!script.endsWith("\n"))
        script += QString("\n");
    QString fname = QFileDialog::getSaveFileName(this, QString("Save QPaintEngine Script"), QFileInfo(filename).absoluteFilePath(), QString("QPaintEngine Script (*.qps);;All files (*.*)"));
    if (!fname.isEmpty()) {
        filename = fname;
        QFile file(fname);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        QTextStream textFile(&file);
        textFile << script;
        osw->filename = fname;
    }    
}

bool InteractiveWidget::eventFilter(QObject *o, QEvent *e)
{
    if (qobject_cast<QTextEdit *>(o) && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Tab) {
            cmds->setFocus();
            return true;
        } else if (ke->key() == Qt::Key_Return
                   && ke->modifiers() == Qt::ControlModifier) {
            run();
            return true;
        }
    }
    return false;
}
