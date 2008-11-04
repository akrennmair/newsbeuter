AC_DEFUN([AC_PROG_COCOCPP],[
AC_CHECK_PROGS(cococpp,[cococpp],no)
if test $cococpp = "no" ;
then
        ifelse($#,0,[AC_MSG_ERROR([Unable to find the cococpp application])],
        $1)
fi
])
