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

    m_onScreenWidget = new OnScreenWidget<QWidget>(vsplit);
    m_onScreenWidget->setMinimumSize(320, 240);

    ui_textEdit = new QTextEdit(vsplit);
    ui_textEdit->installEventFilter(this);

    QWidget *panel = new QWidget(hsplit);
    QVBoxLayout *vlayout = new QVBoxLayout(panel);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    m_cmds = new QListWidget(panel);
    m_cmds->setMinimumSize(150, 150);

    (new QListWidgetItem("clearRenderHint", m_cmds))->setToolTip("bordel de mouille");
    (new QListWidgetItem("drawArc", m_cmds))->setToolTip("bordel d'arc");
    //cmds->addItem("clearRenderHint");
    //cmds->addItem("drawArc");
    m_cmds->addItem("drawChord");
    m_cmds->addItem("drawEllipse");
    m_cmds->addItem("drawLine");
    m_cmds->addItem("drawPath");
    m_cmds->addItem("drawPie");
    m_cmds->addItem("drawPolygon");
    m_cmds->addItem("drawPolyline");
    m_cmds->addItem("drawRect");
    m_cmds->addItem("drawRoundRect");
    m_cmds->addItem("drawText");
    m_cmds->addItem("path_addEllipse");
    m_cmds->addItem("path_addPolygon");
    m_cmds->addItem("path_addRect");
    m_cmds->addItem("path_addText");
    m_cmds->addItem("path_arcTo");
    m_cmds->addItem("path_closeSubPath");
    m_cmds->addItem("path_createOutline");
    m_cmds->addItem("path_cubicTo");
    m_cmds->addItem("path_lineTo");
    m_cmds->addItem("path_moveTo");
    m_cmds->addItem("path_setFillRule");
    m_cmds->addItem("qt3_drawArc");
    m_cmds->addItem("qt3_drawChord");
    m_cmds->addItem("qt3_drawEllipse");
    m_cmds->addItem("qt3_drawPie");
    m_cmds->addItem("qt3_drawRect");
    m_cmds->addItem("qt3_drawRoundRect");
    m_cmds->addItem("region_addRect");
    m_cmds->addItem("region_getClipRegion");
    m_cmds->addItem("resetMatrix");
    m_cmds->addItem("restore");
    m_cmds->addItem("rotate");
    m_cmds->addItem("save");
    m_cmds->addItem("scale");
    m_cmds->addItem("mapQuadToQuad");
    m_cmds->addItem("setMatrix");
    m_cmds->addItem("setBackground");
    m_cmds->addItem("setBgMode");
    m_cmds->addItem("setBrush");
    m_cmds->addItem("setBrushOrigin");
    m_cmds->addItem("setClipPath");
    m_cmds->addItem("setClipRect");
    m_cmds->addItem("setClipRegion");
    m_cmds->addItem("setClipping");
    m_cmds->addItem("setFont");
    m_cmds->addItem("setPen");
    m_cmds->addItem("setRenderHint");
    m_cmds->addItem("translate");

    m_cmdMap.insert("clearRenderHint", "clearRenderHint");
    m_cmdMap.insert("drawArc", "drawArc 10 10 100 100 0 1500");
    m_cmdMap.insert("drawChord", "drawChord 10 10 100 100 0 1500");
    m_cmdMap.insert("drawEllipse", "drawEllipse 0 0 100 100");
    m_cmdMap.insert("drawLine", "drawLine 0 0 100 100");
    m_cmdMap.insert("drawPath", "drawPath path");
    m_cmdMap.insert("drawPie", "drawPie 10 10 100 100 0 1500");
    m_cmdMap.insert("drawPolygon", "drawPolygon [0 0 0 100 100 100]");
    m_cmdMap.insert("drawPolyline", "drawPolyline [0 0 0 100 100 100]");
    m_cmdMap.insert("drawRect", "drawRect 0 0 100 100");
    m_cmdMap.insert("drawRoundRect","drawRoundRect 0 0 100 100");
    m_cmdMap.insert("drawText", "drawText 0 20 \"Hello World\"");
    m_cmdMap.insert("path_addEllipse", "path_addEllipse path 20 20 60 60");
    m_cmdMap.insert("path_addPolygon","path_addPolygon path [ 0 0 0 100 100 100]");
    m_cmdMap.insert("path_addRect", "path_addRect path 0 0 100 100");
    m_cmdMap.insert("path_addText", "path_addText path 10 20 \"Hello World\"");
    m_cmdMap.insert("path_arcTo", "path_arcTo path 0 0 100 100 0 360");
    m_cmdMap.insert("path_closeSubpath", "path_closeSubpath path ");
    m_cmdMap.insert("path_createOutline", "path_createOutline path outline");
    m_cmdMap.insert("path_cubicTo", "path_cubicTo path 100 0 0 100 100 100");
    m_cmdMap.insert("path_lineTo", "path_lineTo path 100 100");
    m_cmdMap.insert("path_moveTo", "path_moveTo path 10 10");
    m_cmdMap.insert("path_setFillRule", "path_setFillRule path winding");
    m_cmdMap.insert("qt3_drawArc", "qt3_drawArc 10 10 100 100 0 1500");
    m_cmdMap.insert("qt3_drawChord", "qt3_drawChord 10 10 100 100 0 1500");
    m_cmdMap.insert("qt3_drawEllipse", "qt3_drawEllipse 0 0 100 100");
    m_cmdMap.insert("qt3_drawPie", "qt3_drawPie 10 10 100 100 0 1500");
    m_cmdMap.insert("qt3_drawRect", "qt3_drawRect 0 0 100 100");
    m_cmdMap.insert("qt3_drawRoundRect","qt3_drawRoundRect 0 0 100 100");
    m_cmdMap.insert("region_addRect", "region_addRect region 0 0 100 100");
    m_cmdMap.insert("region_getClipRegion", "region_getClipRegion region");
    m_cmdMap.insert("resetMatrix", "resetMatrix");
    m_cmdMap.insert("restore", "restore");
    m_cmdMap.insert("rotate", "rotate 10");
    m_cmdMap.insert("save", "save");
    m_cmdMap.insert("scale", "scale 2 2");
    m_cmdMap.insert("setBackground", "setBackground");
    m_cmdMap.insert("setBgMode", "setBgMode");
    m_cmdMap.insert("setBrush", "setBrush red");
    m_cmdMap.insert("setBrushOrigin", "setBrushOrigin 10 10");
    m_cmdMap.insert("setClipPath", "setClipPath path");
    m_cmdMap.insert("setClipRect", "setClipRect 0 0 100 100");
    m_cmdMap.insert("setClipRegion", "setClipRegion region");
    m_cmdMap.insert("setClipping", "setClipping ");
    m_cmdMap.insert("setFont", "setFont \"times\"");
    m_cmdMap.insert("setPen", "setPen black");
    m_cmdMap.insert("setRenderHint", "setRenderHint ");
    m_cmdMap.insert("translate", "translate 10 10");

    vlayout->addWidget(m_cmds);

    hlayout->setStretchFactor(panel, 0);
    hlayout->setStretchFactor(vsplit, 1);
    QPushButton *run = new QPushButton("&Run", panel);
    QPushButton *load = new QPushButton("&Load", panel);
    QPushButton *save = new QPushButton("&Save", panel);

    vlayout->addWidget(run);
    vlayout->addWidget(load);
    vlayout->addWidget(save);

    connect(m_cmds, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(cmdSelected(QListWidgetItem*)));
    connect(run, SIGNAL(clicked()), SLOT(run()));
    connect(load, SIGNAL(clicked()), SLOT(load()));
    connect(save, SIGNAL(clicked()), SLOT(save()));
}

void InteractiveWidget::run()
{
    m_onScreenWidget->m_commands.clear();
    QString script = ui_textEdit->toPlainText();
    QStringList lines = script.split("\n");
    for (int i = 0; i < lines.size(); ++i)
        m_onScreenWidget->m_commands.append(lines.at(i));
    m_onScreenWidget->repaint();
}

void InteractiveWidget::cmdSelected(QListWidgetItem *item)
{
    ui_textEdit->insertPlainText(m_cmdMap[item->text()]);
    ui_textEdit->append(QString());
    ui_textEdit->setFocus();
}

void InteractiveWidget::load()
{
    QString fname = QFileDialog::getOpenFileName(
        this,
        QString("Load QPaintEngine Script"),
        QFileInfo(m_filename).absoluteFilePath(),
        QString("QPaintEngine Script (*.qps);;All files (*.*)"));
    
    load(fname);
}

void InteractiveWidget::load(const QString &fname)
{
    if (!fname.isEmpty()) {
        m_filename = fname;
        ui_textEdit->clear();
        QFile file(fname);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream textFile(&file);
        QString script = textFile.readAll();
        ui_textEdit->setPlainText(script);
        m_onScreenWidget->m_filename = fname;
    }
}

void InteractiveWidget::save()
{
    QString script = ui_textEdit->toPlainText();
    if (!script.endsWith("\n"))
        script += QString("\n");
    QString fname = QFileDialog::getSaveFileName(this, QString("Save QPaintEngine Script"), QFileInfo(m_filename).absoluteFilePath(), QString("QPaintEngine Script (*.qps);;All files (*.*)"));
    if (!fname.isEmpty()) {
        m_filename = fname;
        QFile file(fname);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        QTextStream textFile(&file);
        textFile << script;
        m_onScreenWidget->m_filename = fname;
    }    
}

bool InteractiveWidget::eventFilter(QObject *o, QEvent *e)
{
    if (qobject_cast<QTextEdit *>(o) && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Tab) {
            m_cmds->setFocus();
            return true;
        } else if (ke->key() == Qt::Key_Return
                   && ke->modifiers() == Qt::ControlModifier) {
            run();
            return true;
        }
    }
    return false;
}
