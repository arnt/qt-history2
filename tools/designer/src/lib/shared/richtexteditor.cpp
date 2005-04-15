#include <QtCore/QPointer>
#include <QtCore/QMap>

#include <QtGui/QToolBar>
#include <QtGui/QComboBox>
#include <QtGui/QAction>
#include <QtGui/QTextCursor>
#include <QtGui/QPainter>
#include <QtGui/QIcon>
#include <QtGui/QMoveEvent>
#include <QtGui/QTextDocument>
#include <QtGui/QTextBlock>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>

#include <QtCore/qdebug.h>

#include <iconloader.h>
#include "richtexteditor.h"

static bool operator < (const QColor &c1, const QColor &c2)
{
    if (c1.red() != c2.red())
        return c1.red() < c2.red();
    if (c1.green() != c2.green())
        return c1.green() < c2.green();
    return c1.blue() < c2.blue();
}

class RichTextEditorToolBar : public QToolBar
{
    Q_OBJECT
public:
    RichTextEditorToolBar(RichTextEditor *editor, QWidget *parent = 0);

public slots:
    void updateActions();

private slots:
    void sizeInputActivated(const QString &size);
    void colorInputActivated(const QString &color);

private:
    QAction *m_bold_action;
    QAction *m_italic_action;
    QAction *m_underline_action;
    QComboBox *m_font_size_input;
    QComboBox *m_color_input;

    QPointer<RichTextEditor> m_editor;

    typedef QMap<QColor, QString> ColorMap;
    ColorMap m_color_map;
};

static QAction *createCheckableAction(const QIcon &icon, const QString &text,
                                    QObject *receiver, const char *slot,
                                    QObject *parent = 0)
{
    QAction *result = new QAction(parent);
    result->setIcon(icon);
    result->setText(text);
    result->setCheckable(true);
    result->setChecked(false);
    QObject::connect(result, SIGNAL(checked(bool)), receiver, slot);
    return result;
}

static QIcon iconForColor(const QColor &color)
{
    QPixmap result(12, 12);
    QPainter painter(&result);
    painter.setPen(Qt::black);
    painter.setBrush(color);
    painter.drawRect(0, 0, result.width() - 1, result.height() - 1);
    painter.end();
    return QIcon(result);
}

RichTextEditorToolBar::RichTextEditorToolBar(RichTextEditor *editor,
                                                QWidget *parent)
    : QToolBar(parent)
{
    m_editor = editor;

    m_bold_action = createCheckableAction(createIconSet(QLatin1String("textbold.png")),
            tr("Bold"), editor, SLOT(setFontBold(bool)), this);
    m_bold_action->setShortcut(tr("CTRL+b"));
    addAction(m_bold_action);

    m_italic_action = createCheckableAction(createIconSet(QLatin1String("textitalic.png")),
            tr("Italic"), editor, SLOT(setFontItalic(bool)), this);
    m_italic_action->setShortcut(tr("CTRL+I"));

    addAction(m_italic_action);
    m_underline_action = createCheckableAction(createIconSet(QLatin1String("textunder.png")),
            tr("Underline"), editor, SLOT(setFontUnderline(bool)), this);
    m_underline_action->setShortcut(tr("CTRL+U"));
    addAction(m_underline_action);

    m_font_size_input = new QComboBox(this);
    m_font_size_input->setEditable(false);
    for (int i = 4; i < 30; ++i)
        m_font_size_input->addItem(QString::number(i));
    connect(m_font_size_input, SIGNAL(activated(QString)),
                this, SLOT(sizeInputActivated(QString)));
    addWidget(m_font_size_input);

    QStringList color_names = QColor::colorNames();
    color_names.removeAll(QLatin1String("transparent"));
    foreach (QString color, color_names)
        m_color_map.insert(QColor(color), color);

    m_color_input = new QComboBox(this);
    foreach (QString color, color_names)
        m_color_input->addItem(iconForColor(color), color);
    connect(m_color_input, SIGNAL(activated(QString)),
                this, SLOT(colorInputActivated(QString)));
    addWidget(m_color_input);

    connect(editor, SIGNAL(textChanged()), this, SLOT(updateActions()));

    updateActions();
}

void RichTextEditorToolBar::colorInputActivated(const QString &s)
{
    QColor color(s);
    if (!color.isValid())
        return;

    bool block = m_editor->blockSignals(true);
    m_editor->setTextColor(color);
    m_editor->blockSignals(block);
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
    if (m_editor == 0)
        return;

    bool ok;
    int i = size.toInt(&ok);
    if (!ok)
        return;

    bool block = m_editor->blockSignals(true);
    m_editor->setFontPointSize(i);
    m_editor->blockSignals(block);
}

static void setCheckAction(QAction *action, bool b)
{
    bool block = action->blockSignals(true);
    action->setChecked(b);
    action->blockSignals(block);
}

void RichTextEditorToolBar::updateActions()
{
    if (m_editor == 0) {
        setEnabled(false);
        return;
    }

    QTextCursor cursor = m_editor->textCursor();

    QTextCharFormat char_format = cursor.charFormat();
    setCheckAction(m_bold_action, char_format.fontWeight() == QFont::Bold);
    setCheckAction(m_italic_action, char_format.fontItalic());
    setCheckAction(m_underline_action, char_format.fontUnderline());

    bool block = m_font_size_input->blockSignals(true);
    QString size = QString::number((int) char_format.fontPointSize());
    int idx = m_font_size_input->findText(size);
    m_font_size_input->setCurrentIndex(idx);
    m_font_size_input->blockSignals(block);

    block = m_color_input->blockSignals(true);
    QString color = m_color_map.value(m_editor->textColor());
    idx = m_color_input->findText(color);
    m_color_input->setCurrentIndex(idx);
    m_color_input->blockSignals(block);
}

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
                this, SIGNAL(textChanged()));
}

QToolBar *RichTextEditor::createToolBar(QWidget *parent)
{
    return new RichTextEditorToolBar(this, parent);
}

void RichTextEditor::setFontBold(bool b)
{
    if (b)
        setFontWeight(QFont::Bold);
    else
        setFontWeight(QFont::Normal);
}

void RichTextEditor::setFontPointSize(double d)
{
    QTextEdit::setFontPointSize(qreal(d));
}

void RichTextEditor::setText(const QString &text)
{
    setHtml(text);
}

void RichTextEditor::setDefaultFont(const QFont &font)
{
    document()->setDefaultFont(font);
    setFontPointSize(font.pointSize());
    emit textChanged();
}

Qt::TextFormat RichTextEditor::detectFormat() const
{
    Qt::TextFormat result = Qt::PlainText;

    QFont default_font = document()->defaultFont();
    QTextCursor cursor(document()->begin());
    cursor.movePosition(QTextCursor::End);
    while (!cursor.atStart()) {
        QFont font = cursor.charFormat().font();
        if (font != default_font) {
            result = Qt::RichText;
            break;
        }
        cursor.movePosition(QTextCursor::Left);
    }

    return result;
};

QString RichTextEditor::text(Qt::TextFormat format) const
{
    bool richtext = true;

    if (format == Qt::PlainText)
        richtext = false;
    else if (format != Qt::RichText)
        richtext = detectFormat() == Qt::RichText;

    if (richtext)
        return toHtml();
    else
        return toPlainText();
}

RichTextEditorDialog::RichTextEditorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit text"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(1);
    m_editor = new RichTextEditor(this);
    QToolBar *tool_bar = m_editor->createToolBar(this);
    layout->addWidget(tool_bar);
    layout->addWidget(m_editor);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout->addLayout(layout2);

    layout2->addStretch();
    QPushButton *cancel_button = new QPushButton(tr("&Cancel"), this);
    layout2->addWidget(cancel_button);
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    QPushButton *ok_button = new QPushButton(tr("&Ok"), this);
    layout2->addWidget(ok_button);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    ok_button->setDefault(true);
}

RichTextEditor *RichTextEditorDialog::editor()
{
    return m_editor;
}

#include "richtexteditor.moc"
