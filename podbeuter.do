redo-ifchange _conf.sh
. ./_conf.sh

PODBEUTER_LIBS="-lbeuter -lpthread"
PODBEUTER_SRCS=`cat podbeuter.deps`
PODBEUTER_OBJS=`redo-subst .cpp .o $PODBEUTER_SRCS`
LIBS="libbeuter.a"

redo-ifchange $MODFILES $PODBEUTER_OBJS $LIBS
$CXX $CXXFLAGS -Iinclude -Istfl -Ifilter -I. -Irss -o $3 $PODBEUTER_OBJS $PODBEUTER_LIBS $LDFLAGS
