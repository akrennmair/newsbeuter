redo-ifchange _conf.sh
. ./_conf.sh

RSSPPLIB_OBJS=`ls rss/*.cpp | sed 's/.cpp/.o/g'`

redo-ifchange $RSSPPLIB_OBJS

rm -f $1
ar qc $3 $RSSPPLIB_OBJS
ranlib $3
