/*******************************************************************
 *
 *  Copyright 2006  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QFontDatabase>
#include <QCompleter>
#include <QDirModel>
#include <qendian.h>
#include "ui_generator.h"

#include <qfont.h>
#include <private/qtextengine_p.h>
#include <private/qfontengine_p.h>
#include <qtextlayout.h>

static QString generate(const QString &family, int pixelSize, const QString &sampleText)
{
    QFont font;
    font.setFamily(family);
    font.setPixelSize(pixelSize);
    font.setStyleStrategy(QFont::NoFontMerging);
    font.setKerning(false);

    QTextEngine e(sampleText, font);
    e.itemize();
    e.shape(0);

    QString result;
    QFontEngine *fe = e.fontEngine(e.layoutData->items[0]);
    result = "# Using font '" + fe->fontDef.family + "'\n";

    {
        const QByteArray headTable = fe->getSfntTable(FT_MAKE_TAG('h', 'e', 'a', 'd'));
        const quint32 revision = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(headTable.constData()) + 4);
        result += "# Font Revision: " + QString::number(revision) + "\n";
    }

    result += "# Input Text: " + sampleText;
    result += "\n# Codepoints: ";
    for (int i = 0; i < sampleText.length(); ++i)
        result += "0x" + QString::number(sampleText.at(i).unicode(), 16) + " ";
    result += "\n# Glyphs [glyph] [x advance] [x offset] [y offset]\n";
    for (int i = 0; i < e.layoutData->items[0].num_glyphs; ++i) {
        QGlyphLayout *g = e.layoutData->glyphPtr + i;
        result += "0x" + QString::number(g->glyph, 16);
        result += ' ';
        result += QString::number(g->advance.x.toReal());
        result += ' ';
        result += QString::number(g->offset.x.toReal());
        result += ' ';
        result += QString::number(g->offset.y.toReal());
        result += '\n';
    }

    return result;
}

class Generator : public QWidget, public Ui_Generator
{
    Q_OBJECT
public:
    Generator();
public slots:
    void on_chooseFontFile_clicked();
    void on_fontPath_editingFinished();
    void on_sample_editingFinished();

private:
    int fontId;
};

Generator::Generator()
{
    setupUi(this);

    QCompleter *completer = new QCompleter(fontPath);
    QDirModel *dirModel = new QDirModel(completer);
    dirModel->setFilter(QDir::AllEntries | QDir::Hidden);
    completer->setModel(dirModel);
    fontPath->setCompleter(completer);

    fontId = -1;
}

void Generator::on_chooseFontFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, /*caption*/ "Select TrueType Font",
                                                /*dir*/ QString(),
                                                /*filter*/ "*.ttf");
    if (path.isEmpty())
        return;
    fontPath->setText(path);
    on_fontPath_editingFinished();
}

void Generator::on_fontPath_editingFinished()
{
    const QString path = fontPath->text();
    if (!QFile::exists(path)) {
        fontPath->setStyleSheet("background-color: red");
        return;
    }

    if (fontId != -1)
        QFontDatabase::removeApplicationFont(fontId);

    fontId = QFontDatabase::addApplicationFont(path);
    if (fontId == -1 || QFontDatabase::applicationFontFamilies(fontId).isEmpty()) {
        fontPath->setStyleSheet("background-color: red");
        return;
    } else {
        fontPath->setStyleSheet(QString());
    }
    on_sample_editingFinished();
}

void Generator::on_sample_editingFinished()
{
    if (sample->text().isEmpty()) {
        glyphOutput->setPlainText("Error: No sample text");
        return;
    }
    if (fontId == -1) {
        glyphOutput->setPlainText("Error: No valid font selected");
        return;
    }
    glyphOutput->clear();

    QString result = generate(QFontDatabase::applicationFontFamilies(fontId).first(),
                              /*pixelSize*/ 12,
                              sample->text());
    glyphOutput->setPlainText(result);
}

static void help()
{
    printf("usage: generator /path/to/truetype/font.ttf pixelsize /path/to/sample-text-file.txt\n");
    exit(1);
}

static int mainWithoutGui(int argc, char **argv)
{
    QApplication app(argc, argv);

    QStringList args = app.arguments();

    QString fontFile = args.at(1);
    if (!QFile::exists(fontFile)) {
        printf("font file %s does not exist.\n", qPrintable(fontFile));
        return 1;
    }
    bool ok = false;
    int pixelSize =  args.at(2).toInt(&ok);
    if (!ok) {
        printf("invalid pixel size: %s", qPrintable(args.at(2)));
        return 1;
    }

    QString sampleFile = args.at(3);
    if (!QFile::exists(sampleFile)) {
        printf("sample file %s does not exist.\n", qPrintable(sampleFile));
        return 1;
    }

    int id = QFontDatabase::addApplicationFont(fontFile);
    if (id == -1) {
        printf("cannot use font file %s\n", qPrintable(fontFile));
        return 1;
    }
    QStringList families = QFontDatabase::applicationFontFamilies(id);
    if (families.count() != 1) {
        printf("wrong number of families in font file %s: %d -- should be 1\n", qPrintable(fontFile), families.count());
        return 1;
    }

    QFile f(sampleFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printf("cannot open sample file %s for reading\n", qPrintable(sampleFile));
        return 1;
    }

    QString sampleText = QString::fromUtf8(f.readAll());
    if (sampleText.endsWith(QLatin1Char('\r')))
        sampleText.chop(1);
    if (sampleText.endsWith(QLatin1Char('\n')))
        sampleText.chop(1);

    QString result = generate(families.first(), pixelSize, sampleText);

    puts(result.toUtf8().constData());

    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 4)
        return mainWithoutGui(argc, argv);
    else if (argc != 1) {
        help();
        return 1;
    }

    QApplication app(argc, argv);

    Generator w;
    w.show();

    return app.exec();
}

#include "main.moc"
