#include "qwsgc_qws.h"



QWSGC::QWSGC(const QPaintDevice *){
	qDebug("QWSGC::QWSGC");
}
QWSGC::~QWSGC(){
	qDebug("QWSGC::~QWSGC");
}

bool QWSGC::begin(const QPaintDevice *pdev, QPainterState *state, bool begin = FALSE){
	qDebug("QWSGC::begin");
}
bool QWSGC::end(){
	qDebug("QWSGC::end");
}

void QWSGC::updatePen(QPainterState *ps){
	qDebug("QWSGC::updatePen");
}
void QWSGC::updateBrush(QPainterState *ps){
	qDebug("QWSGC::updateBrush");
}
void QWSGC::updateFont(QPainterState *ps){
    qDebug("QWSGC::updateFont");
}
void QWSGC::updateRasterOp(QPainterState *ps){
    qDebug("QWSGC::updateRasterOp");
}
void QWSGC::updateBackground(QPainterState *ps){
    qDebug("QWSGC::updateBackground");
}
void QWSGC::updateXForm(QPainterState *ps){
    qDebug("QWSGC::updateXForm");
}
void QWSGC::updateClipRegion(QPainterState *ps){
    qDebug("QWSGC::updateClipRegion");
}

void QWSGC::setRasterOp(RasterOp r){
    qDebug("QWSGC::setRasterOp");
}

void QWSGC::drawLine(int x1, int y1, int x2, int y2){
    qDebug("QWSGC::drawLine");
}
void QWSGC::drawRect(int x1, int y1, int w, int h){
    qDebug("QWSGC::drawRect");
}
void QWSGC::drawPoint(int x, int y){
    qDebug("QWSGC::drawPoint");
}
void QWSGC::drawPoints(const QPointArray &pa, int index = 0, int npoints = -1){
    qDebug("QWSGC::drawPoints");
}
void QWSGC::drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor){
    qDebug("QWSGC::drawWinFocusRect");
}
void QWSGC::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd){
    qDebug("QWSGC::drawRoundRect");
}
void QWSGC::drawEllipse(int x, int y, int w, int h){
    qDebug("QWSGC::drawEllipse");
}
void QWSGC::drawArc(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawArc");
}
void QWSGC::drawPie(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawPie");
}
void QWSGC::drawChord(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawChord");
}
void QWSGC::drawLineSegments(const QPointArray &, int index = 0, int nlines = -1){
    qDebug("QWSGC::drawLineSegments");
}
void QWSGC::drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1){
    qDebug("QWSGC::drawPolyline");
}
void QWSGC::drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints =- 1){
    qDebug("QWSGC::drawPolygon");
}
void QWSGC::drawConvexPolygon(const QPointArray &pa, int index = 0, int npoints = -1){
    qDebug("QWSGC::drawConvexPolygon");
}
#ifndef QT_NO_BEZIER
void QWSGC::drawCubicBezier(const QPointArray &pa, int index = 0){
    qDebug("QWSGC::drawCubicBezier");
}
#endif

void QWSGC::drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh){
    qDebug("QWSGC::drawPixmap");
}
void QWSGC::drawTextItem(int x, int y, const QTextItem &ti, int textflags){
    qDebug("QWSGC::drawTextItem");
}

Qt::HANDLE QWSGC::handle() const{
    qDebug("QWSGC::handle");
    return 0;
}


    void QWSGC::initialize(){
	qDebug("QWSGC::initialize");
}
    void QWSGC::cleanup(){
	qDebug("QWSGC::cleanup");
}

