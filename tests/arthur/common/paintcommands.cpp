#include "paintcommands.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtextstream.h>
#include <qtextlayout.h>
#include <qdebug.h>

const char *brushStyleTable[] = {
    "NoBrush",
    "SolidPattern",
    "Dense1Pattern",
    "Dense2Pattern",
    "Dense3Pattern",
    "Dense4Pattern",
    "Dense5Pattern",
    "Dense6Pattern",
    "Dense7Pattern",
    "HorPattern",
    "VerPattern",
    "CrossPattern",
    "BDiagPattern",
    "FDiagPattern",
    "DiagCrossPattern",
    "LinearGradientPattern"
};

const char *penStyleTable[] = {
    "NoPen",
    "SolidLine",
    "DashLine",
    "DotLine",
    "DashDotLine",
    "DashDotDotLine"
};

const char *fontWeightTable[] = {
    "Light",
    "Normal",
    "DemiBold",
    "Bold",
    "Black"
};

const char *clipOperationTable[] = {
    "NoClip",
    "ReplaceClip",
    "IntersectClip",
    "UniteClip"
};

const char *spreadMethodTable[] = {
    "PadSpread",
    "ReflectSpread",
    "RepeatSpread"
};

const char *compositionModeTable[] = {
    "SourceOver",
    "DestinationOver",
    "Clear",
    "Source",
    "Destination",
    "SourceIn",
    "DestinationIn",
    "SourceOut",
    "DestinationOut",
    "SourceAtop",
    "DestinationAtop",
    "Xor"
};

const char *imageFormatTable[] = {
    "Invalid",
    "Mono",
    "MonoLSB",
    "Indexed8",
    "RGB32",
    "ARGB32",
    "ARGB32_Premultiplied"
};

static int translate_enum(const char *table[], const QString &pattern, int limit)
{
    for (int i=0; i<limit; ++i)
        if (pattern.toLower() == QString(table[i]).toLower())
            return i;
    return -1;
}

typedef void (PaintCommands::*qPaintCommand) (QRegExp re);

struct PaintCommand {
    PaintCommand(QRegExp re, qPaintCommand cmd)
        : regExp(re), command(cmd)
    {
        Q_ASSERT(re.isValid());
    }

    QRegExp regExp;
    qPaintCommand command;
};

static QList<PaintCommand> commandTable;

void PaintCommands::runCommand(const QString &command)
{
    for (int i=0; i<commandTable.size(); ++i) {
        const PaintCommand &cmd = commandTable.at(i);
        if (cmd.regExp.indexIn(command) >= 0) {
            (this->*(cmd.command))(cmd.regExp);
            return;
        }
    }
}

void PaintCommands::runCommands()
{
    if (commandTable.isEmpty()) {
        static QRegExp comment("^\\s*#");
        static QRegExp import("import\\s+\"(.*)\"");

        static QRegExp begin_block("begin_block\\s+(\\w*)");
        static QRegExp end_block("end_block");
        static QRegExp repeat_block("repeat_block\\s+(\\w*)");

        static QRegExp drawEllipse("drawEllipse\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp drawRect("drawRect\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp drawRoundRect("drawRoundRect\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s*(-?\\w)?\\s*(-?\\w)?");
        static QRegExp drawPie("drawPie\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp drawChord("drawChord\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp drawArc("drawArc\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp qt3_drawEllipse("qt3_drawEllipse\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp qt3_drawRect("qt3_drawRect\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp qt3_drawRoundRect("qt3_drawRoundRect\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s*(-?\\w)?\\s*(-?\\w)?");
        static QRegExp qt3_drawPie("qt3_drawPie\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp qt3_drawChord("qt3_drawChord\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp qt3_drawArc("qt3_drawArc\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp drawLine("drawLine\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp drawPath("drawPath\\s+(\\w*)");
        static QRegExp drawPixmap("drawPixmap\\s+([\\w.:\\-/]*)"
                                  "\\s+(-?\\w*)\\s+(-?\\w*)\\s*(-?\\w*)?\\s*(-?\\w*)?"    // target rect
                                  "\\s*(-?\\w*)?\\s*(-?\\w*)?\\s*(-?\\w*)?\\s*(-?\\w*)?");  // source rect
        static QRegExp drawImage("drawImage\\s+([\\w.:\\/]*)"
                                 "\\s+(-?\\w*)\\s+(-?\\w*)\\s*(-?\\w*)?\\s*(-?\\w*)?"    // target rect
                                 "\\s*(-?\\w*)?\\s*(-?\\w*)?\\s*(-?\\w*)?\\s*(-?\\w*)?");  // source rect
        static QRegExp drawPoint("drawPoint\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp drawPolygon("drawPolygon\\s+\\[([\\w\\s-.]*)\\]\\s*(\\w*)");
        static QRegExp drawConvexPolygon("drawConvexPolygon\\s+\\[([\\w\\s-.]*)\\]");
        static QRegExp drawPolyline("drawPolyline\\s+\\[([\\w\\s]*)\\]");
        static QRegExp drawText("drawText\\s+(-?\\w*)\\s+(-?\\w*)\\s+\"(.*)\"");
        static QRegExp drawTiledPixmap("drawTiledPixmap\\s+([\\w.:\\/]*)"
                                       "\\s+(-?\\w*)\\s+(-?\\w*)\\s*(-?\\w*)\\s*(-?\\w*)"
                                       "\\s*(-?\\w*)\\s*(-?\\w*)");
        static QRegExp empty(".*");
        static QRegExp path_addEllipse("path_addEllipse\\s+(\\w*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_addRect("path_addRect\\s+(\\w*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_addPolygon("path_addPolygon\\s+(\\w*)\\s+\\[([\\w\\s]*)\\]\\s*(\\w*)");
        static QRegExp path_addText("path_addText\\s+(\\w*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+\"(.*)\"");
        static QRegExp path_arcTo("path_arcTo\\s+(\\w*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_cubicTo("path_cubicTo\\s+(\\w*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_lineTo("path_lineTo\\s+([.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_moveTo("path_moveTo\\s+([.\\w]*)\\s+(-?[.\\w]*)\\s+(-?[.\\w]*)");
        static QRegExp path_createOutline("path_createOutline\\s+(\\w*)\\s+(\\w*)");
        static QRegExp path_getClipPath("path_getClipPath\\s+([\\w0-9]*)");
        static QRegExp path_closeSubpath("path_closeSubpath\\s+(\\w*)");
        static QRegExp path_setFillRule("path_setFillRule\\s+(\\w*)\\s+(\\w*)");
        static QRegExp path_debugPrint("path_debugPrint\\s+(\\w*)");

        static QRegExp pen_setDashPattern("pen_setDashPattern\\s+\\[([\\w\\s.]*)\\]");

        static QRegExp region_addRect("region_addRect\\s+(\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp region_addEllipse("region_addEllipse\\s+(\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)");
        static QRegExp region_getClipRegion("region_getClipRegion\\s+(\\w*)");
        static QRegExp resetMatrix("resetMatrix");
        static QRegExp restore("restore");
        static QRegExp rotate("rotate\\s+(-?[\\w.]*)");
        static QRegExp save("save");
        static QRegExp scale("scale\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp setBackground("setBackground\\s+#?(\\w*)\\s*(\\w*)?");
        static QRegExp setBgMode("setBackgroundMode\\s+(\\w*)");
        static QRegExp setBrush("setBrush\\s+(#?[\\w.:\\/]*)\\s*(\\w*)?");
        static QRegExp setBrushOrigin("setBrushOrigin\\s*(-?\\w*)\\s+(-?\\w*)");
        static QRegExp setClipPath("setClipPath\\s+(\\w*)\\s*(\\w*)");
        static QRegExp setClipRect("setClipRect\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s*(\\w*)");
        static QRegExp setClipRectangle("setClipRectangle\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s+(-?\\w*)\\s*(\\w*)");
        static QRegExp setClipRegion("setClipRegion\\s+(\\w*)\\s*(\\w*)");
        static QRegExp setClipping("setClipping\\s+(\\w*)");
        static QRegExp setFont("setFont\\s+\"([\\w\\s]*)\"\\s*(\\w*)\\s*(\\w*)\\s*(\\w*)");
        static QRegExp setPen("setPen\\s+#?(\\w*)");
        static QRegExp setPen2("setPen\\s+(#?\\w*)\\s+([\\w.]+)\\s*(\\w*)\\s*(\\w*)\\s*(\\w*)");
        static QRegExp setRenderHint("setRenderHint\\s+([\\w_0-9]*)\\s*(\\w*)");
        static QRegExp setCompositionMode("setCompositionMode\\s+([\\w_0-9]*)");

        static QRegExp surface_begin("surface_begin\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp surface_end("surface_end");

        static QRegExp gradient_clearStops("gradient_clearStop");
        static QRegExp gradient_appendStop("gradient_appendStop\\s+([\\w.]*)\\s+#?(\\w*)");
        static QRegExp gradient_setLinear("gradient_setLinear\\s+([\\w.]*)\\s+([\\w.]*)\\s+([\\w.]*)\\s+([\\w.]*)");
        static QRegExp gradient_setRadial("gradient_setRadial\\s+([\\w.]*)\\s+([\\w.]*)\\s+([\\w.]*)\\s?([\\w.]*)\\s?([\\w.]*)");
        static QRegExp gradient_setConical("gradient_setConical\\s+([\\w.]*)\\s+([\\w.]*)\\s+([\\w.]*)");
        static QRegExp gradient_setSpread("gradient_setSpread\\s+(\\w*)");

        static QRegExp translate("translate\\s+(-?[\\w.]*)\\s+(-?[\\w.]*)");
        static QRegExp pixmap_load("pixmap_load\\s+([\\w.:\\/]*)\\s*([\\w.:\\/]*)");
        static QRegExp bitmap_load("bitmap_load\\s+([\\w.:\\/]*)\\s*([\\w.:\\/]*)");
        static QRegExp pixmap_setMask("pixmap_setMask\\s+([\\w.:\\/]*)\\s+([\\w.:\\/]*)");
        static QRegExp image_load("image_load\\s+([\\w.:\\/]*)\\s*([\\w.:\\/]*)");
        static QRegExp image_setNumColors("image_setNumColors\\s+([\\w.:\\/]*)\\s+([0-9]*)");
        static QRegExp image_setColor("image_setColor\\s+([\\w.:\\/]*)\\s+([0-9]*)\\s+#([0-9]*)");
        static QRegExp image_convertToFormat("image_convertToFormat\\s+([\\w.:\\/]*)\\s+([\\w.:\\/]+)\\s+([\\w0-9_]*)");

        static QRegExp textlayout_draw("textlayout_draw\\s+\"(.*)\"\\s+([0-9.]*)");

        static QRegExp abort("^abort");

        // Skip comments first of all
        commandTable.append(PaintCommand(comment,             &PaintCommands::command_comment));

        // Importer
        commandTable.append(PaintCommand(import,              &PaintCommands::command_import));

        // Block commands
        commandTable.append(PaintCommand(begin_block,         &PaintCommands::command_begin_block));
        commandTable.append(PaintCommand(end_block,           &PaintCommands::command_end_block));
        commandTable.append(PaintCommand(repeat_block,        &PaintCommands::command_repeat_block));

        // Setters
        commandTable.append(PaintCommand(setBgMode,           &PaintCommands::command_setBgMode));
        commandTable.append(PaintCommand(setBackground,       &PaintCommands::command_setBackground));
        commandTable.append(PaintCommand(setBrush,            &PaintCommands::command_setBrush));
        commandTable.append(PaintCommand(setBrushOrigin,      &PaintCommands::command_setBrushOrigin));
        commandTable.append(PaintCommand(setClipPath,         &PaintCommands::command_setClipPath));
        commandTable.append(PaintCommand(setClipRect,         &PaintCommands::command_setClipRect));
        commandTable.append(PaintCommand(setClipRegion,       &PaintCommands::command_setClipRegion));
        commandTable.append(PaintCommand(setClipping,         &PaintCommands::command_setClipping));
        commandTable.append(PaintCommand(setFont,             &PaintCommands::command_setFont));
        commandTable.append(PaintCommand(setPen2,             &PaintCommands::command_setPen2));
        commandTable.append(PaintCommand(setPen,              &PaintCommands::command_setPen));
        commandTable.append(PaintCommand(setRenderHint,       &PaintCommands::command_setRenderHint));
        commandTable.append(PaintCommand(setCompositionMode,
                                         &PaintCommands::command_setCompositionMode));
        commandTable.append(PaintCommand(gradient_clearStops,
                                         &PaintCommands::command_gradient_clearStops));
        commandTable.append(PaintCommand(gradient_appendStop,
                                         &PaintCommands::command_gradient_appendStop));
        commandTable.append(PaintCommand(gradient_setLinear,
                                         &PaintCommands::command_gradient_setLinear));
        commandTable.append(PaintCommand(gradient_setRadial,
                                         &PaintCommands::command_gradient_setRadial));
        commandTable.append(PaintCommand(gradient_setConical,
                                         &PaintCommands::command_gradient_setConical));
        commandTable.append(PaintCommand(gradient_setSpread,
                                         &PaintCommands::command_gradient_setSpread));

        // Drawing ops
        commandTable.append(PaintCommand(qt3_drawEllipse,     &PaintCommands::command_qt3_drawEllipse));
        commandTable.append(PaintCommand(qt3_drawArc,         &PaintCommands::command_qt3_drawArc));
        commandTable.append(PaintCommand(qt3_drawChord,       &PaintCommands::command_qt3_drawChord));
        commandTable.append(PaintCommand(qt3_drawPie,         &PaintCommands::command_qt3_drawPie));
        commandTable.append(PaintCommand(qt3_drawRect,        &PaintCommands::command_qt3_drawRect));
        commandTable.append(PaintCommand(qt3_drawRoundRect,
                                         &PaintCommands::command_qt3_drawRoundRect));
        commandTable.append(PaintCommand(drawEllipse,         &PaintCommands::command_drawEllipse));
        commandTable.append(PaintCommand(drawArc,             &PaintCommands::command_drawArc));
        commandTable.append(PaintCommand(drawChord,           &PaintCommands::command_drawChord));
        commandTable.append(PaintCommand(drawPie,             &PaintCommands::command_drawPie));
        commandTable.append(PaintCommand(drawRect,            &PaintCommands::command_drawRect));
        commandTable.append(PaintCommand(drawRoundRect,       &PaintCommands::command_drawRoundRect));
        commandTable.append(PaintCommand(drawLine,            &PaintCommands::command_drawLine));
        commandTable.append(PaintCommand(drawPath,            &PaintCommands::command_drawPath));
        commandTable.append(PaintCommand(drawPixmap,          &PaintCommands::command_drawPixmap));
        commandTable.append(PaintCommand(drawImage,           &PaintCommands::command_drawImage));
        commandTable.append(PaintCommand(drawPoint,           &PaintCommands::command_drawPoint));
        commandTable.append(PaintCommand(drawPolygon,         &PaintCommands::command_drawPolygon));
        commandTable.append(PaintCommand(drawConvexPolygon,
                                         &PaintCommands::command_drawConvexPolygon));
        commandTable.append(PaintCommand(drawPolyline,        &PaintCommands::command_drawPolyline));

        commandTable.append(PaintCommand(drawText,            &PaintCommands::command_drawText));
        commandTable.append(PaintCommand(drawTiledPixmap,     &PaintCommands::command_drawTiledPixmap));
        commandTable.append(PaintCommand(path_addText,        &PaintCommands::command_path_addText));
        commandTable.append(PaintCommand(path_addEllipse,     &PaintCommands::command_path_addEllipse));
        commandTable.append(PaintCommand(path_addRect,        &PaintCommands::command_path_addRect));
        commandTable.append(PaintCommand(path_addPolygon,     &PaintCommands::command_path_addPolygon));
        commandTable.append(PaintCommand(path_arcTo,          &PaintCommands::command_path_arcTo));
        commandTable.append(PaintCommand(path_cubicTo,        &PaintCommands::command_path_cubicTo));
        commandTable.append(PaintCommand(path_createOutline,
                                         &PaintCommands::command_path_createOutline));
        commandTable.append(PaintCommand(path_lineTo,         &PaintCommands::command_path_lineTo));
        commandTable.append(PaintCommand(path_moveTo,         &PaintCommands::command_path_moveTo));
        commandTable.append(PaintCommand(path_closeSubpath,
                                         &PaintCommands::command_path_closeSubpath));
        commandTable.append(PaintCommand(path_setFillRule,    &PaintCommands::command_path_setFillRule));
        commandTable.append(PaintCommand(path_debugPrint,    &PaintCommands::command_path_debugPrint));

        commandTable.append(PaintCommand(pen_setDashPattern, &PaintCommands::command_pen_setDashPattern));

        commandTable.append(PaintCommand(region_addRect,      &PaintCommands::command_region_addRect));
        commandTable.append(PaintCommand(region_addEllipse,
                                         &PaintCommands::command_region_addEllipse));
        commandTable.append(PaintCommand(region_getClipRegion,
                                         &PaintCommands::command_region_getClipRegion));
        commandTable.append(PaintCommand(path_getClipPath,
                                         &PaintCommands::command_path_getClipPath));

        commandTable.append(PaintCommand(surface_begin,
                                         &PaintCommands::command_surface_begin));
        commandTable.append(PaintCommand(surface_end,
                                         &PaintCommands::command_surface_end));

        // XForms
        commandTable.append(PaintCommand(resetMatrix,         &PaintCommands::command_resetMatrix));
        commandTable.append(PaintCommand(translate,           &PaintCommands::command_translate));
        commandTable.append(PaintCommand(rotate,              &PaintCommands::command_rotate));
        commandTable.append(PaintCommand(scale,               &PaintCommands::command_scale));

        // Other commands
        commandTable.append(PaintCommand(save,                &PaintCommands::command_save));
        commandTable.append(PaintCommand(restore,             &PaintCommands::command_restore));
        commandTable.append(PaintCommand(pixmap_load,         &PaintCommands::command_pixmap_load));
        commandTable.append(PaintCommand(bitmap_load,         &PaintCommands::command_bitmap_load));
        commandTable.append(PaintCommand(pixmap_setMask,      &PaintCommands::command_pixmap_setMask));
        commandTable.append(PaintCommand(image_load,          &PaintCommands::command_image_load));
        commandTable.append(PaintCommand(image_setNumColors,
                                         &PaintCommands::command_image_setNumColors));
        commandTable.append(PaintCommand(image_setColor,
                                         &PaintCommands::command_image_setColor));
        commandTable.append(PaintCommand(image_convertToFormat,
                                         &PaintCommands::command_image_convertToFormat));

        commandTable.append(PaintCommand(textlayout_draw,
                                          &PaintCommands::command_textlayout_draw));

        commandTable.append(PaintCommand(abort,               &PaintCommands::command_abort));

        // noops
        commandTable.append(PaintCommand(empty,               &PaintCommands::command_noop));
    }


    QPixmap pm(20, 20);
    pm.fill(Qt::white);

    painter->drawTiledPixmap(0, 0, painter->window().width(), painter->window().height(), pm);

    abort = false;
    for (int i=0; i<commands.size() && !abort; ++i) {
        const QString &commandNow = commands.at(i);
        currentCommand = commandNow;
        currentCommandIndex = i;
        runCommand(commandNow);
    }
}


int PaintCommands::convertToInt(const QString &str)
{
    return qRound(convertToDouble(str));
}

float PaintCommands::convertToFloat(const QString &str)
{
    return float(convertToDouble(str));
}

double PaintCommands::convertToDouble(const QString &str)
{
    static QRegExp re("cp([0-9])([xy])");
    if (str.toLower() == "width")
        return painter->window().width();
    if (str.toLower() == "height")
        return painter->window().height();
    if (re.indexIn(str) >= 0) {
        int index = re.cap(1).toInt();
        bool is_it_x = re.cap(2) == "x";
        if (index < 0 || index >= controlPoints.size()) {
            qWarning("ERROR: control point index=%d is out of bounds", index);
            return 0;
        }
        return is_it_x ? controlPoints.at(index).x() : controlPoints.at(index).y();
    }
    return str.toDouble();
}

QColor PaintCommands::convertToColor(const QString &str)
{
    if (str == "color0")
        return QColor(Qt::color0);
    else if (str == "color1")
        return QColor(Qt::color1);

    static QRegExp alphaColor("#?([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    static QRegExp opaqueColor("#?([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");

    Q_ASSERT(alphaColor.isValid());
    Q_ASSERT(opaqueColor.isValid());

    if (alphaColor.indexIn(str) >= 0) {
        return QColor(alphaColor.cap(2).toInt(0, 16),
                      alphaColor.cap(3).toInt(0, 16),
                      alphaColor.cap(4).toInt(0, 16),
                      alphaColor.cap(1).toInt(0, 16));
    } else if (opaqueColor.indexIn(str) >= 0) {
        return QColor(opaqueColor.cap(1).toInt(0, 16),
                      opaqueColor.cap(2).toInt(0, 16),
                      opaqueColor.cap(3).toInt(0, 16));
    }
    return QColor(str);
}

void PaintCommands::command_comment(QRegExp)
{
    //printf(" - comment: %s\n", qPrintable(currentCommand));
}

void PaintCommands::insertAt(int commandIndex, const QStringList &newCommands)
{
    int index = 0;
    int left = newCommands.size();
    while (left--)
        commands.insert(++commandIndex, newCommands.at(index++));
}

void PaintCommands::command_import(QRegExp re)
{
    QString importFile(re.cap(1));
    QFileInfo fi(filepath);
    QDir dir = fi.absoluteDir();
    QFile *file = new QFile(dir.absolutePath() + QDir::separator() + importFile);
    if (importFile.isEmpty() || !file->exists()) {
        dir.cdUp();
        dir.cd("images");
        delete file;
        file = new QFile(dir.absolutePath() + QDir::separator() + importFile);
        if (importFile.isEmpty() || !file->exists()) {
            printf(" - importing non-existing file at line %d (%s)\n", currentCommandIndex,
                   qPrintable(file->fileName()));
            delete file;
            return;
        }
    }

    if (!file->open(QIODevice::ReadOnly)) {
        printf(" - failed to read file: '%s'\n", qPrintable(file->fileName()));
        delete file;
        return;
    }
//     if (verbose)
//         printf(" - importing file at line %d (%s)\n", currentCommandIndex,
//                qPrintable(file.fileName()));

    QFileInfo fileinfo(*file);
    commands[currentCommandIndex] = QString("# import file (%1) start").arg(fileinfo.fileName());
    QTextStream textFile(file);
    QString rawContent = textFile.readAll();
    QStringList importedData = rawContent.split('\n', QString::SkipEmptyParts);
    importedData.append(QString("# import file (%1) end ---").arg(fileinfo.fileName()));
    insertAt(currentCommandIndex, importedData);

//     if (verbose) {
//         printf(" - Command buffer now looks like:\n");
//         for (int i = 0; i < commands.count(); ++i)
//             printf(" ---> {%s}\n", qPrintable(commands.at(i)));
//     }
    delete file;
}

void PaintCommands::command_begin_block(QRegExp re)
{
    const QString &blockName = re.cap(1);
//     if (Lance::self()->options()->verboseMode)
//         printf(" - begin_block (%s)\n", qPrintable(blockName));

    commands[currentCommandIndex] = QString("# begin block (%1)").arg(blockName);
    QStringList newBlock;
    int i = currentCommandIndex + 1;
    for (; i < commands.count(); ++i) {
        const QString &nextCmd = commands.at(i);
        if (nextCmd.startsWith("end_block")) {
            commands[i] = QString("# end block (%1)").arg(blockName);
            break;
        }
        newBlock += nextCmd;
    }

//     if (Lance::self()->options()->verboseMode)
//         for (int j = 0; j < newBlock.count(); ++j)
//             printf("      %d: %s\n", j, qPrintable(newBlock.at(j)));

    if (i >= commands.count())
        printf(" - Warning! Block doesn't have an 'end_block' marker!\n");

    blockMap.insert(blockName, newBlock);
}

void PaintCommands::command_end_block(QRegExp)
{
    printf(" - end_block should be consumed by begin_block command.\n");
    printf("   You will never see this if your block markers are in sync\n");
    printf("   (noop)\n");
}

void PaintCommands::command_repeat_block(QRegExp re)
{
    QString blockName = re.cap(1);
//     if (Lance::self()->options()->verboseMode)
//         printf(" - repeating block (%s)\n", qPrintable(blockName));

    QStringList block = blockMap.value(blockName);
    if (block.isEmpty()) {
        printf(" - repeated block (%s) is empty!\n", qPrintable(blockName));
        return;
    }

    commands[currentCommandIndex] = QString("# repeated block (%1)").arg(blockName);
    insertAt(currentCommandIndex, block);
}

void PaintCommands::command_drawLine(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double x1 = convertToDouble(caps.at(1));
    double y1 = convertToDouble(caps.at(2));
    double x2 = convertToDouble(caps.at(3));
    double y2 = convertToDouble(caps.at(4));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawLine((%.2f, %.2f), (%.2f, %.2f))\n", x1, y1, x2, y2);


    painter->drawLine(QLineF(x1, y1, x2, y2));
}

void PaintCommands::command_drawPath(QRegExp re)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPath(name=%s)\n", qPrintable(re.cap(1)));

    QPainterPath &path = pathMap[re.cap(1)];
    painter->drawPath(path);
}

void PaintCommands::command_drawPixmap(QRegExp re)
{
    QPixmap pm;
    pm = pixmapMap[re.cap(1)]; // try cache first
    if (pm.isNull())
        pm = QPixmap(re.cap(1));
    if (pm.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        pm = QPixmap(fileName);
        if (pm.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            pm = QPixmap(fileName);
        }
    }
    if (pm.isNull()) {
        fprintf(stderr, "ERROR(drawPixmap): failed to load pixmap: '%s'\n",
                qPrintable(re.cap(1)));
        return;
    }

    int tx = convertToInt(re.cap(2));
    int ty = convertToInt(re.cap(3));
    int tw = convertToInt(re.cap(4));
    int th = convertToInt(re.cap(5));

    int sx = convertToInt(re.cap(6));
    int sy = convertToInt(re.cap(7));
    int sw = convertToInt(re.cap(8));
    int sh = convertToInt(re.cap(9));

    if (tw == 0) tw = -1;
    if (th == 0) th = -1;
    if (sw == 0) sw = -1;
    if (sh == 0) sh = -1;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPixmap('%s' dim=(%d, %d), depth=%d, (%d, %d, %d, %d), (%d, %d, %d, %d)\n",
//                qPrintable(re.cap(1)), pm.width(), pm.height(), pm.depth(),
//                tx, ty, tw, th, sx, sy, sw, sh);


    painter->drawPixmap(QRect(tx, ty, tw, th), pm, QRect(sx, sy, sw, sh));
}

void PaintCommands::command_drawImage(QRegExp re)
{
    QImage im;
    im = imageMap[re.cap(1)]; // try cache first
    if (im.isNull())
        im = QImage(re.cap(1)); // ### gah, just use png as default
    if (im.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        im = QImage(fileName);
        if (im.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            im = QImage(fileName);
        }
    }
    if (im.isNull()) {
        fprintf(stderr, "ERROR(drawImage): failed to load image: '%s'\n", qPrintable(re.cap(1)));
        return;
    }

    int tx = convertToInt(re.cap(2));
    int ty = convertToInt(re.cap(3));
    int tw = convertToInt(re.cap(4));
    int th = convertToInt(re.cap(5));

    int sx = convertToInt(re.cap(6));
    int sy = convertToInt(re.cap(7));
    int sw = convertToInt(re.cap(8));
    int sh = convertToInt(re.cap(9));

    if (tw == 0) tw = -1;
    if (th == 0) th = -1;
    if (sw == 0) sw = -1;
    if (sh == 0) sh = -1;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawImage('%s' dim=(%d, %d), (%d, %d, %d, %d), (%d, %d, %d, %d)\n",
//                qPrintable(re.cap(1)), im.width(), im.height(), tx, ty, tw, th, sx, sy, sw, sh);


    painter->drawImage(QRect(tx, ty, tw, th), im, QRect(sx, sy, sw, sh), Qt::OrderedDither | Qt::OrderedAlphaDither);
}

void PaintCommands::command_drawTiledPixmap(QRegExp re)
{
    QPixmap pm;
    pm = pixmapMap[re.cap(1)]; // try cache first
    if (pm.isNull())
        pm = QPixmap(re.cap(1));
    if (pm.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        pm = QPixmap(fileName);
        if (pm.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            pm = QPixmap(fileName);
        }
    }
    if (pm.isNull()) {
        fprintf(stderr, "ERROR(drawTiledPixmap): failed to load pixmap: '%s'\n",
                qPrintable(re.cap(1)));
        return;
    }

    int tx = convertToInt(re.cap(2));
    int ty = convertToInt(re.cap(3));
    int tw = convertToInt(re.cap(4));
    int th = convertToInt(re.cap(5));

    int sx = convertToInt(re.cap(6));
    int sy = convertToInt(re.cap(7));

    if (tw == 0) tw = -1;
    if (th == 0) th = -1;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawTiledPixmap('%s' dim=(%d, %d), (%d, %d, %d, %d), (%d, %d)\n",
//                qPrintable(re.cap(1)), pm.width(), pm.height(), tx, ty, tw, th, sx, sy);

    painter->drawTiledPixmap(tx, ty, tw, th, pm, sx, sy);
}


void PaintCommands::command_drawPoint(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    float x = convertToFloat(caps.at(1));
    float y = convertToFloat(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPoint(%.2f, %.2f)\n", x, y);

    painter->drawPoint(QPointF(x, y));
}


void PaintCommands::command_drawPolygon(QRegExp re)
{
    static QRegExp separators("\\s");
    QStringList caps = re.capturedTexts();
    QString cap = caps.at(1);
    QStringList numbers = cap.split(separators, QString::SkipEmptyParts);

    QPolygonF array;
    for (int i=0; i + 1<numbers.size(); i+=2)
        array.append(QPointF(convertToDouble(numbers.at(i)), convertToDouble(numbers.at(i+1))));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPolygon(size=%d)\n", array.size());


    painter->drawPolygon(array, caps.at(2).toLower() == "winding" ? Qt::WindingFill : Qt::OddEvenFill);
}


void PaintCommands::command_drawConvexPolygon(QRegExp re)
{
    static QRegExp separators("\\s");
    QStringList caps = re.capturedTexts();
    QString cap = caps.at(1);
    QStringList numbers = cap.split(separators, QString::SkipEmptyParts);

    QPolygonF array;
    for (int i=0; i + 1<numbers.size(); i+=2)
        array.append(QPointF(convertToDouble(numbers.at(i)), convertToDouble(numbers.at(i+1))));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawConvexPolygon(size=%d)\n", array.size());


    painter->drawConvexPolygon(array);
}


void PaintCommands::command_drawPolyline(QRegExp re)
{
    static QRegExp separators("\\s");
    QStringList numbers = re.cap(1).split(separators, QString::SkipEmptyParts);

    QPolygonF array;
    for (int i=0; i + 1<numbers.size(); i+=2)
        array.append(QPointF(numbers.at(i).toFloat(),numbers.at(i+1).toFloat()));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPolyline(size=%d)\n", array.size());


    painter->drawPolyline(array.toPolygon());
}


void PaintCommands::command_drawRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    float x = convertToFloat(caps.at(1));
    float y = convertToFloat(caps.at(2));
    float w = convertToFloat(caps.at(3));
    float h = convertToFloat(caps.at(4));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawRect(%.2f, %.2f, %.2f, %.2f)\n", x, y, w, h);


    painter->drawRect(QRectF(x, y, w, h));
}

void PaintCommands::command_drawRoundRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int xs = caps.at(5).isEmpty() ? 50 : convertToInt(caps.at(5));
    int ys = caps.at(6).isEmpty() ? 50 : convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawRoundRect(%d, %d, %d, %d, [%d, %d])\n", x, y, w, h, xs, ys);


    painter->drawRoundRect(x, y, w, h, xs, ys);
}

void PaintCommands::command_drawEllipse(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    float x = convertToFloat(caps.at(1));
    float y = convertToFloat(caps.at(2));
    float w = convertToFloat(caps.at(3));
    float h = convertToFloat(caps.at(4));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawEllipse(%.2f, %.2f, %.2f, %.2f)\n", x, y, w, h);


    painter->drawEllipse(QRectF(x, y, w, h));
}

void PaintCommands::command_drawPie(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawPie(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    painter->drawPie(x, y, w, h, angle, sweep);
}


void PaintCommands::command_drawChord(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawChord(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    painter->drawChord(x, y, w, h, angle, sweep);
}

void PaintCommands::command_drawArc(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawArc(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    painter->drawArc(x, y, w, h, angle, sweep);
}

void PaintCommands::command_qt3_drawRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawRect(%d, %d, %d, %d)\n", x, y, w, h);


    static_cast<QPainter*>(painter)->drawRect(x, y, w, h);
}

void PaintCommands::command_qt3_drawRoundRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int xrnd = caps.at(5).isEmpty() ? 25 : convertToInt(caps.at(5));
    int yrnd = caps.at(6).isEmpty() ? 25 : convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawRoundRect(%d, %d, %d, %d), %d, %d\n", x, y, w, h, xrnd, yrnd);


    static_cast<QPainter*>(painter)->drawRoundRect(x, y, w, h, xrnd, yrnd);
}


void PaintCommands::command_qt3_drawEllipse(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawEllipse(%d, %d, %d, %d)\n", x, y, w, h);


    static_cast<QPainter*>(painter)->drawEllipse(x, y, w, h);
}

void PaintCommands::command_qt3_drawPie(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawPie(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    static_cast<QPainter*>(painter)->drawPie(x, y, w, h, angle, sweep);
}

void PaintCommands::command_qt3_drawChord(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawChord(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    static_cast<QPainter*>(painter)->drawChord(x, y, w, h, angle, sweep);
}

void PaintCommands::command_qt3_drawArc(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));
    int angle = convertToInt(caps.at(5));
    int sweep = convertToInt(caps.at(6));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - qt3_drawArc(%d, %d, %d, %d, %d, %d)\n", x, y, w, h, angle, sweep);


    static_cast<QPainter*>(painter)->drawArc(x, y, w, h, angle, sweep);
}

void PaintCommands::command_drawText(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    QString txt = caps.at(3);

//     if (Lance::self()->options()->verboseMode)
//         printf(" - drawText(%d, %d, %s)\n", x, y, qPrintable(txt));


    painter->drawText(x, y, txt);
}

void PaintCommands::command_noop(QRegExp)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - noop: %s\n", qPrintable(currentCommand));

    if (!currentCommand.trimmed().isEmpty()) {
        fprintf(stderr, "unknown command: '%s'\n", qPrintable(currentCommand.trimmed()));
    }
}

void PaintCommands::command_path_addText(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x = convertToDouble(caps.at(2));
    double y = convertToDouble(caps.at(3));
    QString text = caps.at(4);

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_addText(%s, %.2f, %.2f, text=%s\n", qPrintable(name),
//                x, y, qPrintable(text));

    pathMap[name].addText(x, y, painter->font(), text);
}

void PaintCommands::command_path_addEllipse(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x = convertToDouble(caps.at(2));
    double y = convertToDouble(caps.at(3));
    double w = convertToDouble(caps.at(4));
    double h = convertToDouble(caps.at(5));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_addEllipse(%s, %.2f, %.2f, %.2f, %.2f)\n",
//                qPrintable(name), x, y, w, h);


    pathMap[name].addEllipse(x, y, w, h);
}

void PaintCommands::command_path_addRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x = convertToDouble(caps.at(2));
    double y = convertToDouble(caps.at(3));
    double w = convertToDouble(caps.at(4));
    double h = convertToDouble(caps.at(5));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_addRect(%s, %.2f, %.2f, %.2f, %.2f)\n",
//                qPrintable(name), x, y, w, h);


    pathMap[name].addRect(x, y, w, h);
}

void PaintCommands::command_path_addPolygon(QRegExp re)
{
    static QRegExp separators("\\s");
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    QString cap = caps.at(2);
    QStringList numbers = cap.split(separators, QString::SkipEmptyParts);

    QPolygonF array;
    for (int i=0; i + 1<numbers.size(); i+=2)
        array.append(QPointF(numbers.at(i).toFloat(),numbers.at(i+1).toFloat()));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_addPolygon(name=%s, size=%d)\n",
//                qPrintable(name), array.size());


    pathMap[name].addPolygon(array);
}

void PaintCommands::command_path_arcTo(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x = convertToDouble(caps.at(2));
    double y = convertToDouble(caps.at(3));
    double w = convertToDouble(caps.at(4));
    double h = convertToDouble(caps.at(5));
    double angle = convertToDouble(caps.at(6));
    double length = convertToDouble(caps.at(7));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_arcTo(%s, %.2f, %.2f, %.2f, %.2f, angle=%.2f, len=%.2f)\n",
//                qPrintable(name), x, y, w, h, angle, length);


    pathMap[name].arcTo(x, y, w, h, angle, length);
}

void PaintCommands::command_path_createOutline(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    QString newName = caps.at(2);
    QPen pen = painter->pen();

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_createOutline(%s, name=%s, width=%d)\n",
//                qPrintable(name), qPrintable(newName), pen.width());

    if (!pathMap.contains(name)) {
        fprintf(stderr, "createOutline(), unknown path: %s\n", qPrintable(name));
        return;
    }
    QPainterPathStroker stroker;
    stroker.setWidth(pen.widthF());
    stroker.setDashPattern(pen.style());
    stroker.setCapStyle(pen.capStyle());
    stroker.setJoinStyle(pen.joinStyle());
    pathMap[newName] = stroker.createStroke(pathMap[name]);
}

void PaintCommands::command_path_cubicTo(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x1 = convertToDouble(caps.at(2));
    double y1 = convertToDouble(caps.at(3));
    double x2 = convertToDouble(caps.at(4));
    double y2 = convertToDouble(caps.at(5));
    double x3 = convertToDouble(caps.at(6));
    double y3 = convertToDouble(caps.at(7));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_cubicTo(%s, (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f))\n",
//                qPrintable(name), x1, y1, x2, y2, x3, y3);


    pathMap[name].cubicTo(x1, y1, x2, y2, x3, y3);
}

void PaintCommands::command_path_moveTo(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x1 = convertToDouble(caps.at(2));
    double y1 = convertToDouble(caps.at(3));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_moveTo(%s, (%.2f, %.2f))\n",
//                qPrintable(name), x1, y1);

    pathMap[name].moveTo(x1, y1);
}


void PaintCommands::command_path_lineTo(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    double x1 = convertToDouble(caps.at(2));
    double y1 = convertToDouble(caps.at(3));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_lineTo(%s, (%.2f, %.2f))\n",
//                qPrintable(name), x1, y1);


    pathMap[name].lineTo(x1, y1);
}


void PaintCommands::command_path_setFillRule(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    bool winding = caps.at(2).toLower() == "winding";

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_setFillRule(name=%s, winding=%d)\n",
//                qPrintable(name), winding);

    pathMap[name].setFillRule(winding ? Qt::WindingFill : Qt::OddEvenFill);
}


void PaintCommands::command_path_closeSubpath(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);

//     if (Lance::self()->options()->verboseMode)
//         printf(" - path_closeSubpath(name=%s)\n", qPrintable(name));

    pathMap[name].closeSubpath();
}

void PaintCommands::command_path_getClipPath(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);

    QPainterPath path = painter->clipPath();
    QRectF bounds = path.boundingRect();

//     if (Lance::self()->options()->verboseMode) {
//         printf(" - path_getClipPath(name=%s), x=%.2f, y=%.2f, w=%.2f, h=%.2f\n",
//                qPrintable(name),
//                bounds.x(), bounds.y(), bounds.width(), bounds.height());
//     }

    pathMap[name] = path;
}

static void qt_debug_path(const QPainterPath &path, const QString &name)
{
    const char *names[] = {
        "MoveTo     ",
        "LineTo     ",
        "CurveTo    ",
        "CurveToData"
    };

    printf("\nQPainterPath (%s): elementCount=%d\n", qPrintable(name), path.elementCount());
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        Q_ASSERT(e.type >= 0 && e.type <= QPainterPath::CurveToDataElement);
        printf(" - %3d:: %s, (%.2f, %.2f)\n", i, names[e.type], e.x, e.y);
    }
}

void PaintCommands::command_path_debugPrint(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    qt_debug_path(pathMap[name], name);
}

void PaintCommands::command_pen_setDashPattern(QRegExp re)
{
    static QRegExp separators("\\s");
    QStringList caps = re.capturedTexts();
    QString cap = caps.at(1);
    QStringList numbers = cap.split(separators, QString::SkipEmptyParts);

    QVector<qreal> pattern;
    for (int i=0; i<numbers.size(); ++i)
        pattern.append(convertToDouble(numbers.at(i)));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - pen_setDashPattern(size=%d)\n", pattern.size());

    QPen p = painter->pen();
    p.setDashPattern(pattern);
    painter->setPen(p);
}

void PaintCommands::command_region_addRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    int x = convertToInt(caps.at(2));
    int y = convertToInt(caps.at(3));
    int w = convertToInt(caps.at(4));
    int h = convertToInt(caps.at(5));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - region_addRect(%s, %d, %d, %d, %d)\n",
//                qPrintable(name), x, y, w, h);


    regionMap[name] += QRect(x, y, w, h);
}


void PaintCommands::command_region_addEllipse(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    int x = convertToInt(caps.at(2));
    int y = convertToInt(caps.at(3));
    int w = convertToInt(caps.at(4));
    int h = convertToInt(caps.at(5));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - region_addEllipse(%s, %d, %d, %d, %d)\n",
//                qPrintable(name), x, y, w, h);


    regionMap[name] += QRegion(x, y, w, h, QRegion::Ellipse);
}


void PaintCommands::command_region_getClipRegion(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString name = caps.at(1);
    QRegion region = painter->clipRegion();

//     if (Lance::self()->options()->verboseMode)
//         printf(" - region_getClipRegion(name=%s), bounds=[%d, %d, %d, %d]\n",
//                qPrintable(name),
//                region.boundingRect().x(),
//                region.boundingRect().y(),
//                region.boundingRect().width(),
//                region.boundingRect().height());

    regionMap[name] = region;
}


void PaintCommands::command_resetMatrix(QRegExp)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - resetMatrix()\n");

    painter->resetMatrix();
    painter->setWindow(0, 0, width, height);
}


void PaintCommands::command_restore(QRegExp)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - restore()\n");


    painter->restore();
}


void PaintCommands::command_rotate(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int angle = convertToInt(caps.at(1));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - rotate(%d)\n", angle);


    painter->rotate(angle);
}


void PaintCommands::command_save(QRegExp)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - save()\n");


    painter->save();
}


void PaintCommands::command_scale(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double sx = convertToDouble(caps.at(1));
    double sy = convertToDouble(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - scale(%.2f, %.2f)\n", sx, sy);


    painter->scale(sx, sy);
}


void PaintCommands::command_setBackground(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QColor color = convertToColor(caps.at(1));
    QString pattern = caps.at(2);

    int style = translate_enum(brushStyleTable, pattern, Qt::LinearGradientPattern);
    if (style < 0)
        style = Qt::SolidPattern;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setBackground(%s, %s)\n", qPrintable(color.name()),
//                qPrintable(pattern));


    painter->setBackground(QBrush(color, Qt::BrushStyle(style)));
}


void PaintCommands::command_setBgMode(QRegExp re)
{
    QString cap = re.cap(1);
    Qt::BGMode mode = Qt::TransparentMode;
    if (cap == "OpaqueMode" || cap == "Opaque")
        mode = Qt::OpaqueMode;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setBackgroundMode(%s)\n",
//                mode == Qt::OpaqueMode ? "OpaqueMode" : "TransparentMode");


    painter->setBackgroundMode(mode);
}


void PaintCommands::command_setBrush(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString image_file = re.cap(1);
    if (QFileInfo(image_file).exists()) { // Assume pixmap
        QPixmap pm(image_file);
        if (pm.isNull()) {
            fprintf(stderr, "ERROR(setBrush): pattern '%s' is not a pixmap\n", qPrintable(caps.at(1)));
            return;
        }

//         if (Lance::self()->options()->verboseMode)
//             printf(" - setBrush(pixmap=%s, width=%d, height=%d)\n",
//                    qPrintable(caps.at(1)), pm.width(), pm.height());

        painter->setBrush(QBrush(pm));
    } else if (caps.at(1).toLower() == "nobrush") {
        painter->setBrush(Qt::NoBrush);
//         if (Lance::self()->options()->verboseMode)
//             printf(" - setBrush(Qt::NoBrush)\n");
    } else {
        QColor color = convertToColor(caps.at(1));
        QString pattern = caps.at(2);

        int style = translate_enum(brushStyleTable, pattern, Qt::LinearGradientPattern);
        if (style < 0)
            style = Qt::SolidPattern;

//         if (Lance::self()->options()->verboseMode)
//             printf(" - setBrush(#%02x%02x%02x%02x, %s (%d))\n",
//                    color.alpha(), color.red(), color.green(), color.blue(),
//                    qPrintable(pattern), style);


        painter->setBrush(QBrush(color, Qt::BrushStyle(style)));
    }
}

void PaintCommands::command_setBrushOrigin(QRegExp re)
{
    int x = convertToInt(re.cap(1));
    int y = convertToInt(re.cap(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setBrushOrigin(%d, %d)\n", x, y);


    painter->setBrushOrigin(x, y);
}

void PaintCommands::command_setClipping(QRegExp re)
{
    bool clipping = re.cap(1).toLower() == "true";

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setClipping(%d)\n", clipping);


    painter->setClipping(clipping);
}

void PaintCommands::command_setClipRect(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    int x = convertToInt(caps.at(1));
    int y = convertToInt(caps.at(2));
    int w = convertToInt(caps.at(3));
    int h = convertToInt(caps.at(4));

    int combine = translate_enum(clipOperationTable, caps.at(5), Qt::UniteClip + 1);
    if (combine == -1)
        combine = Qt::ReplaceClip;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setClipRect(%d, %d, %d, %d), %s\n", x, y, w, h, clipOperationTable[combine]);


    painter->setClipRect(x, y, w, h, Qt::ClipOperation(combine));
}


void PaintCommands::command_setClipPath(QRegExp re)
{
    int combine = translate_enum(clipOperationTable, re.cap(2), Qt::UniteClip + 1);
    if (combine == -1)
        combine = Qt::ReplaceClip;

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setClipPath(name=%s), %s\n", qPrintable(re.cap(1)),
//                clipOperationTable[combine]);

    if (!pathMap.contains(re.cap(1)))
        fprintf(stderr, " - setClipPath, no such path");
    painter->setClipPath(pathMap[re.cap(1)], Qt::ClipOperation(combine));
}


void PaintCommands::command_setClipRegion(QRegExp re)
{
    int combine = translate_enum(clipOperationTable, re.cap(2), Qt::UniteClip + 1);
    if (combine == -1)
        combine = Qt::ReplaceClip;
    QRegion r = regionMap[re.cap(1)];

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setClipRegion(name=%s), bounds=[%d, %d, %d, %d], %s\n",
//                qPrintable(re.cap(1)),
//                r.boundingRect().x(),
//                r.boundingRect().y(),
//                r.boundingRect().width(),
//                r.boundingRect().height(),
//                clipOperationTable[combine]);

    painter->setClipRegion(regionMap[re.cap(1)], Qt::ClipOperation(combine));
}

void PaintCommands::command_setFont(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QString family = caps.at(1);
    int size = convertToInt(caps.at(2));
    int weight = caps.at(3).toLower() == "bold" ? QFont::Bold : QFont::Normal;
    bool italic = caps.at(4).toLower() == "true" || caps.at(4).toLower() == "italic";

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setFont(family=%s, size=%d, weight=%d, italic=%d\n",
//                qPrintable(family), size, weight, italic);


    painter->setFont(QFont(family, size, weight, italic));
}


void PaintCommands::command_setPen(QRegExp re)
{
    QString cap = re.cap(1);
    int style = translate_enum(penStyleTable, cap, Qt::DashDotDotLine + 1);
    if (style >= 0) {
//         if (Lance::self()->options()->verboseMode)
//             printf(" - setPen(%s)\n", qPrintable(cap));

        painter->setPen(Qt::PenStyle(style));
    } else if (cap.toLower() == "brush") {
        QPen pen(painter->brush(), 0);
//         if (Lance::self()->options()->verboseMode) {
//             printf(" - setPen(brush), style=%d, color=%08x\n",
//                    pen.brush().style(), pen.color().rgba());
//         }
        painter->setPen(pen);
    } else {
        QColor color = convertToColor(cap);
//         if (Lance::self()->options()->verboseMode)
//             printf(" - setPen(%s)\n", qPrintable(color.name()));

        painter->setPen(color);
    }
}


void PaintCommands::command_setPen2(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QBrush brush;

    if (caps.at(1).toLower() == "brush")
        brush = painter->brush();
    else
        brush = convertToColor(caps.at(1));

    double width = convertToDouble(caps.at(2));
    int penStyle = translate_enum(penStyleTable, caps.at(3), Qt::DashDotDotLine + 1);
    if (penStyle < 0)
        penStyle = Qt::SolidLine;

    Qt::PenCapStyle capStyle = Qt::SquareCap;
    if (caps.at(4).toLower() == "flatcap") capStyle = Qt::FlatCap;
    else if (caps.at(4).toLower() == "squarecap") capStyle = Qt::SquareCap;
    else if (caps.at(4).toLower() == "roundcap") capStyle = Qt::RoundCap;
    else if (!caps.at(4).isEmpty())
        fprintf(stderr, "ERROR: setPen, unknown capStyle: %s\n",
                qPrintable(caps.at(4)));

    Qt::PenJoinStyle joinStyle = Qt::BevelJoin;
    if (caps.at(5).toLower() == "miterjoin") joinStyle = Qt::MiterJoin;
    else if (caps.at(5).toLower() == "beveljoin") joinStyle = Qt::BevelJoin;
    else if (caps.at(5).toLower() == "roundjoin") joinStyle = Qt::RoundJoin;
    else if (!caps.at(5).isEmpty())
        fprintf(stderr, "ERROR: setPen, unknown joinStyle: %s\n", qPrintable(caps.at(5)));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setPen(%s, width=%f, style=%d, cap=%d, join=%d)\n",
//                qPrintable(brush.color().name()), width, penStyle, capStyle, joinStyle);


    painter->setPen(QPen(brush, width, Qt::PenStyle(penStyle), capStyle, joinStyle));
}


void PaintCommands::command_setRenderHint(QRegExp re)
{
    QString hintString = re.cap(1).toLower();
    bool on = re.cap(2).isEmpty() || re.cap(2).toLower() == "true";
    if (hintString.contains("antialiasing")) {
//         if (Lance::self()->options()->verboseMode)
//             printf(" - setRenderHint Antialiasing\n");

        painter->setRenderHint(QPainter::Antialiasing, on);
    } else if (hintString.contains("smoothpixmaptransform")) {
//         if (Lance::self()->options()->verboseMode)
//             printf(" - setRenderHint SmoothPixmapTransform\n");
        painter->setRenderHint(QPainter::SmoothPixmapTransform, on);
    } else {
        fprintf(stderr, "ERROR(setRenderHint): unknown hint '%s'\n", qPrintable(hintString));
    }
}


void PaintCommands::command_setCompositionMode(QRegExp re)
{
    QString modeString = re.cap(1).toLower();
    int mode = translate_enum(compositionModeTable, modeString, 12);

    if (mode < 0 || mode > QPainter::CompositionMode_Xor) {
        fprintf(stderr, "ERROR: invalid mode: %s\n", qPrintable(modeString));
        return;
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - setCompositionMode: %d: %s\n", mode, qPrintable(modeString));

    painter->setCompositionMode(QPainter::CompositionMode(mode));
}


void PaintCommands::command_translate(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double dx = convertToDouble(caps.at(1));
    double dy = convertToDouble(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - translate(%f, %f)\n", dx, dy);

    painter->translate(dx, dy);
}

void PaintCommands::command_pixmap_load(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString fileName = caps.at(1);
    QString name = caps.at(2);

    if (name.isEmpty())
        name = fileName;

    QPixmap px(fileName, (const char *)0, Qt::OrderedDither | Qt::OrderedAlphaDither);
    if (px.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        px = QPixmap(fileName, (const char *)0, Qt::OrderedDither | Qt::OrderedAlphaDither);
        if (px.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            px = QPixmap(fileName, (const char *)0, Qt::OrderedDither | Qt::OrderedAlphaDither);
        }
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - pixmap_load(%s as %s), size=[%d, %d], depth=%d\n",
//                qPrintable(fileName), qPrintable(name), px.width(), px.height(), px.depth());

    pixmapMap[name] = px;
}


void PaintCommands::command_bitmap_load(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString fileName = caps.at(1);
    QString name = caps.at(2);

    if (name.isEmpty())
        name = fileName;

    QBitmap bm(fileName);
    if (bm.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        bm = QPixmap(fileName);
        if (bm.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            bm = QPixmap(fileName);
        }
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - bitmap_load(%s as %s), size=[%d, %d], depth=%d\n",
//                qPrintable(fileName), qPrintable(name),
//                bm.width(), bm.height(), bm.depth());

    pixmapMap[name] = bm;
}


void PaintCommands::command_pixmap_setMask(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    QBitmap mask(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - pixmap_setMask(%s, %s)\n", qPrintable(caps.at(1)), qPrintable(caps.at(2)));

    if (!pixmapMap[caps.at(1)].isNull())
        pixmapMap[caps.at(1)].setMask(mask);
}

void PaintCommands::command_image_load(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString fileName = caps.at(1);
    QString name = caps.at(2);

    if (name.isEmpty())
        name = fileName;

    QImage image(fileName);

    if (image.isNull()) {
        QFileInfo fi(filepath);
        QDir dir = fi.absoluteDir();
        dir.cdUp();
        dir.cd("images");
        QString fileName = QString("%1/%2").arg(dir.absolutePath()).arg(re.cap(1));
        image = QImage(fileName);
        if (image.isNull() && !fileName.endsWith(".png")) {
            fileName.append(".png");
            image = QImage(fileName);
        }
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - image_load(%s as %s), size=[%d, %d], format=%d\n",
//                qPrintable(fileName), qPrintable(name),
//                image.width(), image.height(), image.format());

    imageMap[name] = image;
}


void PaintCommands::command_image_setNumColors(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString name = caps.at(1);
    int count = convertToInt(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - image_setNumColors(%s), %d -> %d\n",
//                qPrintable(name), imageMap[name].numColors(), count);

    imageMap[name].setNumColors(count);
}


void PaintCommands::command_image_setColor(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString name = caps.at(1);
    int index = convertToInt(caps.at(2));
    QColor color = convertToColor(caps.at(3));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - image_setColor(%s), %d = %08x\n", qPrintable(name), index, color.rgba());

    imageMap[name].setColor(index, color.rgba());
}


void PaintCommands::command_abort(QRegExp)
{
    abort = true;
}

void PaintCommands::command_gradient_clearStops(QRegExp)
{
//     if (Lance::self()->options()->verboseMode)
//         printf(" - gradient_clearStops\n");
    gradientStops.clear();
}

void PaintCommands::command_gradient_appendStop(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double pos = convertToDouble(caps.at(1));
    QColor color = convertToColor(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - gradient_appendStop(%.2f, %x)\n", pos, color.rgba());

    gradientStops << QGradientStop(pos, color);
}

void PaintCommands::command_gradient_setLinear(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double x1 = convertToDouble(caps.at(1));
    double y1 = convertToDouble(caps.at(2));
    double x2 = convertToDouble(caps.at(3));
    double y2 = convertToDouble(caps.at(4));

//     if (Lance::self()->options()->verboseMode) {
//         printf(" - gradient_setLinear (%.2f, %.2f), (%.2f, %.2f), spread=%d\n",
//                x1, y1, x2, y2, gradientSpread);
//     }

    QLinearGradient lg(QPointF(x1, y1), QPointF(x2, y2));
    lg.setStops(gradientStops);
    lg.setSpread(gradientSpread);
    painter->setBrush(lg);
}

void PaintCommands::command_gradient_setRadial(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double cx = convertToDouble(caps.at(1));
    double cy = convertToDouble(caps.at(2));
    double rad = convertToDouble(caps.at(3));
    double fx = convertToDouble(caps.at(4));
    double fy = convertToDouble(caps.at(5));

//     if (Lance::self()->options()->verboseMode) {
//         printf(" - gradient_setRadial center=(%.2f, %.2f), radius=%.2f focal=(%.2f, %.2f), "
//                "spread=%d\n",
//                cx, cy, rad, fx, fy, gradientSpread);
//     }

    QRadialGradient rg(QPointF(cx, cy), rad, QPointF(fx, fy));
    rg.setStops(gradientStops);
    rg.setSpread(gradientSpread);
    painter->setBrush(rg);
}

void PaintCommands::command_gradient_setConical(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double cx = convertToDouble(caps.at(1));
    double cy = convertToDouble(caps.at(2));
    double angle = convertToDouble(caps.at(3));

//     if (Lance::self()->options()->verboseMode) {
//         printf(" - gradient_setConical center=(%.2f, %.2f), angle=%.2f\n, spread=%d",
//                cx, cy, angle, gradientSpread);
//     }

    QConicalGradient cg(QPointF(cx, cy), angle);
    cg.setStops(gradientStops);
    cg.setSpread(gradientSpread);
    painter->setBrush(cg);

}


void PaintCommands::command_gradient_setSpread(QRegExp re)
{
    int spreadMethod = translate_enum(spreadMethodTable, re.cap(1), 3);

//     if (Lance::self()->options()->verboseMode)
//         printf(" - gradient_setSpread %d=[%s]\n", spreadMethod,
//                spreadMethodTable[spreadMethod]);

    gradientSpread = QGradient::Spread(spreadMethod);
}


void PaintCommands::command_surface_begin(QRegExp re)
{
    QStringList caps = re.capturedTexts();
    double x = convertToDouble(caps.at(1));
    double y = convertToDouble(caps.at(2));
    double w = convertToDouble(caps.at(3));
    double h = convertToDouble(caps.at(4));

    if (surface_painter) {
        fprintf(stderr, "ERROR: surface already active");
        return;
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - surface_begin, pos=[%.2f, %.2f], size=[%.2f, %.2f]\n", x, y, w, h);

    surface_image = QImage(qRound(w), qRound(h), QImage::Format_ARGB32_Premultiplied);
    surface_image.fill(0);
    surface_rect = QRectF(x, y, w, h);

    surface_painter = painter;
    painter = new QPainter(&surface_image);
}


void PaintCommands::command_surface_end(QRegExp)
{
    if (!surface_painter) {
        fprintf(stderr, "ERROR: surface not active");
        return;
    }

//     if (Lance::self()->options()->verboseMode)
//         printf(" - surface_end, pos=[%.2f, %.2f], size=[%.2f, %.2f]\n",
//                surface_rect.x(),
//                surface_rect.y(),
//                surface_rect.width(),
//                surface_rect.height());

    painter->end();

    delete painter;
    painter = surface_painter;
    painter->drawImage(surface_rect, surface_image);

    surface_painter = 0;
    surface_image = QImage();
    surface_rect = QRectF();
}

void PaintCommands::command_image_convertToFormat(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString srcName = caps.at(1);
    QString destName = caps.at(2);

    if (!imageMap.contains(srcName)) {
        fprintf(stderr, "ERROR(convertToFormat): no such image '%s'\n", qPrintable(srcName));
        return;
    }

    int format = translate_enum(imageFormatTable, caps.at(3),
                                QImage::Format_ARGB32_Premultiplied + 1);
    if (format < 0 || format > QImage::Format_ARGB32_Premultiplied) {
        fprintf(stderr, "ERROR(convertToFormat): invalid format %d = '%s'\n",
                format, qPrintable(caps.at(3)));
        return;
    }

    QImage src = imageMap[srcName];
    QImage dest = src.convertToFormat(QImage::Format(format),
                                      Qt::OrderedAlphaDither | Qt::OrderedDither);

//     if (Lance::self()->options()->verboseMode) {
//         printf(" - convertToFormat %s:%d -> %s:%d\n",
//                qPrintable(srcName), src.format(),
//                qPrintable(destName), dest.format());
//     }

    imageMap[destName] = dest;
}

void PaintCommands::command_textlayout_draw(QRegExp re)
{
    QStringList caps = re.capturedTexts();

    QString text = caps.at(1);
    double width = convertToDouble(caps.at(2));

//     if (Lance::self()->options()->verboseMode)
//         printf(" - textlayout_draw text='%s', width=%f\n", qPrintable(text), width);

    QFont copy = painter->font();
    copy.setPointSize(10);

    QTextLayout layout(text, copy, painter->device());
    layout.beginLayout();

    double y_offset = 0;

    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(width);
        line.setPosition(QPointF(0, y_offset));

        y_offset += line.height();
    }

    layout.draw(painter, QPointF(0, 0));
}
