#include <QApplication>
#include <QCalendarWidget>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCalendarWidget calendar;
    calendar.setSelectedDate(calendar.selectedDate().addDays(3));
    calendar.setGridVisible(true);
    calendar.show();

    return app.exec();
}
