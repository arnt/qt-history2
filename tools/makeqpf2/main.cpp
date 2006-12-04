#include <QtGui>

#include "qpf2.h"

#include <private/qfontengine_p.h>

static void help()
{
    printf("usage:\n");
    printf("makeqpf fontname pixelsize [italic] [bold] [--exclude-cmap] [-v]\n");
    printf("makeqpf -dump file.qpf2\n");
    exit(0);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, QApplication::Tty);
    const QStringList arguments = app.arguments();

    if (arguments.count() <= 1)
        help();
    const QString &firstArg = arguments.at(1);
    if (firstArg == QLatin1String("-h") || firstArg == QLatin1String("--help"))
        help();
    if (firstArg == QLatin1String("-dump")) {
        if (arguments.count() != 3)
            help();
        QFile f(arguments.at(2));
        if (!f.open(QIODevice::ReadOnly)) {
            printf("cannot open %s\n", qPrintable(arguments.at(2)));
            exit(1);
        }

        QByteArray qpf = f.readAll();
        f.close();

        QPF::dump(qpf);
        return 0;
    }

    if (arguments.count() < 3) help();

    QFont font;

    QString fontName = firstArg;
    if (QFile::exists(fontName)) {
        int id = QFontDatabase::addApplicationFont(fontName);
        if (id == -1) {
            printf("cannot open font %s", qPrintable(fontName));
            help();
        }
        QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (families.isEmpty()) {
            printf("cannot find any font families in %s", qPrintable(fontName));
            help();
        }
        fontName = families.first();
    }
    font.setFamily(fontName);

    bool ok = false;
    int pixelSize = arguments.at(2).toInt(&ok);
    if (!ok) help();
    font.setPixelSize(pixelSize);

    bool includeCMap = true;

    for (int i = 3; i < arguments.count(); ++i) {
        const QString &arg = arguments.at(i);
        if (arg == QLatin1String("italic")) {
            font.setItalic(true);
        } else if (arg == QLatin1String("bold")) {
            font.setBold(true);
        } else if (arg == QLatin1String("--exclude-cmap")) {
            includeCMap = false;
        } else if (arg == QLatin1String("-v")) {
            ++QPF::debugVerbosity;
        } else {
            printf("unknown option %s\n", qPrintable(arg));
            help();
        }
    }

    font.setStyleStrategy(QFont::NoFontMerging);

    QTextEngine engine("Test", font);
    engine.itemize();
    engine.shape(0);
    QFontEngine *fontEngine = engine.fontEngine(engine.layoutData->items[0]);
    if (fontEngine->type() == QFontEngine::Multi)
        fontEngine = static_cast<QFontEngineMulti *>(fontEngine)->engine(0);

    QByteArray qpf = QPF::generate(fontEngine, includeCMap);

    QString fileName = fontName.toLower() + "_" + QString::number(pixelSize)
                       + "_" + QString::number(font.weight())
                       + (font.italic() ? "_italic" : "")
                       + ".qpf2";
    fileName.replace(QLatin1Char(' '), QLatin1Char('_'));

    QFile f(fileName);
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(qpf);
    f.close();

    QFontEngine::FaceId face = fontEngine->faceId();
    if (includeCMap) {
        printf("Created %s from %s\n", qPrintable(fileName), face.filename.constData());
    } else {
        printf("Created %s from %s excluding the character-map\n", qPrintable(fileName), face.filename.constData());
        printf("The TrueType font file is therefore required for the font to work\n");
    }

    return 0;
}

