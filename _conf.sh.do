echo "CXX=c++"
echo "CXXFLAGS=\"-Wall -Wextra -ggdb -DLOCALEDIR=\\\"$localedir\\\"\""
echo "LDFLAGS=-L."

FAILSTATUS=""
CXXFLAGS=""
LDFLAGS=""

check_pkg() {
	pkgname=$1
	pkgconfig_args=$2
	echo -n "Checking for package ${pkgname}... " >&2
	if pkg-config --silence-errors "${pkgname}" ; then
		echo "found" >&2
		CXXFLAGS="$CXXFLAGS `pkg-config --cflags $pkgconfig_args ${pkgname}`"
		LDFLAGS="$LDFLAGS `pkg-config --libs $pkgconfig_args ${pkgname}`"
	else
		echo "not found" >&2
		return 1
	fi
	return 0
}

check_custom() {
	pkgname=$1
	customconfig=$2
	echo -n "Checking for package ${pkgname} using ${customconfig}... " >&2
	if ${customconfig} --cflags > /dev/null 2>&1 ; then
		echo "found" >&2
		CXXFLAGS="$CXXFLAGS `${customconfig} --cflags $pkgconfig_args ${pkgname}`"
		LDFLAGS="$LDFLAGS `${customconfig} --libs $pkgconfig_args ${pkgname}`"
	else
		echo "not found" >&2
		return 1
	fi
	return 0
}

fail() {
	pkgname=$1
	rm -f config.mk
	dlurl=`grep -i "$pkgname" README | awk '{ print $NF }'`
	echo "" >&2
	echo "You need package ${pkgname} in order to compile this program." >&2
	echo "Please make sure it is installed." >&2
	echo "" >&2
	echo "You can download ${pkgname} from here: ${dlurl}" >&2
	FAILSTATUS="1"
}

fail_custom() {
	err=$1
	echo "" >&2
	echo "ERROR: ${err}" >&2
	FAILSTATUS="1"
}

all_aboard_the_fail_boat() {
	if [ "x$FAILSTATUS" != "x" ] ; then
		rm -f config.mk
		echo "" >&2
		echo "One or more dependencies couldn\'t be found. Please install" >&2
		echo "these packages and retry compilation." >&2
		exit 1
	fi
}

check_pkg "sqlite3" || fail "sqlite3"
check_pkg "libcurl" || check_custom "libcurl" "curl-config" || fail "libcurl"
check_pkg "libxml-2.0" || check_custom "libxml2" "xml2-config" || fail "libxml2"
check_pkg "stfl" "--static" || fail "stfl"
all_aboard_the_fail_boat

echo "CXXFLAGS=\"\$CXXFLAGS $CXXFLAGS\""
echo "LDFLAGS=\"\$LDFLAGS $LDFLAGS\""
