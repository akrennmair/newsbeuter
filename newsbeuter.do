redo-ifchange _conf.sh
. ./_conf.sh

NEWSBEUTER_LIBS="-lbeuter -lfilter -lpthread -lrsspp"
MOFILES=`ls po/*.po | sed 's/.po/.mo/g'`
NEWSBEUTER_OBJS=`cat newsbeuter.deps | sed 's/.cpp/.o/g'`
LIBS="libbeuter.a librsspp.a libfilter.a"

redo-ifchange $MOFILES $NEWSBEUTER_OBJS $LIBS
$CXX $CXXFLAGS -Iinclude -Istfl -Ifilter -I. -Irss -o $3 $NEWSBEUTER_OBJS $NEWSBEUTER_LIBS $LDFLAGS
