#if defined(QT_THREAD_SUPPORT)

#if defined(Q_WS_WIN)

/*
  QCriticalSection
*/

class QCriticalSectionPrivate;

class QCriticalSection
{
public:
    QCriticalSection();
    ~QCriticalSection();
    void enter();
    void leave();

private:
    QCriticalSectionPrivate *d;
};

#endif

#endif
