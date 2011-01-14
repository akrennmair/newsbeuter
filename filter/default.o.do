redo-ifchange ../_conf.sh
. ../_conf.sh

$CXX $CXXFLAGS -I../include -I../stfl -I../filter -I.. -I../rss -o $3 -c $1.cpp
