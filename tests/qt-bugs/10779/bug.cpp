//-----------------------------------------------
#include <qapplication.h>
#include <qmainwindow.h>
#include <qscrollview.h>
#include <stdio.h>

class cDrawWindow : public QScrollView {
protected:
  virtual void viewportResizeEvent(QResizeEvent *e);
  virtual void viewportPaintEvent(QPaintEvent *pe);
  void SetVisibleArea(QSize size);
public:
  cDrawWindow(QWidget *parent);
  };

void cDrawWindow::viewportResizeEvent(QResizeEvent *e)
{
  viewport()->setUpdatesEnabled(false);
  SetVisibleArea(e->size());
  QScrollView::viewportResizeEvent(e);
  viewport()->setUpdatesEnabled(true);
  int w = QMIN(e->oldSize().width(), e->size().width());
  int h = QMIN(e->oldSize().height(), e->size().height());
  updateContents(contentsX(), contentsY(), w, h);
} 

void cDrawWindow::SetVisibleArea(QSize size)
{
  int w = viewport()->width() * 3;
  int h = viewport()->height() * 3;
  resizeContents(w, h);
  setContentsPos(w / 3, h / 3);
}

cDrawWindow::cDrawWindow(QWidget *parent)
:QScrollView(parent, NULL, WNorthWestGravity | WPaintClever)
{
  resizeContents(1000, 1000);
}

void cDrawWindow::viewportPaintEvent(QPaintEvent *pe)
{
  printf("viewportPaintEvent\n");
  QMemArray<QRect> rects = pe->region().rects();
  QPainter p(viewport());
  for (int i = rects.size(); i--; ) {
      printf("rect %5d %5d %5d %5d\n", rects[i].x(), rects[i].y(), rects[i].width(), rects[i].height());
      static char *color[] = { "red", "green", "blue", "cyan", "magenta" };
      p.fillRect(rects[i], QBrush(QColor(color[i % 5])));
      p.drawRoundRect(rects[i], 30, 30);
      }
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QMainWindow mw;
  a.setMainWidget(&mw);
  cDrawWindow dw(&mw);
  mw.setCentralWidget(&dw);
  mw.show();
  a.exec();
  return 0;
}


