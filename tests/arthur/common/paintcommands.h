#ifndef PAINTCOMMANDS_H
#define PAINTCOMMANDS_H

#include <qcolor.h>
#include <qmap.h>
#include <qpainterpath.h>
#include <qregion.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qbrush.h>

class QPainter;
class QRegExp;
#ifndef QT_NO_OPENGL
class QGLPixelBuffer;
#endif

enum DeviceType {
    WidgetType,
    BitmapType,
    PixmapType,
    ImageType,
    ImageMonoType,
    OpenGLType,
    OpenGLPBufferType,
    PictureType,
    PrinterType,
    PdfType,
    PsType,
    GrabType,
    CustomType
};

class PaintCommands
{
public:
    PaintCommands(const QStringList &cmds, int w, int h)
        : painter(0), surface_painter(0),
          commands(cmds), gradientSpread(QGradient::PadSpread),
          width(w), height(h), verboseMode(false), type(WidgetType),
          checkers_background(true)
    { }

    void setCheckersBackground(bool b)
    {
        checkers_background = b;
    }

    void setContents(const QStringList &cmds)
    {
        commands = cmds;
    }

    void setPainter(QPainter *pt)
    {
        painter = pt;
    }

    void setType(DeviceType t)
    {
        type = t;
    }

    void runCommand(const QString &command);
    void runCommands();
    void setFilePath(const QString &path) { filepath = path; }

    void setControlPoints(const QVector<QPointF> &points)
    {
        controlPoints = points;
    }
    void setVerboseMode(bool v)
    {
        verboseMode = v;
    }

private:
    int convertToInt(const QString &str);
    double convertToDouble(const QString &str);
    float convertToFloat(const QString &str);
    QColor convertToColor(const QString &str);

    void command_abort(QRegExp re);
    void command_comment(QRegExp re);
    void command_import(QRegExp re);
    void command_begin_block(QRegExp re);
    void command_end_block(QRegExp re);
    void command_repeat_block(QRegExp re);

    void command_qt3_drawArc(QRegExp re);
    void command_qt3_drawChord(QRegExp re);
    void command_qt3_drawEllipse(QRegExp re);
    void command_qt3_drawPie(QRegExp re);
    void command_qt3_drawRect(QRegExp re);
    void command_qt3_drawRoundRect(QRegExp re);

    void command_surface_begin(QRegExp re);
    void command_surface_end(QRegExp re);

    void command_bitmap_load(QRegExp re);
    void command_drawArc(QRegExp re);
    void command_drawChord(QRegExp re);
    void command_drawConvexPolygon(QRegExp re);
    void command_drawEllipse(QRegExp re);
    void command_drawImage(QRegExp re);
    void command_drawLine(QRegExp re);
    void command_drawPath(QRegExp re);
    void command_drawPie(QRegExp re);
    void command_drawPixmap(QRegExp re);
    void command_drawPoint(QRegExp re);
    void command_drawPolygon(QRegExp re);
    void command_drawPolyline(QRegExp re);
    void command_drawRect(QRegExp re);
    void command_drawRoundRect(QRegExp re);
    void command_drawText(QRegExp re);
    void command_drawTiledPixmap(QRegExp re);
    void command_gradient_appendStop(QRegExp re);
    void command_gradient_clearStops(QRegExp re);
    void command_gradient_setConical(QRegExp re);
    void command_gradient_setLinear(QRegExp re);
    void command_gradient_setRadial(QRegExp re);
    void command_gradient_setSpread(QRegExp re);
    void command_image_convertToFormat(QRegExp re);
    void command_image_load(QRegExp re);
    void command_image_setColor(QRegExp re);
    void command_image_setNumColors(QRegExp re);
    void command_noop(QRegExp re);
    void command_path_addEllipse(QRegExp re);
    void command_path_addPolygon(QRegExp re);
    void command_path_addRect(QRegExp re);
    void command_path_addText(QRegExp re);
    void command_path_arcTo(QRegExp re);
    void command_path_closeSubpath(QRegExp re);
    void command_path_createOutline(QRegExp re);
    void command_path_cubicTo(QRegExp re);
    void command_path_debugPrint(QRegExp re);
    void command_path_getClipPath(QRegExp re);
    void command_path_lineTo(QRegExp re);
    void command_path_moveTo(QRegExp re);
    void command_path_setFillRule(QRegExp re);
    void command_pen_setDashPattern(QRegExp re);
    void command_pen_setCosmetic(QRegExp re);
    void command_pixmap_load(QRegExp re);
    void command_pixmap_setMask(QRegExp re);
    void command_region_addEllipse(QRegExp re);
    void command_region_addRect(QRegExp re);
    void command_region_getClipRegion(QRegExp re);
    void command_resetMatrix(QRegExp re);
    void command_restore(QRegExp re);
    void command_rotate(QRegExp re);
    void command_save(QRegExp re);
    void command_scale(QRegExp re);
    void command_setBackground(QRegExp re);
    void command_setOpacity(QRegExp re);
    void command_setBgMode(QRegExp re);
    void command_setBrush(QRegExp re);
    void command_setBrushOrigin(QRegExp re);
    void command_mapQuadToQuad(QRegExp re);
    void command_setMatrix(QRegExp re);

    void command_brushTranslate(QRegExp re);
    void command_brushRotate(QRegExp re);
    void command_brushScale(QRegExp re);
    void command_brushShear(QRegExp re);

    void command_setClipPath(QRegExp re);
    void command_setClipRect(QRegExp re);
    void command_setClipRectangle(QRegExp re);
    void command_setClipRegion(QRegExp re);
    void command_setClipping(QRegExp re);
    void command_setCompositionMode(QRegExp re);
    void command_setFont(QRegExp re);
    void command_setPen(QRegExp re);
    void command_setPen2(QRegExp re);

    void command_setRenderHint(QRegExp re);
    void command_textlayout_draw(QRegExp re);
    void command_translate(QRegExp re);

    void insertAt(int commandIndex, const QStringList &newCommands);

    QPainter *painter;
    QPainter *surface_painter;
    QImage surface_image;
    QPixmap surface_pixmap;
#ifndef QT_NO_OPENGL
    QGLPixelBuffer *surface_pbuffer;
#endif
    QRectF surface_rect;
    QStringList commands;
    QString currentCommand;
    int currentCommandIndex;
    QString filepath;
    QMap<QString, QStringList> blockMap;
    QMap<QString, QPainterPath> pathMap;
    QMap<QString, QPixmap> pixmapMap;
    QMap<QString, QImage> imageMap;
    QMap<QString, QRegion> regionMap;
    QGradientStops gradientStops;
    QGradient::Spread gradientSpread;
    bool abort;
    int width;
    int height;

    bool verboseMode;
    DeviceType type;
    bool checkers_background;

    QVector<QPointF> controlPoints;
};

#endif // PAINTCOMMANDS_H
