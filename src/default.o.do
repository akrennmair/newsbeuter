redo-ifchange ../_conf.sh
. ../_conf.sh

# TODO: moves this to an own view.o.do file as soon as I find a way to make redo can handle this correctly.
STFLHDRS=`redo-subst .stfl .h ../stfl/*.stfl`
redo-ifchange $STFLHDRS ../xlicense.h

$CXX $CXXFLAGS -I../include -I../stfl -I../filter -I.. -I../rss -o $3 -c $1.cpp
