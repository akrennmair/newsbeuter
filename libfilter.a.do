redo-ifchange _conf.sh
. ./_conf.sh

OBJS="filter/Scanner.o filter/Parser.o filter/FilterParser.o"

redo-ifchange $OBJS

rm -f $1
ar qc $3 $OBJS
ranlib $3
