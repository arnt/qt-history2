TARGET   = QtPatternist
TEMPLATE = lib

# This ensures we get a framework on OS X, a library bundle.
CONFIG   += lib_bundle

QT       = core xml
DESTDIR  = $(QTDIR)/lib
include(common.pri)

include(acceltree/acceltree.pri)
include(api/api.pri)
include(data/data.pri)
include(environment/environment.pri)
include(expr/expr.pri)
include(functions/functions.pri)
include(iterators/iterators.pri)
include(janitors/janitors.pri)
include(parser/parser.pri)
include(projection/projection.pri)
include(type/type.pri)
include(utils/utils.pri)
include(qobjectmodel/qobjectmodel.pri)
