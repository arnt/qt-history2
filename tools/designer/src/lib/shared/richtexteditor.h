#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QtGui/QTextEdit>

class RichTextEditorToolBar;

class RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    RichTextEditor(QWidget *parent = 0);
    void setFormat(Qt::TextFormat format);
    void setDefaultFont(const QFont &font);

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);

signals:
    void textChanged(const QString &html);

protected:
    virtual void moveEvent(QMoveEvent *e);

private slots:
    void contentsChanged();

private:
    Qt::TextFormat detectFormat() const;

    RichTextEditorToolBar *m_tool_bar;
    Qt::TextFormat m_format;
};

#endif // RITCHTEXTEDITOR_H
