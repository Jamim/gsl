AC_DEFUN([AX_CBLAS],[

  ext_cblas=no
  ext_cblas_libs="-lcblas"
  ext_cblas_cflags=""

  AC_ARG_WITH(cblas-external,
	[AS_HELP_STRING([--with-cblas-external], 
			[Use external CBLAS library (default is no)])],
	[with_ext_cblas=$withval],
	[with_ext_cblas=no])

  case $with_ext_cblas in
	no) ext_cblas=no ;;
	yes) ext_cblas=yes ;;
	-* | */* | *.a | *.so | *.so.* | *.o) 
	   ext_cblas=yes
	   ext_cblas_libs="$with_cblas" ;;
	*) ext_cblas=yes
	   ext_cblas_libs="-l$with_cblas" ;;
  esac

  AC_ARG_WITH(cblas-external-libs,
	[AS_HELP_STRING([--with-cblas-external-libs=<libs>],
			[External cblas libraries to link with (default is "$ext_cblas_libs")])],
	[ext_cblas_libs=$withval],
	[])

  AC_ARG_WITH(cblas-external-cflags,
	[AS_HELP_STRING([--with-cblas-external-cflags=<flags>],
			[Pre-processing flags to compile with external cblas ("-I<dir>")])],
	[ext_cblas_cflags=$withval],
	[])

  if test x$ext_cblas != xno; then
	if test "x$CBLAS_LIBS" = x; then
	   CBLAS_LIBS="$ext_cblas_libs"
     	fi
     	if test "x$CBLAS_CFLAGS" = x; then
       	   CBLAS_CFLAGS="$ext_cblas_cflags"
   	fi

   	CFLAGS_sav="$CFLAGS"
   	CFLAGS="$CFLAGS $CBLAS_CFLAGS"
   	AC_CHECK_HEADER(cblas.h, ,
		[AC_MSG_ERROR([
	   	*** Header file cblas.h not found.
	   	*** If you installed cblas header in a non standard place,
	   	*** specify its install prefix using the following option
	   	***  --with-cblas-external-cflags="-I<include_dir>"])
	 	])
   	CFLAGS="$CFLAGS_sav"

   	LIBS_sav="$LIBS"
   	LIBS="$LIBS $CBLAS_LIBS -lm"
   	AC_MSG_CHECKING([for cblas_sgemm in $CBLAS_LIBS])
   	AC_TRY_LINK_FUNC(cblas_sgemm, [ext_cblas=yes],
    		[AC_MSG_ERROR([
	    	*** Linking with cblas with $LIBS failed.
       	    	*** If you installed cblas library in a non standard place,
   	    	*** specify its install prefix using the following option
	    	***  --with-cblas-external-libs="-L<lib_dir> -l<lib>"])
	 	])
   	AC_MSG_RESULT($ext_cblas)
   	LIBS="$LIBS_sav"
	AC_SUBST([CBLAS_CFLAGS])
	AC_SUBST([CBLAS_LIBS])
 fi
])
