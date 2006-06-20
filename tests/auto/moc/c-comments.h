#include <qobject.h>

/* test support for multi-line comments in preprocessor statements */

#if 0 /* comment starts here
       ends here */ || defined(Q_MOC_RUN) || 1

class IfdefedClass : public QObject
{
    Q_OBJECT
public:
    inline IfdefedClass() {}
};

#endif
