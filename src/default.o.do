redo-ifchange ../_conf.sh
. ../_conf.sh

# TODO: moves this to an own view.o.do file as soon as redo can handle this correctly.
STFLHDRS=`ls ../stfl/*.stfl | sed 's/.stfl$/.h/g'`
redo-ifchange $STFLHDRS ../xlicense.h

$CXX $CXXFLAGS -I../include -I../stfl -I../filter -I.. -I../rss -o $3 -c $1.cpp
