redo-ifchange _conf.sh
. ./_conf.sh

SRCS=`cat libbeuter.deps`
OBJS=`redo-subst .cpp .o $SRCS`

redo-ifchange $OBJS

rm -f $1
ar qc $3 $OBJS
ranlib $3
