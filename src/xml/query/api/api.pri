HEADERS += api/qabstractxmlpullprovider.h           \
           api/PullBridge_p.h                       \
           api/PushBridge_p.h                       \
           api/qabstractmessagehandler.h            \
           api/qabstracturiresolver.h               \
           api/qabstractxmlpushcallback.h           \
           api/qxmlitemiterator.h                   \
           api/qxmlitemiterator_p.h                 \
           api/qserializationsettings.h             \
           api/qsourcelocation.h                    \
           api/qxmlname.h                           \
           api/qxmlquery_p.h                        \
           api/VariableLoader_p.h                   \
           $$PWD/../../cli/ColoringMessageHandler.h \
           api/qxmlquery.h

SOURCES += api/qabstractxmlpullprovider.cpp             \
           api/PullBridge_p.cpp                         \
           api/PushBridge_p.cpp                         \
           api/VariableLoader_p.cpp                     \
           api/qabstractmessagehandler.cpp              \
           api/qabstractxmlpushcallback.cpp             \
           api/qxmlitemiterator.cpp                     \
           api/qserializationsettings.cpp               \
           api/qsourcelocation.cpp                      \
           api/qabstracturiresolver.cpp                 \
           api/qxmlname.cpp                             \
           $$PWD/../../cli/ColoringMessageHandler.cpp   \
           $$PWD/../../cli/ColorOutput.cpp              \
           api/qxmlquery.cpp

INCLUDEPATH += $$PWD/../../cli/
