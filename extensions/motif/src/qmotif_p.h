#ifndef QMOTIF_P_H
#define QMOTIF_P_H

class QMotifPrivate
{
public:
    QMotifPrivate();

    void hookMeUp();
    void unhook();

    XtAppContext appContext;
    bool ownContext;
    QArray<XtEventDispatchProc> dispatchers;
    QMotifEventLoop *eventloop;
    QWidgetIntDict mapper;
};

#endif // QMOTIF_P_H
