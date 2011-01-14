redo-ifchange _conf.sh
. ./_conf.sh

$CXX $CXXFLAGS -Iinclude -Istfl -Ifilter -I. -Irss -o $3 -c $1.cpp
