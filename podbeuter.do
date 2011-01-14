redo-ifchange _conf.sh
. ./_conf.sh

PODBEUTER_LIBS="-lbeuter -lpthread"
PODBEUTER_OBJS=`cat podbeuter.deps | sed 's/.cpp/.o/g'`
LIBS="libbeuter.a"

redo-ifchange $MODFILES $PODBEUTER_OBJS $LIBS
$CXX $CXXFLAGS -Iinclude -Istfl -Ifilter -I. -Irss -o $3 $PODBEUTER_OBJS $PODBEUTER_LIBS $LDFLAGS
