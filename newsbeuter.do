redo-ifchange _conf.sh
. ./_conf.sh

NEWSBEUTER_LIBS="-lbeuter -lfilter -lpthread -lrsspp"
MOFILES=`redo-subst .po .mo po/*.po`
NEWSBEUTER_SRCS=`cat newsbeuter.deps`
NEWSBEUTER_OBJS=`redo-subst .cpp .o $NEWSBEUTER_SRCS`
LIBS="libbeuter.a librsspp.a libfilter.a"

redo-ifchange $MOFILES $NEWSBEUTER_OBJS $LIBS
$CXX $CXXFLAGS -Iinclude -Istfl -Ifilter -I. -Irss -o $3 $NEWSBEUTER_OBJS $NEWSBEUTER_LIBS $LDFLAGS
