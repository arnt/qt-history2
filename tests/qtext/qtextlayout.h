#ifndef _Q_PARAGRAPH_H_
#define _Q_PARAGRAPH_H_

#include <qstring.h>
#include <qpoint.h>
#include <qrect.h>
#include <qlist.h>
#include <qapplication.h>

class QPainter;

class QBidiContext {
public:
    QBidiContext(unsigned char level, QChar::Direction embedding, QBidiContext *parent = 0, bool override = false);
    ~QBidiContext();

    void ref() const;
    void deref() const;

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    QBidiContext *parent;

    // refcounting....
    mutable int count;
};

struct QBidiStatus {
    QBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor 		: 5;
    QChar::Direction lastStrong 	: 5;
    QChar::Direction last		: 5;
};

class QTextLine {
public:
    QTextLine(const QString &text, int from, int length, QTextLine *previous);
    virtual ~QTextLine();

    QTextLine *previousLine() const { return prev; }
    QTextLine *nextLine() const { return next; }
    void setPreviousLine(QTextLine *l) { prev = l; }
    void setNextLine(QTextLine *l) { next = l; }

    QBidiContext *startEmbedding() { return startEmbed; }
    QBidiContext *endEmbedding() { return endEmbed; }
    int from() { return start; }
    int length() { return len; }

    virtual void paint(QPainter *p, int x, int y);

    void setPosition(int _x, int _y);
    int width() const { return w; }
    int height() const { return h; }
    int x() const { return xPos; }
    int y() const { return yPos; }

    void setBoundingRect(const QRect &r);
    QRect boundingRect();

private:
    bool hasComplexText();
    void bidiReorderLine();

    bool complexText : 1;

    QBidiContext *startEmbed;
    QBidiContext *endEmbed;
    QBidiStatus bidiStatus;
    int start;
    short len;
    int w;
    short h;
    int xPos;
    int yPos;
    QString text;
    QString reorderedText;

    QTextLine *prev;
    QTextLine *next;
};

class QTextArea;

class QParagraph {
public:
    QParagraph(const QString &, QTextArea *, QParagraph *last = 0);
    virtual ~QParagraph();

    QRect boundingRect() const { return bRect; }
    QPoint nextLine() const;

    void paint(QPainter *p, int x, int y);

protected:
    virtual void layout();

    int findLineBreak(int pos);
    void addLine(int pos, int length);

private:
    QTextArea *area;
    QTextLine *first;
    QTextLine *last;

    QString text;

    int xPos;
    int yPos;
    QRect bRect;
};


class QTextArea {
public:
    QTextArea();
    QTextArea(int width);
    virtual ~QTextArea();

    virtual int lineWidth(int x, int y, int h = 0) const;
    virtual QRect lineRect(int x, int y, int h) const;

    void appendParagraph(const QString &);
    void insertParagraph(const QString &, int pos);
    void removeParagraph(int pos);

    virtual QParagraph *createParagraph(const QString &text, QParagraph *before);

    void paint(QPainter *p, int x, int y);

private:
    int width;
    QList<QParagraph> paragraphs;
};


#endif
