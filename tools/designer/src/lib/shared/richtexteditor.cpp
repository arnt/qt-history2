/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::RichTextEditorDialog
*/

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextCursor>
#include <QtGui/QPainter>
#include <QtGui/QIcon>
#include <QtGui/QMenu>
#include <QtGui/QMoveEvent>
#include <QtGui/QTabWidget>
#include <QtGui/QTextDocument>
#include <QtGui/QTextBlock>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>

#include <QtCore/qdebug.h>

#include "iconloader_p.h"
#include "richtexteditor_p.h"
#include "htmlhighlighter_p.h"
#include "qtcolorbutton_p.h"
#include "ui_addlinkdialog.h"

QT_BEGIN_NAMESPACE

/*
static bool operator < (const QColor &c1, const QColor &c2)
{
    if (c1.red() != c2.red())
        return c1.red() < c2.red();
    if (c1.green() != c2.green())
        return c1.green() < c2.green();
    return c1.blue() < c2.blue();
}
*/

namespace qdesigner_internal {

class AddLinkDialog : public QDialog
{
    Q_OBJECT

public:
    AddLinkDialog(RichTextEditor *editor, QWidget *parent = 0);
    ~AddLinkDialog();

public slots:
    void accept();

private:
    RichTextEditor *m_editor;
    Ui::AddLinkDialog *m_ui;
};

AddLinkDialog::AddLinkDialog(RichTextEditor *editor, QWidget *parent)
    : QDialog(parent)
{
    m_ui = new Ui::AddLinkDialog;
    m_ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_editor = editor;
    const QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        m_ui->titleInput->setText(cursor.selectedText());
        m_ui->urlInput->setFocus();
    } else {
        m_ui->titleInput->setFocus();
    }
}

AddLinkDialog::~AddLinkDialog()
{
    delete m_ui;
}

void AddLinkDialog::accept()
{
    const QString title = m_ui->titleInput->text();
    const QString url = m_ui->urlInput->text();

    if (!title.isEmpty()) {
        QString html = QLatin1String("<a href=\"");
        html += url;
        html += QLatin1String("\">");
        html += title;
        html += QLatin1String("</a>");

        m_editor->insertHtml(html);
    }

    m_ui->titleInput->clear();
    m_ui->urlInput->clear();

    QDialog::accept();
}

class HtmlTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    HtmlTextEdit(QWidget *parent = 0)
        : QTextEdit(parent)
    {}

    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void actionTriggered(QAction *action);
};

void HtmlTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    QMenu *htmlMenu = new QMenu(tr("Insert HTML entity"), menu);

    typedef struct {
        const char *text;
        const char *entity;
    } Entry;

    const Entry entries[] = {
        { "&&amp; (&&)", "&amp;" },
        { "&&nbsp;", "&nbsp;" },
        { "&&lt; (<)", "&lt;" },
        { "&&gt; (>)", "&gt;" },
        { "&&copy; (Copyright)", "&copy;" },
        { "&&reg; (Trade Mark)", "&reg;" },
    };

    for (int i = 0; i < 6; ++i) {
        QAction *entityAction = new QAction(QLatin1String(entries[i].text),
                                            htmlMenu);
        entityAction->setData(QLatin1String(entries[i].entity));
        htmlMenu->addAction(entityAction);
    }

    menu->addMenu(htmlMenu);
    connect(htmlMenu, SIGNAL(triggered(QAction*)),
                      SLOT(actionTriggered(QAction*)));
    menu->exec(event->globalPos());
    delete menu;
}

void HtmlTextEdit::actionTriggered(QAction *action)
{
    insertPlainText(action->data().toString());
}

class RichTextEditorToolBar : public QToolBar
{
    Q_OBJECT
public:
    RichTextEditorToolBar(RichTextEditor *editor, QWidget *parent = 0);

public slots:
    void updateActions();

private slots:
    void alignmentActionTriggered(QAction *action);
    void sizeInputActivated(const QString &size);
    //void colorInputActivated(const QString &color);
    void colorChanged(const QColor &color);
    void setVAlignSuper(bool super);
    void setVAlignSub(bool sub);
    void insertLink();

private:
    QAction *m_bold_action;
    QAction *m_italic_action;
    QAction *m_underline_action;
    QAction *m_valign_sup_action;
    QAction *m_valign_sub_action;
    QAction *m_align_left_action;
    QAction *m_align_center_action;
    QAction *m_align_right_action;
    QAction *m_align_justify_action;
    QAction *m_link_action;
    QtColorButton *m_color_button;
    QComboBox *m_font_size_input;
    //QComboBox *m_color_input;

    QPointer<RichTextEditor> m_editor;

    //typedef QMap<QColor, QString> ColorMap;
    //ColorMap m_color_map;
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
    if (slot) {
        QObject::connect(result, SIGNAL(triggered(bool)), receiver, slot);
    }
    return result;
}

/*
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
*/

RichTextEditorToolBar::RichTextEditorToolBar(RichTextEditor *editor,
                                             QWidget *parent)
    : QToolBar(parent)
{
    m_editor = editor;

    // Font size combo box

    m_font_size_input = new QComboBox(this);
    m_font_size_input->setEditable(false);
    QList<int> font_sizes = QFontDatabase::standardSizes();
    foreach (int font_size, font_sizes)
        m_font_size_input->addItem(QString::number(font_size));

    connect(m_font_size_input, SIGNAL(activated(QString)),
            this, SLOT(sizeInputActivated(QString)));
    addWidget(m_font_size_input);

    addSeparator();

    // Bold, italic and underline buttons

    m_bold_action = createCheckableAction(
            createIconSet(QLatin1String("textbold.png")),
            tr("Bold"), editor, SLOT(setFontBold(bool)), this);
    m_bold_action->setShortcut(tr("CTRL+B"));
    addAction(m_bold_action);

    m_italic_action = createCheckableAction(
            createIconSet(QLatin1String("textitalic.png")),
            tr("Italic"), editor, SLOT(setFontItalic(bool)), this);
    m_italic_action->setShortcut(tr("CTRL+I"));
    addAction(m_italic_action);

    m_underline_action = createCheckableAction(
            createIconSet(QLatin1String("textunder.png")),
            tr("Underline"), editor, SLOT(setFontUnderline(bool)), this);
    m_underline_action->setShortcut(tr("CTRL+U"));
    addAction(m_underline_action);

    addSeparator();

    // Left, center, right and justified alignment buttons

    QActionGroup *alignment_group = new QActionGroup(this);
    connect(alignment_group, SIGNAL(triggered(QAction*)),
                             SLOT(alignmentActionTriggered(QAction*)));

    m_align_left_action = createCheckableAction(
            createIconSet(QLatin1String("textleft.png")),
            tr("Left Align"), editor, 0, alignment_group);
    addAction(m_align_left_action);

    m_align_center_action = createCheckableAction(
            createIconSet(QLatin1String("textcenter.png")),
            tr("Center"), editor, 0, alignment_group);
    addAction(m_align_center_action);

    m_align_right_action = createCheckableAction(
            createIconSet(QLatin1String("textright.png")),
            tr("Right Align"), editor, 0, alignment_group);
    addAction(m_align_right_action);

    m_align_justify_action = createCheckableAction(
            createIconSet(QLatin1String("textjustify.png")),
            tr("Justify"), editor, 0, alignment_group);
    addAction(m_align_justify_action);

    addSeparator();

    // Superscript and subscript buttons

    m_valign_sup_action = createCheckableAction(
            createIconSet(QLatin1String("textsuperscript.png")),
            tr("Superscript"),
            this, SLOT(setVAlignSuper(bool)), this);
    addAction(m_valign_sup_action);

    m_valign_sub_action = createCheckableAction(
            createIconSet(QLatin1String("textsubscript.png")),
            tr("Subscript"),
            this, SLOT(setVAlignSub(bool)), this);
    addAction(m_valign_sub_action);

    addSeparator();

    // Insert hyperlink button

    m_link_action = new QAction(this);
    m_link_action->setIcon(createIconSet(QLatin1String("textanchor.png")));
    m_link_action->setText(tr("Insert &Link"));
    QObject::connect(m_link_action, SIGNAL(triggered(bool)),
                                    SLOT(insertLink()));
    addAction(m_link_action);

    addSeparator();

    /* Font color combo box not used
    QStringList color_names = QColor::colorNames();
    color_names.removeAll(QLatin1String("transparent"));
    foreach (QString color, color_names)
        m_color_map.insert(QColor(color), color);

    m_color_input = new QComboBox();
    foreach (QString color, color_names)
        m_color_input->addItem(iconForColor(color), color);
    connect(m_color_input, SIGNAL(activated(QString)),
            this, SLOT(colorInputActivated(QString)));
    addWidget(m_color_input);
    */

    // Text color button

    m_color_button = new QtColorButton(this);
    connect(m_color_button, SIGNAL(colorChanged(QColor)),
            this, SLOT(colorChanged(QColor)));
    addWidget(m_color_button);
    m_color_button->setAutoRaise(false);

    connect(editor, SIGNAL(textChanged()), this, SLOT(updateActions()));

    updateActions();
}

void RichTextEditorToolBar::alignmentActionTriggered(QAction *action)
{
    Qt::Alignment new_alignment;

    if (action == m_align_left_action) {
        new_alignment = Qt::AlignLeft;
    } else if (action == m_align_center_action) {
        new_alignment = Qt::AlignCenter;
    } else if (action == m_align_right_action) {
        new_alignment = Qt::AlignRight;
    } else {
        new_alignment = Qt::AlignJustify;
    }

    m_editor->setAlignment(new_alignment);
}

/*
void RichTextEditorToolBar::colorInputActivated(const QString &s)
{
    QColor color(s);
    if (!color.isValid())
        return;

    m_editor->setTextColor(color);
    m_editor->setFocus();
}
*/

void RichTextEditorToolBar::colorChanged(const QColor &color)
{
    m_editor->setTextColor(color);
    m_editor->setFocus();
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
    bool ok;
    int i = size.toInt(&ok);
    if (!ok)
        return;

    m_editor->setFontPointSize(i);
    m_editor->setFocus();
}

void RichTextEditorToolBar::setVAlignSuper(bool super)
{
    const QTextCharFormat::VerticalAlignment align = super ?
        QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sub_action->setChecked(false);
}

void RichTextEditorToolBar::setVAlignSub(bool sub)
{
    const QTextCharFormat::VerticalAlignment align = sub ?
        QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sup_action->setChecked(false);
}

void RichTextEditorToolBar::insertLink()
{
    AddLinkDialog linkDialog(m_editor, this);
    linkDialog.exec();
    m_editor->setFocus();
}

void RichTextEditorToolBar::updateActions()
{
    if (m_editor == 0) {
        setEnabled(false);
        return;
    }

    const Qt::Alignment alignment = m_editor->alignment();
    const QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat char_format = cursor.charFormat();
    const QTextCharFormat::VerticalAlignment valign =
        char_format.verticalAlignment();
    const bool superScript = valign == QTextCharFormat::AlignSuperScript;
    const bool subScript = valign == QTextCharFormat::AlignSubScript;

    if (alignment & Qt::AlignLeft) {
        m_align_left_action->setChecked(true);
    } else if (alignment & Qt::AlignRight) {
        m_align_right_action->setChecked(true);
    } else if (alignment & Qt::AlignHCenter) {
        m_align_center_action->setChecked(true);
    } else {
        m_align_justify_action->setChecked(true);
    }

    m_bold_action->setChecked(char_format.fontWeight() == QFont::Bold);
    m_italic_action->setChecked(char_format.fontItalic());
    m_underline_action->setChecked(char_format.fontUnderline());
    m_valign_sup_action->setChecked(superScript);
    m_valign_sub_action->setChecked(subScript);

    int size = (int) char_format.fontPointSize();
    if (size == 0) // workaround for a bug in QTextEdit
        size = (int) m_editor->document()->defaultFont().pointSize();
    int idx = m_font_size_input->findText(QString::number(size));
    if (idx != -1)
        m_font_size_input->setCurrentIndex(idx);

    /* Font color combo box not used
    const QString color = m_color_map.value(m_editor->textColor());
    idx = m_color_input->findText(color);
    m_color_input->setCurrentIndex(idx);
    */

    m_color_button->setColor(m_editor->textColor());
}

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
                this, SIGNAL(textChanged()));
    connect(this, SIGNAL(cursorPositionChanged()),
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
    if (Qt::mightBeRichText(text))
        setHtml(text);
    else
        setPlainText(text);
}

void RichTextEditor::setDefaultFont(const QFont &font)
{
    document()->setDefaultFont(font);
    if (font.pointSize() > 0)
        setFontPointSize(font.pointSize());
    else
        setFontPointSize(QFontInfo(font).pointSize());
    emit textChanged();
}

static bool compareFontSizes(const QFont &font1, const QFont &font2)
{
    int ps1 = font1.pointSize();
    if (ps1 == -1 && font1.pointSizeF() > 0)
        ps1 = (int) font1.pointSizeF();

    int ps2 = font2.pointSize();
    if (ps2 == -1 && font2.pointSizeF() > 0)
        ps2 = (int) font2.pointSizeF();

    if (ps1 != -1 || ps2 != -1)
        return ps1 == ps2;

    return font1.pixelSize() == font2.pixelSize();
}

static inline bool compareFonts(const QFont &font1, const QFont &font2)
{
    return font1.family() == font2.family()
            && compareFontSizes(font1, font2)
            && font1.bold() == font2.bold()
            && font1.italic() == font2.italic()
            && font1.overline() == font2.overline()
            && font1.underline() == font2.underline()
            && font1.strikeOut() == font2.strikeOut();
}

Qt::TextFormat RichTextEditor::detectFormat() const
{
    Qt::TextFormat result = Qt::PlainText;

    const QFont default_font = document()->defaultFont();
    QTextCursor cursor(document()->begin());
    cursor.movePosition(QTextCursor::End);
    while (!cursor.atStart()) {
        QFont font = cursor.charFormat().font();
        if (!compareFonts(font, default_font)) {
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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_editor = new RichTextEditor();
    m_text_edit = new HtmlTextEdit;
    m_text_edit->setAcceptRichText(false);
    new HtmlHighlighter(m_text_edit);

    // The toolbar needs to be created after the RichTextEditor
    QToolBar *tool_bar = m_editor->createToolBar();
    tool_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QWidget *rich_edit = new QWidget;
    QVBoxLayout *rich_edit_layout = new QVBoxLayout(rich_edit);
    rich_edit_layout->addWidget(tool_bar);
    rich_edit_layout->addWidget(m_editor);

    QWidget *plain_edit = new QWidget;
    QVBoxLayout *plain_edit_layout = new QVBoxLayout(plain_edit);
    plain_edit_layout->addWidget(m_text_edit);

    QTabWidget *tab_widget = new QTabWidget(this);
    tab_widget->setTabPosition(QTabWidget::South);
    tab_widget->addTab(rich_edit, tr("Rich Text"));
    tab_widget->addTab(plain_edit, tr("Source"));
    connect(tab_widget, SIGNAL(currentChanged(int)),
                        SLOT(tabIndexChanged(int)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal,
            this);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setText(tr("&OK"));
    ok_button->setDefault(true);
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tab_widget);
    layout->addWidget(buttonBox);

    m_editor->setFocus();
}

RichTextEditor *RichTextEditorDialog::editor()
{
    return m_editor;
}

void RichTextEditorDialog::tabIndexChanged(int newIndex)
{
    // Remember the cursor position, since it is invalidated by setPlainText
    QTextEdit *new_edit = (newIndex == 1) ? m_text_edit : m_editor;
    const int position = new_edit->textCursor().position();

    if (newIndex == 1)
        m_text_edit->setPlainText(m_editor->text(Qt::RichText));
    else
        m_editor->setHtml(m_text_edit->toPlainText());

    QTextCursor cursor = new_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (cursor.position() > position) {
        cursor.setPosition(position);
    }
    new_edit->setTextCursor(cursor);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

#include "richtexteditor.moc"


