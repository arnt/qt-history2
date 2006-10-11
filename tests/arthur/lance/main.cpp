#include "interactivewidget.h"
#include "widgets.h"
#include "paintcommands.h"

#include <QtCore>
#include <QtGui>
#include <qimage.h>
#include <QPicture>

#include <private/qmath_p.h>

#ifdef USE_CUSTOM_DEVICE
#include "customdevice.h"
#endif

#ifndef QT_NO_OPENGL
#include <qgl.h>
#include <QGLPixelBuffer>
#endif

#ifdef QT_PDF_SUPPORT
#include <qpdf.h>
#endif

// #define DO_QWS_DEBUGGING

#ifdef DO_QWS_DEBUGGING
extern bool qt_show_painter_debug_output = false;
#endif

static void printHelp()
{
    printf("\nUsage:\n\n"
           "  paintcmd [options] files\n"
           "\n"
           "  Options:\n"
           "    -widget         Paints the files to a widget on screen\n"
           "    -pixmap         Paints the files to a pixmap\n"
           "    -bitmap         Paints the files to a bitmap\n"
           "    -image          Paints the files to an image\n"
           "    -imagemono      Paints the files to a monochrome image\n"
#ifndef QT_NO_OPENGL
           "    -opengl         Paints the files to an OpenGL on screen\n"
#endif
#ifdef USE_CUSTOM_DEVICE
           "    -custom         Paints the files to the custom paint device\n"
#endif
#ifdef QT_PDF_SUPPORT
           "    -pdf            Paints to a pdf\n"
#endif
           "    -picture        Prints into a picture, then shows the result in a label\n"
           "    -printer        Prints the commands to a file called output.ps|pdf\n"
           "    -highres        Prints in highres mod\n"
           "    -grab           Paints the files to a pixmap called filename_qps.png\n"
           "    -i              Interactive mode.\n"
           "    -v              Verbose.\n"
           "    -bg-white       No checkers background\n"
           "    -commands       Displays all available commands\n");
}

static void displayCommands()
{
    printf("Drawing operations:\n"
           "  drawArc x y width height angle sweep\n"
           "  drawChord x y width height angle sweep\n"
           "  drawEllipse x y width height\n"
           "  drawLine x1 y1 x2 y2\n"
           "  drawPath pathname\n"
           "  drawPie x y width height angle sweep\n"
           "  drawPixmap pixmapfile x y width height sx sy sw sh\n"
           "  drawPolygon [ x1 y1 x2 y2 ... ] winding|oddeven\n"
           "  drawPolyline [ x1 y1 x2 y2 ... ]\n"
           "  drawRect x y width height\n"
           "  drawRoundRect x y width height xfactor yfactor\n"
           "  drawText x y \"text\"\n"
           "  drawTiledPixmap pixmapfile x y width height sx sy\n"
           "\n"
           "Compat functions for Qt 3:\n"
           "  qt3_drawArc x y width height angle sweep\n"
           "  qt3_drawChord x y width height angle sweep\n"
           "  qt3_drawEllipse x y width height\n"
           "  qt3_drawPie x y width height angle sweep\n"
           "  qt3_drawRect x y width height\n"
           "  qt3_drawRoundRect x y width height xfactor yfactor\n"
           "\n"
           "Path commands:\n"
           "  path_addEllipse pathname x y width height\n"
           "  path_addPolygon pathname [ x1 y1 x2 y2 ... ] winding?\n"
           "  path_addRect pathname x y width height\n"
           "  path_addText pathname x y \"text\"                        Uses current font\n"
           "  path_arcTo pathname x y width hegiht\n"
           "  path_closeSubpath pathname\n"
           "  path_createOutline pathname newoutlinename                Uses current pen\n"
           "  path_cubicTo pathname c1x c1y c2x c2y endx endy\n"
           "  path_lineTo pathname x y\n"
           "  path_moveTo pathname x y\n"
           "  path_setFillRule pathname winding?\n"
           "\n"
           "Painter configuration:\n"
           "  resetMatrix\n"
           "  restore\n"
           "  save\n"
           "  rotate degrees\n"
           "  translate dx dy\n"
           "  scale sx sy\n"
           "  mapQuadToQuad x0 y0 x1 y1 x2 y2 x3 y3  x4 y4 x5 y5 x6 y6 x7 y7"
           "  setMatrix m11 m12 m13 m21 m22 m23 m31 m32 m33"
           "  setBackground color pattern?\n"
           "  setBackgroundMode TransparentMode|OpaqueMode\n"
           "  setBrush pixmapfile\n"
           "  setBrush nobrush\n"
           "  setBrush color pattern\n"
           "  setBrush x1 y1 color1 x2 y2 color2                        gradient brush\n"
           "  setBrushOrigin x y\n"
           "  setFont \"fontname\" pointsize bold? italic?\n"
           "  setPen style color\n"
           "  setPen color width style capstyle joinstyle\n"
           "  setRenderHint LineAntialiasing\n"
           "  gradient_clearStops\n"
           "  gradient_appendStop pos color"
           "  gradient_setSpread [PadSpread|ReflectSpread|RepeatSpread]\n"
           "  gradient_setLinear x1 y1 x2 y2\n"
           "  gradient_setRadial center_x center_y radius focal_x focal_y\n"
           "  gradient_setConical center_x center_y angle\n"
           "\n"
           "Clipping commands:\n"
           "  region_addRect regionname x y width height\n"
           "  region_getClipRegion regionname\n"
           "  setClipRect x y width height\n"
           "  setClipRegion regionname\n"
           "  setClipping true|false\n"
           "  setClipPath pathname\n"
           "\n"
           "Various commands:\n"
           "  surface_begin x y width height\n"
           "  surface_end\n"
           "  pixmap_load filename name_in_script\n"
           "  image_load filename name_in_script\n");
}

static InteractiveWidget *interactive_widget = 0;

static void runInteractive()
{
    interactive_widget = new InteractiveWidget;
    interactive_widget->show();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
#ifdef DO_QWS_DEBUGGING
    qt_show_painter_debug_output = false;
#endif
    
    DeviceType type = WidgetType;
    bool checkers_background = true;
    
    QLocale::setDefault(QLocale::c());

    QStringList files;

    bool interactive = false;
    bool highres = false;
    bool show_cmp = false;
    int width = 800, height = 800;
    bool verboseMode = false;

#ifndef QT_NO_OPENGL
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    f.setStencil(true);
    f.setAlpha(true);
    f.setAlphaBufferSize(8);
    QGLFormat::setDefaultFormat(f);
#endif

    char *arg;
    for (int i=1; i<argc; ++i) {
        arg = argv[i];
        if (*arg == '-') {
            QString option = QString(arg + 1).toLower();
            if (option == "widget")
                type = WidgetType;
            else if (option == "bitmap")
                type = BitmapType;
            else if (option == "pixmap")
                type = PixmapType;
            else if (option == "image")
                type = ImageType;
            else if (option == "imagemono")
                type = ImageMonoType;
#ifndef QT_NO_OPENGL
            else if (option == "opengl")
                type = OpenGLType;
            else if (option == "pbuffer")
                type = OpenGLPBufferType;
#endif
#ifdef USE_CUSTOM_DEVICE
            else if (option == "custom")
                type = CustomType;
#endif
#ifdef QT_PDF_SUPPORT
            else if (option == "pdf")
                type = PdfType;
#endif
            else if (option == "picture")
                type = PictureType;
            else if (option == "printer")
                type = PrinterType;
            else if (option == "highres") {
                type = PrinterType;
                highres = true;
            }
            else if (option == "grab")
                type = GrabType;
            else if (option == "i")
                interactive = true;
            else if (option == "v")
                verboseMode = true;
            else if (option == "commands") {
                displayCommands();
                return 0;
            } else if (option == "w") {
                Q_ASSERT_X(i + 1 < argc, "main", "-w must be followed by a value");
                width = atoi(argv[++i]);
            } else if (option == "h") {
                Q_ASSERT_X(i + 1 < argc, "main", "-h must be followed by a value");
                height = atoi(argv[++i]);
            } else if (option == "cmp") {
                show_cmp = true;
            } else if (option == "bg-white") {
                checkers_background = false;
            }
        } else {
#if defined (Q_WS_WIN)
            QString input = QString::fromLocal8Bit(argv[i]);
            if (input.indexOf('*') >= 0) {
                QFileInfo info(input);
                QDir dir = info.dir();
                QFileInfoList infos = dir.entryInfoList(QStringList(info.fileName()));
                for (int ii=0; ii<infos.size(); ++ii)
                    files.append(infos.at(ii).absoluteFilePath());
            } else {
                files.append(input);
            }
#else
            files.append(QString(argv[i]));
#endif
        }
    }

    PaintCommands pcmd(QStringList(), 800, 800);
    pcmd.setVerboseMode(verboseMode);
    pcmd.setType(type);
    pcmd.setCheckersBackground(checkers_background);
    
    OnScreenWidget<QWidget> *qWidget = 0;
#ifndef QT_NO_OPENGL
    OnScreenWidget<QGLWidget> *qGLWidget = 0;
#endif
    if (interactive) {
        runInteractive();
        if (!files.isEmpty())
            interactive_widget->load(files.at(0));
    } else if (files.isEmpty()) {
        printHelp();
        return 0;
    } else {
        for (int j=0; j<files.size(); ++j) {
            const QString &fileName = files.at(j);
            QStringList content;

            QFile file(fileName);
            QFileInfo fileinfo(file);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream textFile(&file);
                QString script = textFile.readAll();
                content = script.split("\n", QString::SkipEmptyParts);
            } else {
                printf("failed to read file: '%s'\n", qPrintable(fileName));
                continue;
            }
            pcmd.setContents(content);

            if (show_cmp) {
                QString pmFile = QString(files.at(j)).replace(".", "_") + ".png";
                QPixmap pixmap(pmFile);
                if (!pixmap.isNull()) {
                    QLabel *label = new QLabel;
                    label->setWindowTitle("VERIFY: " + pmFile);
                    label->setPixmap(pixmap);
                    label->show();
                }
            }

            switch (type) {

            case WidgetType:
                {
                    qWidget = new OnScreenWidget<QWidget>;
                    qWidget->setVerboseMode(verboseMode);
                    qWidget->setType(type);
                    qWidget->setCheckersBackground(checkers_background);
                    qWidget->filename = files.at(j);
                    qWidget->setWindowTitle(fileinfo.filePath());
                    qWidget->cmds = content;
                    qWidget->resize(width, height);
                    qWidget->show();
                    break;
                }
#ifndef QT_NO_OPENGL
            case OpenGLPBufferType:
            {
                QGLPixelBuffer pbuffer(QSize(width, height));
                QPainter pt(&pbuffer);
                pcmd.setPainter(&pt);
                pcmd.setFilePath(fileinfo.absolutePath());
                pcmd.runCommands();
                pt.end();

                QImage image = pbuffer.toImage();
                QImage new_image(image.bits(), image.width(), image.height(), QImage::Format_ARGB32_Premultiplied);
                image.save("output_image.png", "PNG");

                QLabel *l = new QLabel(0);
                l->setPixmap(QPixmap::fromImage(new_image));
                l->resize(l->sizeHint());
                l->show();
                break;
            }
            case OpenGLType:
                {
                    qGLWidget = new OnScreenWidget<QGLWidget>;
                    qGLWidget->filename = files.at(j);
                    qGLWidget->setWindowTitle(fileinfo.filePath());
                    qGLWidget->cmds = content;
                    qGLWidget->resize(width, height);
                    qGLWidget->show();
                    break;
                }
#else
            case OpenGLType:
                printf("OpenGL type not supported in this Qt build\n");
                break;
#endif
#ifdef USE_CUSTOM_DEVICE
            case CustomType:
                {
                    CustomPaintDevice custom(width, height);
                    QPainter pt;
                    pt.begin(&custom);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();
                    custom.save("custom_output_pixmap.png", "PNG");
                    break;
                }
#endif
            case PixmapType:
                {
                    QPixmap pixmap(width, height);
                    pixmap.fill(Qt::white);
                    QPainter pt(&pixmap);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();
                    pixmap.save("output_pixmap.png", "PNG");
                    break;
                }

            case BitmapType:
                {
                    QBitmap bitmap(width, height);
                    QPainter pt(&bitmap);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();
                    bitmap.save("output_bitmap.png", "PNG");

                    QLabel *label = new QLabel(0);
                    label->setPixmap(bitmap);
                    label->resize(label->sizeHint());
                    label->show();
                    break;
                }

            case ImageMonoType:
            case ImageType:
                {
                    qDebug() << "Creating image";
                    QImage image(width, height, type == ImageMonoType
                                 ? QImage::Format_MonoLSB
                                 : QImage::Format_ARGB32_Premultiplied);
                    QPainter pt(&image);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();
                    image.convertToFormat(QImage::Format_ARGB32).save("output_image.png", "PNG");
                    QLabel *l = new QLabel(0);
                    l->setPixmap(QPixmap::fromImage(image));
                    l->resize(l->sizeHint());
                    l->show();
                    break;
                }

            case PictureType:
                {
                    QPicture pic;
                    QPainter pt(&pic);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();

                    QImage image(width, height, 32, 0, QImage::BigEndian);
                    pt.begin(&image);
                    pt.drawPicture(0, 0, pic);
                    pt.end();
                    QLabel *l = new QLabel(0);
                    l->setWindowTitle(fileinfo.absolutePath());
                    l->setPixmap(QPixmap::fromImage(image));
                    l->resize(l->sizeHint());
                    l->show();
                    break;
                }

            case PrinterType:
                {
                    QPrinter p(highres ? QPrinter::HighResolution : QPrinter::ScreenResolution);
                    p.setOutputToFile(true);
                    QString file = QString(files.at(j)).replace(".", "_") + ".ps";
                    p.setOutputFileName(file);
                    QPainter pt(&p);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();

                    printf("wrote file: %s\n", qPrintable(file));

                    Q_ASSERT(!p.paintingActive());
                    break;
                }
#ifdef QT_PDF_SUPPORT
            case PdfType:
                {
                    QPdf p;
                    p.setCompression(false);
                    QString file = QString(files.at(j)).replace(".", "_") + ".pdf";
                    p.setFileName(file);
                    p.setPageSize(QPrinter::A4);
                    QPainter pt(&p);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();

                    printf("write file: %s\n", qPrintable(file));
                    break;
                }
#endif
            case GrabType:
                {
                    QPixmap pixmap(width, height);
                    pixmap.fill(Qt::white);
                    QPainter pt(&pixmap);
                    pcmd.setPainter(&pt);
                    pcmd.setFilePath(fileinfo.absolutePath());
                    pcmd.runCommands();
                    pt.end();
                    QString filename = QString(files.at(j)).replace(".", "_") + ".png";
                    pixmap.save(filename, "PNG");
                    printf("%s grabbed to %s\n", qPrintable(files.at(j)), qPrintable(filename));
                    break;
                }

            default:
                break;
            }
        }
    }

    if (type == WidgetType || type ==  ImageType || type == ImageMonoType
        || type == OpenGLType || type == OpenGLPBufferType || type == PictureType || type == BitmapType
        || interactive) {
        QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
        app.exec();
    }

    delete qWidget;
#ifndef QT_NO_OPENGL
    delete qGLWidget;
#endif
    return 0;
}

