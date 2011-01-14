redo-ifchange _conf.sh
. ./_conf.sh

OBJS=`cat libbeuter.deps | sed 's/.cpp/.o/g'`

redo-ifchange $OBJS

rm -f $1
ar qc $3 $OBJS
ranlib $3
