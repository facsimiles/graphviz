# Process this file with autoconf to produce a configure script
AC_PREREQ(2.69)

dnl ===========================================================================
dnl Set Graphviz version information

# set:
#	graphviz_version_major
#	graphviz_version_minor
#	graphviz_version_micro
#	graphviz_version_date
#	graphviz_change_date
#	graphviz_author_name
#	graphviz_author_email
m4_include(./version.m4)

AC_INIT([graphviz],[graphviz_version_major().graphviz_version_minor().graphviz_version_micro()],
  [https://gitlab.com/graphviz/graphviz/-/issues])

GRAPHVIZ_VERSION_MAJOR=graphviz_version_major()
GRAPHVIZ_VERSION_MINOR=graphviz_version_minor()
GRAPHVIZ_VERSION_MICRO=graphviz_version_micro()

# NB: date/time of last commit - or "0"
GRAPHVIZ_VERSION_DATE=graphviz_version_date()

GRAPHVIZ_AUTHOR_NAME=graphviz_author_name()
GRAPHVIZ_AUTHOR_EMAIL=graphviz_author_email()
GRAPHVIZ_CHANGE_DATE=graphviz_change_date()

AC_SUBST([GRAPHVIZ_VERSION_MAJOR])
AC_SUBST([GRAPHVIZ_VERSION_MINOR])
AC_SUBST([GRAPHVIZ_VERSION_MICRO])
AC_SUBST([GRAPHVIZ_VERSION_DATE])
AC_SUBST([GRAPHVIZ_AUTHOR_NAME])
AC_SUBST([GRAPHVIZ_AUTHOR_EMAIL])
AC_SUBST([GRAPHVIZ_CHANGE_DATE])

dnl ===========================================================================

# libtool shared library version for plugins

# Increment if the interface has additions, changes, removals.
GVPLUGIN_CURRENT=6

# Increment any time the source changes; set to
# 0 if you increment CURRENT
GVPLUGIN_REVISION=0

# Increment if any interfaces have been added; set to 0
# if any interfaces have been removed. removal has
# precedence over adding, so set to 0 if both happened.
GVPLUGIN_AGE=0

GVPLUGIN_CONFIG_FILE="config$GVPLUGIN_CURRENT"
AC_DEFINE_UNQUOTED(GVPLUGIN_CONFIG_FILE,"$GVPLUGIN_CONFIG_FILE",[Filename for plugin configuration file.])
AC_SUBST([GVPLUGIN_CONFIG_FILE])

GVPLUGIN_VERSION_INFO="$GVPLUGIN_CURRENT:$GVPLUGIN_REVISION:$GVPLUGIN_AGE"
AC_SUBST([GVPLUGIN_VERSION_INFO])

dnl ===========================================================================


AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

dnl -----------------------------------
dnl Checks for system type

AC_CANONICAL_HOST

# This nonsense drives me crazy!!
case "${host_os}" in
  *irix* | *nonstopux* )
    GVPLUGIN_VERSION=`expr $GVPLUGIN_CURRENT + 1`
    ;;
  * )
    GVPLUGIN_VERSION=$GVPLUGIN_CURRENT
    ;;
esac
AC_DEFINE_UNQUOTED(GVPLUGIN_VERSION,$GVPLUGIN_VERSION,[Compatibility version number for plugins.])
AC_SUBST([GVPLUGIN_VERSION])

AC_ARG_VAR(LIBPOSTFIX, [subscript to lib, e.g. "/64" for "/usr/lib/64" on Solaris])
AC_ARG_VAR(INTGOSIZE, [integer size for go, 32 or 64])
AC_ARG_VAR(RUBY_VER, [version number of the ruby package])

INTGOSIZE=32
if test -z "${LIBPOSTFIX++}"; then
  case "${host_os}" in
    *linux* )
      case "${host_cpu}" in
        aarch64 | powerpc64 | powerpc64le | s390x | x86_64 | sparc64 | mips64* | riscv64 | e2k )
          LIBPOSTFIX="64"
          INTGOSIZE=64
          ;;
      esac
      ;;
    *solaris* )
      case "${host_cpu}" in
        x86_64 | sparc64 )
          LIBPOSTFIX="/64"
          INTGOSIZE=64
          ;;
      esac
      ;;
  esac
fi

UWIN=no
CYGWIN=no
MINGW32=no
case "${host_os}" in
  *uwin* )
	UWIN=yes
	BROWSER="xdg-open"
	JSHEXT=so
	SHARED_LIBRARY_SUFFIX=.so
	;;
  *cygwin* )
	CYGWIN=yes
	BROWSER="xdg-open"
	JSHEXT=so
	SHARED_LIBRARY_SUFFIX=.dll
	;;
  *mingw32* )
	MINGW32=yes
	BROWSER="xdg-open"
	JSHEXT=so
	SHARED_LIBRARY_SUFFIX=.so
	;;
  *darwin* )
	DARWIN=yes
	AC_DEFINE_UNQUOTED(DARWIN,1,[Define for any Darwin-based OS.])
	BROWSER="open"
	JSHEXT=jnilib
	SHARED_LIBRARY_SUFFIX=.dylib
	AC_DEFINE_UNQUOTED(DARWIN_DYLIB,"$DARWIN_DYLIB",[Define for Darwin-style shared library names.])
	;;
  hpux* )
	BROWSER="xdg-open"
	JSHEXT=so
	SHARED_LIBRARY_SUFFIX=.sl
	;;
  * )
	BROWSER="xdg-open"
	JSHEXT=so
	SHARED_LIBRARY_SUFFIX=.so
	;;
esac
AC_SUBST([CYGWIN])
AC_SUBST([MINGW32])
AC_SUBST([DARWIN])
JNI_EXTRA_LDFLAGS="-shrext .$JSHEXT"
AC_SUBST([JNI_EXTRA_LDFLAGS])
AC_SUBST([JSHEXT])
AC_SUBST([SHARED_LIBRARY_SUFFIX])
AM_CONDITIONAL(WITH_WIN32, [test "$UWIN" = "yes" -o "$CYGWIN" = "yes" -o "$MINGW32" = "yes"])
AM_CONDITIONAL(WITH_CYGWIN, [test "$CYGWIN" = "yes"])
AM_CONDITIONAL(WITH_MINGW, [test "$MINGW32" = "yes"])

DEFAULT_DPI=96
AC_DEFINE_UNQUOTED(DEFAULT_DPI,$DEFAULT_DPI,Default DPI.)
AC_DEFINE_UNQUOTED(BROWSER,"$BROWSER",[Command to open a browser on a URL])

AM_INIT_AUTOMAKE([no-define subdir-objects tar-ustar])

# Suppress verbose compile lines
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])

# put BUILDDATE in its own file so that only targets that use it get rebuilt
echo "#define BUILDDATE \"$GRAPHVIZ_VERSION_DATE\"" > builddate.h

AC_PREFIX_DEFAULT([/usr/local])
if test "${prefix}" = "NONE"; then
	prefix=${ac_default_prefix}
	AC_SUBST([prefix])
fi

dnl -----------------------------------
# Static/Shared binaries

AC_ARG_ENABLE(static,
        [AS_HELP_STRING([--enable-static],[build static executable])])
if test "$enable_static" = "yes"; then
	AC_ENABLE_STATIC
        use_static="Yes"
else
	AC_DISABLE_STATIC
        use_static="No (disabled by default)"
fi
AM_CONDITIONAL(ENABLE_STATIC, [test "$enable_static" = "yes"])

AC_ARG_ENABLE(shared,
        [AS_HELP_STRING([--enable-shared],[build shared executable])])
if test "$enable_shared" != "no"; then
	AC_ENABLE_SHARED
        use_shared="Yes"
	case "${host_os}" in
  		*uwin* | *cygwin* | *mingw32* )
			AC_DEFINE_UNQUOTED(GVDLL,1, [Define for DLLs on Windows.])
		;;
	esac

else
        use_shared="No (disabled)"
	AC_DISABLE_SHARED
fi
AM_CONDITIONAL(ENABLE_SHARED, [test "$enable_shared" = "yes"])

dnl -----------------------------------
dnl checks for various programs

AM_PROG_LEX
AC_PROG_YACC
AC_PROG_EGREP
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT

AC_ARG_ENABLE([man-pdfs],
  [AS_HELP_STRING([--enable-man-pdfs], [Generate PDFs of man pages (default: auto)])])

AS_IF([test "$enable_man_pdfs" != "no"], [
  AC_CHECK_PROGS([GROFF], [groff])
  AS_IF([test "$enable_man_pdfs" = "yes" && test -z "$GROFF"], [
    AC_MSG_ERROR([You requested PDF generation from man pages, but 'groff' could not be found])
  ])

  AC_CHECK_PROGS([PS2PDF], [ps2pdf pstopdf])
  AS_IF([test "$enable_man_pdfs" = "yes" && test -z "$PS2PDF"], [
    AC_MSG_ERROR([You requested PDF generation from man pages, but neither 'ps2pdf' nor 'pstopdf' could be found])
  ])
])
AM_CONDITIONAL([ENABLE_MAN_PDFS], [test "$enable_man_pdfs" != "no" && test -n "$GROFF" && test -n "$PS2PDF"])

# allow disabling of demos installation
AC_ARG_WITH([demos],
  [AS_HELP_STRING([--with-demos=DIR], [Install demos (default: $(pkgdatadir)/demo)])])

demodir='$(pkgdatadir)/demo'
AS_IF([test "$with_demos" != "yes" && test -n "$with_demos"], [
  dnl user provided --with-demos=/my/demo/dir
  demodir="$with_demos"
])
AM_CONDITIONAL([INSTALL_DEMOS], [test "$with_demos" != "no"])
AC_SUBST([demodir])

PKG_INSTALLDIR
PKG_PROG_PKG_CONFIG

AC_ARG_WITH(tclsh,
  [AS_HELP_STRING([--with-tclsh=PROG],[use a specific tclsh])],
  TCLSH=$withval,)

if test "$TCLSH" = ""; then
  AC_PATH_PROGS(TCLSH,[tclsh8.6 tclsh8.5 tclsh8.4 tclsh8.3 tclsh],[],[/usr/bin])
fi


dnl -----------------------------------
dnl checks for compilers

AC_PROG_CC
m4_version_prereq([2.70],[],[
  AX_CHECK_COMPILE_FLAG([-std=gnu11],[
    CFLAGS+=" -std=gnu11"
  ],[
    AX_CHECK_COMPILE_FLAG([-std=c11],[
      CFLAGS+=" -std=c11"
    ],[
      AX_CHECK_COMPILE_FLAG([-std:c11],[
        CFLAGS+=" -std:c11"
      ],[
        ac_cv_prog_cc_c11="no"
      ])
    ])
  ])
])
if test "$ac_cv_prog_cc_c11" = "no"; then
  AC_MSG_ERROR([C compiler does not support C11])
fi
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_17(noext,mandatory)
AC_PROG_OBJC

AC_C_INLINE

dnl ===========================================================================
dnl Set GCC compiler flags

if [ test "${GCC}" = "yes" ]
then
  # Enable common warnings flags
  CFLAGS="${CFLAGS} -Wall"

  # Enable common extra flags
  CFLAGS="${CFLAGS} -Wextra"

  # Enable specific warning flags not included by -Wall or -Wextra
  CFLAGS="${CFLAGS} -Wmissing-include-dirs -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wconversion -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs"

  # Enable additional C warnings
  save_CFLAGS="${CFLAGS}"
  CFLAGS=""
  AX_CHECK_COMPILE_FLAG([-Wtrampolines],
    [save_CFLAGS="${save_CFLAGS} -Wtrampolines"],[],[-Werror])
  AX_CHECK_COMPILE_FLAG([-Wlogical-op],
    [save_CFLAGS="${save_CFLAGS} -Wlogical-op"],[],[-Werror])
  CFLAGS="${save_CFLAGS}"
fi
# Workaround for native compilers
#  HP  : http://bugs.gnome.org/db/31/3163.html
#  DEC : Enable NaN/Inf

if test "${GCC}" != "yes" ; then
    case "${build}" in
        *-*-hpux* )
	    CFLAGS="${CFLAGS} -Wp,-H30000"
            ;;
        *-dec-osf* )
            CFLAGS="${CFLAGS} -ieee"
            ;;
	*-apple-darwin* )
	    CFLAGS="${CFLAGS} -fno-common"
	    LDFLAGS="${LDFLAGS} -Wl,-headerpad_max_install_names"
	    ;;
	*-sgi-irix* )
	    ;;
    esac
else
    case "${build}" in
    *-dec-osf* )
      CFLAGS="${CFLAGS} -mieee -Wall"
      ;;
    *alpha*-*-linux* )
      CFLAGS="${CFLAGS} -mieee -Wall"
      ;;
    *-apple-darwin* )
      CFLAGS="${CFLAGS} -fno-common -Wall"
      LDFLAGS="${LDFLAGS} -Wl,-headerpad_max_install_names"
      ;;
    *-freebsd* )
      CFLAGS="${CFLAGS} -Wstrict-prototypes -Wpointer-arith -Wall"
      ;;
    * )
      CFLAGS="${CFLAGS} -Wno-unknown-pragmas -Wstrict-prototypes -Wpointer-arith -Wall"
      ;;
    esac
fi

# debug
AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[Compile the debug version (default: disabled)]),
	[enable_debug=$enableval],
	[enable_debug=no])
AM_CONDITIONAL([DEBUG], [test $enable_debug = "yes"])
if test "$enable_debug" = "yes"; then
  dnl add -O0 only if GCC is used
  if test "$GCC" = "yes"; then
    CFLAGS="$CFLAGS -g -O0"
    CXXFLAGS="$CXXFLAGS -g -O0"
  fi
fi

dnl -----------------------------------
dnl Check for various typedefs and provide substitutes if they do not exist.

AC_TYPE_PID_T
AC_TYPE_SSIZE_T

dnl -----------------------------------
dnl Checks for header files

AC_CHECK_HEADERS([sys/time.h sys/select.h sys/mman.h sys/ioctl.h sys/inotify.h])

dnl -----------------------------------
dnl Checks for -lm library

AC_CHECK_LIB(m, main, [MATH_LIBS="-lm"])
AC_SUBST([MATH_LIBS])

# -----------------------------------

# Checks for library functions
AC_CHECK_FUNCS([lrand48 drand48 srand48 setmode setenv memrchr select dl_iterate_phdr inotify_init1])

AC_REPLACE_FUNCS([strcasestr])

dnl -----------------------------------
dnl libtool ltdl on-demand plugin loading

save_LIBS="$LIBS"
LT_INIT([dlopen])
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_INIT

AC_ARG_ENABLE(ltdl,
  [AS_HELP_STRING([--enable-ltdl],[support on-demand plugin loading])])
if test "$enable_ltdl" != "no"; then
  AC_DEFINE(ENABLE_LTDL,1,[Define if you want on-demand plugin loading])
  use_ltdl="Yes"
  # The lt_dladvise_init symbol was added with libtool-2.2
  if test "$with_included_ltdl" != "yes"; then
    save_CFLAGS="$CFLAGS"
    save_LDFLAGS="$LDFLAGS"
    CFLAGS="$CFLAGS $LTDLINCL"
    LDFLAGS="$LDFLAGS $LIBLTDL"
    AC_CHECK_LIB([ltdl], [lt_dladvise_init], [],
	[AC_MSG_ERROR([installed libltdl is too old])])
    LDFLAGS="$save_LDFLAGS"
    CFLAGS="$save_CFLAGS"
  fi
else
  use_ltdl="No (disabled)"
fi
AM_CONDITIONAL(ENABLE_LTDL, [test "$enable_ltdl" != "no"])
AC_SUBST([INCLTDL])
AC_SUBST([LIBLTDL])
LIBS="$save_LIBS"

dnl -----------------------------------
dnl Checks for X header files.

AC_PATH_XTRA
if test "$no_x" = "yes"; then
  use_xlib="No (disabled or unavailable)"
  AC_MSG_WARN([X11 not available.])
  X_SUBDIRS=""
else
  use_xlib="Yes"
  PKG_CHECK_MODULES(XRENDER, [xrender],HAVE_XRENDER="yes" ,HAVE_XRENDER="no")
fi
AM_CONDITIONAL(WITH_X, [test "$use_xlib" = "Yes"])

dnl -----------------------------------
dnl check for SWIG - needed for script-language bindings

AC_ARG_ENABLE(swig,
  [AS_HELP_STRING([--enable-swig=yes],[swig-generated language bindings])],
  [],[enable_swig=yes])

if test "$enable_swig" != "yes"; then
  use_swig="No (disabled)"
else
  AC_CHECK_PROG(SWIG,swig,swig)
  if test "$SWIG" = ""; then
    use_swig="No (swig not available)"
  else
    SWIG_VERSION=`$SWIG -version 2>&1 | $EGREP Version | cut -d ' ' -f 3`
    AC_SUBST([SWIG_VERSION])
    AX_COMPARE_VERSION([$SWIG_VERSION], [lt], [1.3.0], [SWIG=])
    AX_COMPARE_VERSION([$SWIG_VERSION], [lt], [1.3.40], [
      # EL5 has swig-1.3.29 which does not define SWIGJAVA
      SWIGJAVA_CPPFLAGS=-DSWIGJAVA
      AC_SUBST([SWIGJAVA_CPPFLAGS])
    ])
    AX_COMPARE_VERSION([$SWIG_VERSION], [lt], [4.1.0], [
      SWIG_PHP_DATA=gv.php
      SWIG_PHP_DATA_TO_PACKAGE=usr/share/php/gv.php
    ], [
      # SWIG ≥ 4.1.0 no longer generates any PHP code for the bindings
      SWIG_PHP_DATA=
      SWIG_PHP_DATA_TO_PACKAGE=
    ])
    AC_SUBST([SWIG_PHP_DATA])
    AC_SUBST([SWIG_PHP_DATA_TO_PACKAGE])
    if test "$SWIG" = ""; then
      AC_MSG_WARN([The version of swig is too old.])
      use_swig="No (swig too old)"
    else
      use_swig="Yes"
    fi
  fi
fi
AM_CONDITIONAL(WITH_SWIG, [test "$SWIG" != ""])
AM_CONDITIONAL(WITH_SWIG_PHP_DATA,
               [test "$SWIG" != "" -a "$SWIG_PHP_DATA" != ""])

dnl -----------------------------------
dnl INCLUDES and LIBS for C#

AC_ARG_ENABLE(sharp,
  [AS_HELP_STRING([--enable-sharp=yes],[C# language bindings])],
  [],[enable_sharp=yes])

if test "$enable_sharp" != "yes"; then
  use_sharp="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_sharp="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-csharp *- Generate'` = 0; then
      use_sharp="No (swig does not support -csharp option)"
    else
      AC_CHECK_PROG(MCS,mcs,mcs)
      if test "$MCS" = ""; then
        use_sharp="No (mcs not available)"
      else
        use_sharp="Yes"
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_SHARP, [test "$use_sharp" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for D (disabled by default - incomplete)

AC_ARG_ENABLE(d,
  [AS_HELP_STRING([--enable-d=no],[d language bindings])],
  [], [enable_d=no])

if test "$enable_d" != "yes"; then
  use_d="No (disabled by default - incomplete)"
else
  if test "$use_swig" != "Yes"; then
    use_d="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-d* *- Generate'` = 0 ; then
      use_d="No (swig does not support -d option)"
    else
      use_d="Yes"
    fi
  fi
fi
AM_CONDITIONAL(WITH_D, [test "$use_d" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GO

AC_ARG_ENABLE(go,
  [AS_HELP_STRING([--enable-go=yes],[go language bindings])],
  [], [enable_go=yes])

if test "$enable_go" != "yes"; then
  use_go="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_go="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-go* *- Generate'` = 0 ; then
      use_go="No (swig does not support -go option)"
    else
      AC_CHECK_PROG(GO,go,go)
      if test "$GO" = ""; then
        use_go="No (GO compiler not available)"
      else
        GO_INCLUDES=
        GO_LIBS=
	if test "$GO" != ""; then
	  use_go="Yes"
          AC_SUBST([GO_INCLUDES])
          AC_SUBST([GO_LIBS])
          AC_SUBST([INTGOSIZE])
        fi
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_GO, [test "$use_go" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GUILE

AC_ARG_ENABLE(guile,
  [AS_HELP_STRING([--enable-guile=yes],[guile language bindings])],
  [],[enable_guile=yes])

if test "$enable_guile" != "yes"; then
  use_guile="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_guile="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-guile *- Generate'` = 0; then
      use_guile="No (swig does not support -guile option)"
    else
      PKG_CHECK_MODULES([GUILE], [guile-3.0], [
        use_guile="Yes"
        AC_SUBST([GUILE_CFLAGS])
        AC_SUBST([GUILE_LIBS])
      ], [
        PKG_CHECK_MODULES([GUILE], [guile-2.2], [
          use_guile="Yes"
          AC_SUBST([GUILE_CFLAGS])
          AC_SUBST([GUILE_LIBS])
        ], [
          PKG_CHECK_MODULES([GUILE], [guile-2.0], [
            use_guile="Yes"
            AC_SUBST([GUILE_CFLAGS])
            AC_SUBST([GUILE_LIBS])
          ], [
            PKG_CHECK_MODULES([GUILE], [guile-1.8], [
              use_guile="Yes"
              AC_SUBST([GUILE_CFLAGS])
              AC_SUBST([GUILE_LIBS])
            ], [
              use_guile="No (Guile not available)"
            ])
          ])
        ])
      ])
    fi
  fi
fi
AM_CONDITIONAL(WITH_GUILE, [test "$use_guile" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for JAVA

AC_ARG_ENABLE(java,
  [AS_HELP_STRING([--enable-java=yes],[java language bindings])],
  [], [enable_java=yes])

if test "$enable_java" != "yes"; then
  use_java="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_java="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-java *- Generate'` = 0; then
      use_java="No (swig does not support -java option)"
    else
      AC_CHECK_PROG(JAVA,java,java)
      if test "$JAVA" = ""; then
        use_java="No (java not available)"
      else
        for try_java_include in \
          /System/Library/Frameworks/JavaVM.framework/Headers \
          /usr/lib${LIBPOSTFIX}/jvm/java/include \
          /usr/lib${LIBPOSTFIX}/jvm/java/include/linux \
          /usr/lib${LIBPOSTFIX}/jvm/default-java/include \
          /usr/lib${LIBPOSTFIX}/jvm/default-java/include/linux \
          /usr/lib/jvm/java/include \
          /usr/lib/jvm/java/include/linux \
          /usr/lib/jvm/default-java/include \
          /usr/lib/jvm/default-java/include/linux \
          /etc/java-config-2/current-system-vm/include \
          /etc/java-config-2/current-system-vm/include/linux \
          /etc/java-config-2/current-system-vm/include/freebsd
        do
          if test -d "$try_java_include" ; then
            JAVA_INCLUDES="$JAVA_INCLUDES -I${try_java_include}"
          fi
        done
        JAVA_LIBS=
        AC_ARG_WITH(javaincludedir,
           [AS_HELP_STRING([--with-javaincludedir=DIR],[use JAVA includes from DIR])],
          [JAVA_INCLUDES="-I$withval"])
        AC_ARG_WITH(javalibdir,
          [AS_HELP_STRING([--with-javalibdir=DIR],[use JAVA libraries from DIR])],
          [JAVA_LIBS="$withval"])
	save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $JAVA_INCLUDES"
        AC_CHECK_HEADER(jni.h,,[
            use_java="No (missing header)"
  	    JAVA=
        ])
        CPPFLAGS="$save_CPPFLAGS"
        if test "$JAVA" != ""; then
          use_java="Yes"
	  AC_SUBST([JAVA_INCLUDES])
          AC_SUBST([JAVA_LIBS])
        fi
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_JAVA, [test "$use_java" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for JAVASCRIPT (disabled by default - incomplete)

AC_ARG_ENABLE(javascript,
  [AS_HELP_STRING([--enable-javascript=no],[Javascript language bindings])],
  [], [enable_javascript=no])

if test "$enable_javascript" != "yes"; then
  use_javascript="No (disabled by default - incomplete)"
else
  if test "$use_swig" != "Yes"; then
    use_javascript="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-javascript* *- Generate'` = 0 ; then
      use_javascript="No (swig does not support -javascript option)"
    else
      AC_CHECK_PROG(JAVASCRIPT,js,js)
      if test "$JAVASCRIPT" = ""; then
        use_javascript="No (JAVASCRIPT compiler not available)"
      else
        use_javascript="Yes"
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_JAVASCRIPT, [test "$use_javascript" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for LUA

AC_ARG_ENABLE(lua,
  [AS_HELP_STRING([--enable-lua=yes],[lua language bindings])],
  [], [enable_lua=yes])

if test "$enable_lua" != "yes"; then
  use_lua="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_lua="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-lua *- Generate'` = 0; then
      use_lua="No (swig does not support -lua option)"
    else
      AC_CHECK_PROG(LUA,lua,lua)
      if test "$LUA" = ""; then
        use_lua="No (lua not available)"
      else
        ac_save_CPPFLAGS="$CPPFLAGS"
        ac_save_CFLAGS="$CFLAGS"
        ac_save_LDFLAGS="$LDFLAGS"

        AC_CHECK_PROG(LUA,lua,lua)

        LUA_INCLUDES=""
        LUA_LIBS=""
        LUA_INSTALL_DIR=""

        if test -n "$lua_inc"; then
           CFLAGS="$CFLAGS -I$lua_inc"
           CPPFLAGS="$CPPFLAGS -I$lua_inc"
        fi
        if test -n "$lua_lib"; then
           LDFLAGS="$LDFLAGS -L$lua_lib"
        fi

        if test "$LUA_INCLUDES" = "" -a "$LUA_LIBS" = "" ; then
            if test "$PKG_CONFIG" != ""; then
	        AC_MSG_CHECKING(for Lua headers and libraries with pkg-config)
	        echo
	        for l in "$lua_suffix" "" "54" "5.4" "53" "5.3" "52" "5.2" "51" "5.1" ; do
	            pkgconfig_lua_found=`$PKG_CONFIG --exists lua$l 2>/dev/null`
	            if test "$?" = "0" ; then
                        LUA_INCLUDES="$LUA_CFLAGS "`$PKG_CONFIG --cflags lua$l`
                        LUA_LIBS="$LUA_LFLAGS "`$PKG_CONFIG --libs lua$l`
		        LUA_INSTALL_DIR="`$PKG_CONFIG --variable=libdir lua$l`/lua"

		        pkgconfig_lualib_found=`$PKG_CONFIG --exists lualib$l 2>/dev/null`
		        if test  "$?" = "0"; then
		           LUA_INCLUDES="$LUA_CFLAGS "`$PKG_CONFIG --cflags lualib$l`
		           LUA_LIBS="$LUA_LFLAGS "`$PKG_CONFIG --libs lualib$l`
		        fi
                        break
                    fi
                done
            fi
        fi

        if test "$LUA_INCLUDES" = "" -o "$LUA_LIBS" = "" ; then
            AC_MSG_CHECKING(for Lua headers and libraries)
	    echo
            AC_CHECK_HEADER(lua.h,ac_found_lua_header="yes",ac_found_lua_header="no")
            AC_CHECK_HEADER(lualib.h,ac_found_liblua_header="yes",ac_found_liblua_header="no")
            if test "$ac_found_lua_header" = "yes" -a "$ac_found_liblua_header" = "yes"; then
                LUA_INCLUDES="$CFLAGS"
            fi
            for l in "$lua_suffix" "" "52" "5.2" "51" "5.1" ; do
                AC_CHECK_LIB(lua$l,lua_call,ac_found_lua_lib="yes",ac_found_lua_lib="no")
                if test "$ac_found_lua_lib" = "yes" ; then
	            LUA_LIBS="$LDFLAGS -llua$l -lm"

		    ac2_save_LDFLAGS="$LDFLAGS"
		    LDFLAGS="$LDFLAGS -llua$l -lm"

		    if echo "$host_os" | grep "^linux" 1>/dev/null 2>&1 ; then
		       LDFLAGS="$LDFLAGS -ldl"
		    fi

		    AC_CHECK_LIB(lualib$l,luaL_openlib,ac_found_liblua_lib="yes",ac_found_liblua_lib="no")
		    if test "$ac_found_liblua_lib" = "yes" ; then
		       LUA_LIBS="$LUA_LFLAGS -llualib$l"

		       if echo "$host_os" | grep "^linux" 1>/dev/null 2>&1 ; then
		          LUA_LIBS="$LUA_LFLAGS -ldl"
		       fi
		    fi

		    LDFLAGS="$ac2_save_LDFLAGS"
		    break
	        fi
            done
        fi

        AC_MSG_CHECKING(for Lua compiling and linking)
        LUA_TEST=`LT=luatest.c ; echo "#include <lua.h>" > $LT;  echo "#include <lualib.h>" >> $LT; echo "int main() { return 0; }" >> $LT ; $CC -Wall $LT -o $LT.out $LUA_INCLUDES $LUA_LIBS $CFLAGS $LDFLAGS 1>/dev/null 2>&1 ; echo $?; rm -f $LT $LT.out 1>/dev/null 2>&1`
        if test "$LUA_TEST" != "0" ; then
          AC_MSG_RESULT(no)
          AC_MSG_WARN([
*** Lua (>=5.1) headers and/or libraries could not be found in your system.
*** Try to install liblua, liblualib and liblua-dev with your software package manager.
*** Graphviz will be built without Lua support.])
          use_lua="No (lua compiling/linking failed)"
        else
	  use_lua="Yes"
          AC_SUBST([LUA_INCLUDES])
          AC_SUBST([LUA_LIBS])
          AC_SUBST([LUA_INSTALL_DIR])
          # Not needed for Graphviz: AC_DEFINE(PLUGIN_LUA)
        fi
        CFLAGS="$ac_save_CFLAGS"
        CPPFLAGS="$ac_save_CPPFLAGS"
        LDFLAGS="$ac_save_LDFLAGS"
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_LUA, [test "$use_lua" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for PERL

AC_ARG_ENABLE(perl,
  [AS_HELP_STRING([--enable-perl=yes],[perl language bindings])],
  [], [enable_perl=yes])

if test "$enable_perl" != "yes"; then
  use_perl="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_perl="No (swig not available)"
  else
    if test `$SWIG -perl5 2>&1 | $EGREP -c '(Unable|No target)'` -eq 1; then
      use_perl="No (swig does not support -perl5 option)"
    else
      AC_CHECK_PROG(PERL,perl,perl)
      if test "$PERL" = ""; then
        use_perl="No (perl not available)"
      else
        PERL_INCLUDES="`$PERL $srcdir/config/config_perl.pl PERL_INCLUDES`"
        PERL_LIBS="`$PERL $srcdir/config/config_perl.pl PERL_LIBS`"
        PERL_INSTALL_DIR="`$PERL $srcdir/config/config_perl.pl PERL_INSTALL_DIR`"
        save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $PERL_INCLUDES"
        AC_CHECK_HEADER(EXTERN.h,,[
          use_perl="No (missing header)"
          PERL=
        ])
        CPPFLAGS=$save_CPPFLAGS
        if test "$PERL" != ""; then
          use_perl="Yes"
	  AC_SUBST([PERL_INCLUDES])
          AC_SUBST([PERL_LIBS])
	  AC_SUBST([PERL_INSTALL_DIR])
        fi
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_PERL, [test "$use_perl" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for PHP

AC_ARG_ENABLE(php,
  [AS_HELP_STRING([--enable-php=yes],[php language bindings])],
  [], [enable_php=yes])

if test "$enable_php" != "yes"; then
  use_php="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_php="No (swig not available)"
  else
    if test `$SWIG -php7 2>&1 | $EGREP -c '(Unable|No target)'` -eq 1; then
      SWIG_PHP_OPT='-php'
    else
      SWIG_PHP_OPT='-php7'
    fi
    if test `$SWIG -help 2>&1 | $EGREP -c "(-php|-php7) *- Generate"` = 0 ; then
      use_php="No (swig does not support -php or -php7 option)"
    else
      AC_CHECK_PROG(PHP,php,php)
      if test "$PHP" = ""; then
        use_php="No (php not available)"
      else
        AC_CHECK_PROG(PHPCONFIG,php-config,php-config)
        if test "$PHPCONFIG" = ""; then
          use_php="No (php-config not available)"
        else
          PHP_INCLUDES="`$PHPCONFIG --includes`"
          PHP_INSTALL_DIR="`$PHPCONFIG --extension-dir`"
          PHP_INSTALL_DATADIR="/usr/share/php"
          PHP_LIBS="`$PHPCONFIG --ldflags` `$PHPCONFIG --libs`"
        fi
      fi
      if test "$PHP" != ""; then
	    use_php="Yes"
        AC_SUBST([PHP_INCLUDES])
        AC_SUBST([PHP_INSTALL_DIR])
        AC_SUBST([PHP_INSTALL_DATADIR])
        AC_SUBST([PHP_LIBS])
        AC_SUBST([SWIG_PHP_OPT])
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_PHP, [test "$use_php" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for PYTHON

# alias for `--enable-python3`
AC_ARG_ENABLE(python,
  [AS_HELP_STRING([--enable-python=yes],[python language bindings])],
  [], [enable_python3=yes])

AC_ARG_ENABLE(python3,
  [AS_HELP_STRING([--enable-python3=yes],[python3 language bindings])],
  [], [enable_python3=yes])

if test "$enable_python3" != "yes"; then
  use_python3="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_python3="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-python *- Generate'` = 0; then
      use_python="No (swig does not support -python option)"
    else
      AC_CHECK_PROG(PYTHON3,python3,python3)
      if test "$PYTHON3" = ""; then
        use_python3="No (python3 not available)"
      else
        PYTHON3_VERSION=`$PYTHON3 -c "import sys; print('%d.%d' % sys.version_info[[0:2]])"`
        if test "$PYTHON3_VERSION" = ""; then
          PYTHON3=
        fi
      fi
      if test "$PYTHON3" = ""; then
	use_python3="No (python3 is too old)"
	PYTHON3=
      else
        if test "$PKG_CONFIG" != ""; then
          AC_MSG_CHECKING(for Python3 headers and libraries with pkg-config)
          echo
          pkgconfig_python3_found=`$PKG_CONFIG --exists python-$PYTHON3_VERSION 2>/dev/null`
          if test "$?" = "0" ; then
               PYTHON3_INCLUDES="$PYTHON3_CFLAGS "`$PKG_CONFIG --cflags python-$PYTHON3_VERSION`
               PYTHON3_LIBS="$PYTHON3_LFLAGS "`$PKG_CONFIG --libs python-$PYTHON3_VERSION`
          else
	           use_python3="No (python-$PYTHON3_VERSION.pc not found)"
          fi
        fi
        PYTHON3_INSTALL_DIR="`$PYTHON3 -c 'import sysconfig; print(sysconfig.get_path("platlib"))'`"
        save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $PYTHON3_INCLUDES"
        AC_CHECK_HEADER(Python.h,,[
          use_python3="No (missing header)"
          PYTHON3=
        ])
        CPPFLAGS=$save_CPPFLAGS
	if test "$PYTHON3" != ""; then
	  use_python3="Yes"
          AC_SUBST([PYTHON3_INSTALL_DIR])
          AC_SUBST([PYTHON3_INCLUDES])
          AC_SUBST([PYTHON3_LIBS])
        fi
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_PYTHON3, [test "$use_python3" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for R

AC_ARG_ENABLE(r,
  [AS_HELP_STRING([--enable-r=yes],[R language bindings])],
  [], [enable_r=yes])

if test "$enable_r" != "yes"; then
  use_r="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_r="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-r *- Generate'` = 0; then
      use_r="No (swig does not support -r option)"
    else
      PKG_CHECK_MODULES(R, [libR],[
	use_r="Yes"
	AC_SUBST([R_CFLAGS])
	AC_SUBST([R_LIBS])
      ],[
	use_r="No (R not available)"
      ])
    fi
  fi
fi
AM_CONDITIONAL(WITH_R, [test "$use_r" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for RUBY

AC_ARG_ENABLE(ruby,
  [AS_HELP_STRING([--enable-ruby=yes],[ruby language bindings])],
  [], [enable_ruby=yes])

if test "$enable_ruby" != "yes"; then
  use_ruby="No (disabled)"
else
  if test "$use_swig" != "Yes"; then
    use_ruby="No (swig not available)"
  else
    if test `$SWIG -help 2>&1 | $EGREP -c '\-ruby *- Generate'` = 0; then
      use_ruby="No (swig does not support -ruby option)"
    else
      PKG_CHECK_MODULES([RUBY], [ruby],[
	AC_CHECK_PROG(RUBY,ruby,ruby)
        if test "$RUBY" = ""; then
          use_ruby="No (ruby not available)"
        else
          RUBY_INSTALL_DIR="`$RUBY $srcdir/config/config_ruby.rb vendorarchdir`"
	fi
      ],[
        PKG_CHECK_MODULES([RUBY], [ruby-${RUBY_VER}],[
	  AC_CHECK_PROG(RUBY,ruby,ruby)
          if test "$RUBY" = ""; then
            use_ruby="No (ruby not available)"
          else
            RUBY_INSTALL_DIR="`$RUBY $srcdir/config/config_ruby.rb vendorarchdir`"
	  fi
        ],[
          AC_CHECK_PROG(RUBY,ruby,ruby)
          if test "$RUBY" = ""; then
	    use_ruby="No (ruby not available)"
          else
            RUBY_CFLAGS="-I`$RUBY $srcdir/config/config_ruby.rb rubyarchhdrdir`"
            RUBY_CFLAGS="$RUBY_CFLAGS -I`$RUBY $srcdir/config/config_ruby.rb vendorhdrdir`"
            RUBY_CFLAGS="$RUBY_CFLAGS -I`$RUBY $srcdir/config/config_ruby.rb sitehdrdir`"
            RUBY_CFLAGS="$RUBY_CFLAGS -I`$RUBY $srcdir/config/config_ruby.rb rubyhdrdir`"
            RUBY_LIBS="-L`$RUBY $srcdir/config/config_ruby.rb libdir` `$RUBY -rrbconfig -e \"puts RbConfig::CONFIG[['LIBRUBYARG_SHARED']]\"`"
            RUBY_INSTALL_DIR="`$RUBY $srcdir/config/config_ruby.rb vendorarchdir`"
	    if test "$RUBY_INSTALL_DIR" = ""; then
	  	  RUBY_INSTALL_DIR="`$RUBY $srcdir/config/config_ruby.rb sitearchdir`"
	    fi
            save_CPPFLAGS=$CPPFLAGS
            CPPFLAGS="$CPPFLAGS $RUBY_CFLAGS"
            AC_CHECK_HEADER(ruby.h,,[
              use_ruby="No (missing header)"
              RUBY=
            ])
            CPPFLAGS=$save_CPPFLAGS
          fi
        ])
      ])
      if test "$RUBY" != ""; then
	use_ruby="Yes"
        AC_SUBST([RUBY_CFLAGS])
        AC_SUBST([RUBY_LIBS])
        AC_SUBST([RUBY_INSTALL_DIR])
      fi
    fi
  fi
fi
AM_CONDITIONAL(WITH_RUBY, [test "$use_ruby" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for TCL

AC_ARG_ENABLE(tcl,
  [AS_HELP_STRING([--enable-tcl=yes],[tcl language bindings])],
  [], [enable_tcl=yes])

if test "$enable_tcl" != "yes"; then
  use_tcl="No (disabled)"
fi

if test "$use_tcl" = ""; then
  TCL_VERSION_FOUND=`echo 'puts [[info tclversion]]' | $TCLSH`

  # can't assume ksh on all architectures
  # TCLSH_EXEC_PREFIX=${TCLSH%%/bin/tclsh.*}
  # so use sed instead
  TCLSH_EXEC_PREFIX=`echo $TCLSH|sed -e 's%/bin/.*tclsh.*$%%'`

  if test -f ${TCLSH_EXEC_PREFIX}/include/tcl${TCL_VERSION_FOUND}/tcl.h; then
    TCL_INCLUDES="-I${TCLSH_EXEC_PREFIX}/include/tcl${TCL_VERSION_FOUND}"
    if test -d ${TCLSH_EXEC_PREFIX}/include/tcl${TCL_VERSION_FOUND}/generic; then
      TCL_INCLUDES="${TCL_INCLUDES} -I${TCLSH_EXEC_PREFIX}/include/tcl${TCL_VERSION_FOUND}/generic"
    fi
  else
    if test "${TCLSH_EXEC_PREFIX}" != "/usr" -a -f ${TCLSH_EXEC_PREFIX}/include/tcl.h; then
      TCL_INCLUDES=-I${TCLSH_EXEC_PREFIX}/include
    fi
  fi
  if test "$TCL_INCLUDES" != "" ; then
    AC_MSG_RESULT([using tcl headers from $TCL_INCLUDES])
  fi
  AC_SUBST([TCL_INCLUDES])
  save_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$CPPFLAGS $TCL_INCLUDES"
  AC_CHECK_HEADER(tcl.h,,[
    AC_MSG_WARN([Unable to find header tcl.h. The Tcl packages will not be built])
    use_tcl="No (missing header)"
  ])
  CPPFLAGS=$save_CPPFLAGS
fi
AC_SUBST([TCLSH_EXEC_PREFIX])

if test "$use_tcl" = ""; then
  # Some systems don't have Tcl.  Don't build
  # the Tcl products if we don't have the library.
  if test -f ${TCLSH_EXEC_PREFIX}/lib${LIBPOSTFIX}/tcl${TCL_VERSION_FOUND}/tclConfig.sh; then
    TCLCONFIG=${TCLSH_EXEC_PREFIX}/lib${LIBPOSTFIX}/tcl${TCL_VERSION_FOUND}/tclConfig.sh
  else
    if test -f ${TCLSH_EXEC_PREFIX}/lib${LIBPOSTFIX}/tclConfig.sh; then
      TCLCONFIG=${TCLSH_EXEC_PREFIX}/lib${LIBPOSTFIX}/tclConfig.sh
    else
      if test -f ${TCLSH_EXEC_PREFIX}/lib/tclConfig.sh; then
        TCLCONFIG=${TCLSH_EXEC_PREFIX}/lib/tclConfig.sh
      fi
    fi
  fi
  if test "$TCLCONFIG" != ""; then
    AC_MSG_RESULT([using $TCLCONFIG])
    file=${TCLCONFIG}
      . $file
  else
    AC_MSG_WARN([Unable to find tclConfig.sh. The Tcl packages will not be built])
    use_tcl="No (missing tclConfig.sh)"
  fi
  TCL_INSTALL_DIR=${TCLSH_EXEC_PREFIX}/lib${LIBPOSTFIX}/tcl${TCL_VERSION_FOUND}
fi

if test "$use_tcl" = ""; then
  # TCL STUBS support is required
  if test "${TCL_SUPPORTS_STUBS}" = "1"; then
    use_tcl="Yes"
    TCL_CFLAGS="${TCL_CFLAGS} -DUSE_TCL_STUBS"
    AC_DEFINE_UNQUOTED(HAVE_TCL,1,
	[Define if you have the tcl library])
  else
    AC_MSG_WARN([Tcl does not have STUBs support, perhaps it is too old?  The Tcl packages will not be built])
    use_tcl="No (no STUBs support)"
  fi
fi

AC_SUBST([TCLCONFIG])
AC_SUBST([TCL_INSTALL_DIR])

AM_CONDITIONAL(WITH_TCL, [test "$use_tcl" = "Yes"])

AC_SUBST([TCL_VERSION])
AC_SUBST([TCL_MAJOR_VERSION])
AC_SUBST([TCL_MINOR_VERSION])
AC_SUBST([TCL_PATCH_LEVEL])
AC_SUBST([TCL_CFLAGS])
AC_SUBST([TCL_DEFS])

# TCL modules use stubs and don't need any other tcl libraries
TCL_LIBS=
AC_SUBST([TCL_LIBS])

AC_SUBST([TCL_SHLIB_SUFFIX])
AC_SUBST([TCL_SRC_DIR])
AC_SUBST([TCL_STUB_LIB_SPEC])

dnl -----------------------------------
dnl Support for generic "extra" search paths for includes and libraries

AC_ARG_WITH(extraincludedir,
  [AS_HELP_STRING([--with-extraincludedir=DIR],[use extra includes from DIR])],
  [CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_WITH(extralibdir,
  [AS_HELP_STRING([--with-extralibdir=DIR],[use extra libraries from DIR])],
  [LDFLAGS="$LDFLAGS -L$withval"])

dnl -----------------------------------
dnl INCLUDES and LIBS for EXPAT

AC_ARG_WITH(expat,
  [AS_HELP_STRING([--with-expat=yes],[use expat])],
  [],[with_expat=yes])

if test "$with_expat" != "yes"; then
  use_expat="No (disabled)"
else
  use_expat="Yes"
  PKG_CHECK_MODULES([EXPAT],[expat],[],[
    AC_CHECK_HEADERS([expat.h],[
      AC_CHECK_LIB([expat],[main],[
         EXPAT_LIBS="$EXPAT_LIBS -lexpat"
        ],[
         use_expat="No (expat library not available)"
        ],[])
      ],[
       use_expat="No (expat.h missing header)"
       AC_MSG_WARN(expat.h missing header)
    ],[])
  ],[])

  if test "$use_expat" = "Yes"; then
    AC_DEFINE_UNQUOTED([HAVE_EXPAT],[1],[Define if you have the expat library])
  fi
fi

dnl -----------------------------------
dnl INCLUDES and LIBS for DEVIL

AC_ARG_WITH(devil,
  [AS_HELP_STRING([--with-devil=yes],[DevIL plugin])],
  [],[with_devil=yes])

if test "$with_devil" != "yes"; then
  use_devil="No (disabled)"
else

  AC_ARG_WITH(devilincludedir,
    [AS_HELP_STRING([--with-devilincludedir=DIR],[use DevIL includes from DIR])],
    [DEVIL_INCLUDES="-I$withval"])
  AC_ARG_WITH(devillibdir,
    [AS_HELP_STRING([--with-devillibdir=DIR],[use DevIL libraries from DIR])],
    [DEVIL_LIBS="-L$withval"])

  save_CPPFLAGS=$CPPFLAGS
  save_LDFLAGS=$LDFLAGS
  CPPFLAGS="$CPPFLAGS $DEVIL_INCLUDES"
  LDFLAGS="$LDFLAGS $DEVIL_LIBS"
  AC_CHECK_HEADERS([IL/il.h],
    [],
    [use_devil="No (missing header)"
     AC_MSG_WARN(Optional DevIL library not available - missing headers)],
    [])
  AC_CHECK_LIB(IL,main,
    [DEVIL_LIBS="$DEVIL_LIBS -lIL -lILU"],
    [use_devil="No (missing library)"
     AC_MSG_WARN(Optional DevIL library not available)],
    [])
  LDFLAGS=$save_LDFLAGS
  CPPFLAGS=$save_CPPFLAGS
fi
if test "$use_devil" = ""; then
  use_devil="Yes"
  AC_DEFINE_UNQUOTED(HAVE_DEVIL,1,
    [Define if you have the DevIL library])
  AC_SUBST([DEVIL_INCLUDES])
  AC_SUBST([DEVIL_LIBS])
fi

AM_CONDITIONAL(WITH_DEVIL, [test "$use_devil" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for Z

AC_ARG_WITH(zincludedir,
  [AS_HELP_STRING([--with-zincludedir=DIR],[use Z includes from DIR])],
  [Z_INCLUDES="-I$withval"])
AC_ARG_WITH(zlibdir,
  [AS_HELP_STRING([--with-zlibdir=DIR],[use Z libraries from DIR])],
  [Z_LIBS="-L$withval"])

save_CPPFLAGS=$CPPFLAGS
save_LDFLAGS=$LDFLAGS
CPPFLAGS="$CPPFLAGS $Z_INCLUDES"
LDFLAGS="$LDFLAGS $Z_LIBS"
AC_CHECK_HEADER(zlib.h,
	[AC_CHECK_LIB(z,main,
		[Z_LIBS="$Z_LIBS -lz"
		AC_DEFINE_UNQUOTED(HAVE_LIBZ,1,[Define if you have the Z library])],
			AC_MSG_WARN(Optional z library not available))],
	AC_MSG_WARN(Optional z library not available - no zlib.h))
	LDFLAGS="$LDFLAGS $Z_LIBS"
LDFLAGS=$save_LDFLAGS
CPPFLAGS=$save_CPPFLAGS
AC_SUBST([Z_INCLUDES])
AC_SUBST([Z_LIBS])

dnl -----------------------------------
dnl INCLUDES and LIBS for WEBP

AC_ARG_WITH(webp,
  [AS_HELP_STRING([--with-webp=yes],[webp library])],
  [], [with_webp=yes])

if test "$with_webp" != "yes"; then
  use_webp="No (disabled)"
else
  PKG_CHECK_MODULES(WEBP, [libwebp],[
    use_webp="Yes"
    AC_DEFINE_UNQUOTED(HAVE_WEBP,1,
      [Define if you have the webp library])
    AC_SUBST([WEBP_CFLAGS])
    AC_SUBST([WEBP_LIBS])
  ],[
    use_webp="No (webp library not available)"
  ])
fi
AM_CONDITIONAL(WITH_WEBP, [test "$use_webp" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for POPPLER

AC_ARG_WITH(poppler,
  [AS_HELP_STRING([--with-poppler=yes],[poppler library])],
  [], [with_poppler=yes])

if test "$with_poppler" != "yes"; then
  use_poppler="No (disabled)"
else
  PKG_CHECK_MODULES(POPPLER, [poppler-glib],[
    use_poppler="Yes"
    AC_DEFINE_UNQUOTED(HAVE_POPPLER,1,
      [Define if you have the poppler library])
    AC_SUBST([POPPLER_CFLAGS])
    AC_SUBST([POPPLER_LIBS])
  ],[
    use_poppler="No (poppler library not available)"
  ])
fi
AM_CONDITIONAL(WITH_POPPLER, [test "$use_poppler" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for RSVG

AC_ARG_WITH(rsvg,
  [AS_HELP_STRING([--with-rsvg=yes],[rsvg library])],
  [], [with_rsvg=yes])

if test "$with_rsvg" != "yes"; then
  use_rsvg="No (disabled)"
else
  PKG_CHECK_MODULES(RSVG, [librsvg-2.0],[
    use_rsvg="Yes"
    AC_SUBST([RSVG_CFLAGS])
    AC_SUBST([RSVG_LIBS])
  ],[
    use_rsvg="No (rsvg library not available)"
  ])
fi
AM_CONDITIONAL(WITH_RSVG, [test "$use_rsvg" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GHOSTSCRIPT

AC_ARG_WITH(ghostscript,
  [AS_HELP_STRING([--with-ghostscript=yes],[ghostscript library])],
  [], [with_ghostscript=yes])

if test "$with_ghostscript" != "yes"; then
  use_ghostscript="No (disabled)"
else
  if test "$HAVE_XRENDER" != "yes" -a "$MINGW32" != "yes"; then
    use_ghostscript="No (missing Xrender)"
  else
      AC_CHECK_HEADER(ghostscript/iapi.h, GS_CFLAGS="",use_ghostscript="No (missing headers)")
      if test "$use_ghostscript" = ""; then
        AC_CHECK_LIB(gs, main, GS_LIBS="-lgs", use_ghostscript="No (missing lib)")
        if test "$use_ghostscript" = ""; then
          use_ghostscript="Yes"
          AC_DEFINE_UNQUOTED(HAVE_GS,1,
            [Define if you have the gs library])
          AC_SUBST([GS_CFLAGS])
          AC_SUBST([GS_LIBS])
        fi
     fi
  fi
fi
AM_CONDITIONAL(WITH_GS, [test "$use_ghostscript" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for PANGOCAIRO

AC_ARG_WITH(pangocairo,
  [AS_HELP_STRING([--with-pangocairo=yes],[pangocairo library])],
  [], [with_pangocairo=yes])

if test "$with_pangocairo" != "yes"; then
  use_pangocairo="No (disabled)"
else
  PKG_CHECK_MODULES(PANGOCAIRO, [pangocairo >= 1.22.0],[
    use_pangocairo="Yes"
    AC_DEFINE_UNQUOTED(HAVE_PANGOCAIRO,1,
      [Define if you have the pangocairo library])
    AC_SUBST([PANGOCAIRO_CFLAGS])
    AC_SUBST([PANGOCAIRO_LIBS])
  ],[
    use_pangocairo="No (pangocairo library not available)"
  ])
fi
AM_CONDITIONAL(WITH_PANGOCAIRO, [test "$use_pangocairo" = "Yes"])

# see if pango uses a fontconfig/freetype2 backend
if test "$use_pangocairo" = "Yes"; then
    save_LIBS=$LIBS
    PKG_CHECK_MODULES(PANGOFT2, [pangoft2])
    LIBS="$LIBS $PANGOCAIRO_LIBS $PANGOFT2_LIBS"
    AC_CHECK_FUNCS([pango_fc_font_lock_face])
    LIBS=$save_LIBS
fi

dnl -----------------------------------
dnl INCLUDES and LIBS for LASI

AC_ARG_WITH(lasi,
  [AS_HELP_STRING([--with-lasi=yes],[lasi library])],
  [], [with_lasi=yes])

if test "$with_lasi" != "yes"; then
  use_lasi="No (disabled)"
else
  if test "$use_pangocairo" != "Yes"; then
    use_lasi="No (missing pangocairo support)"
  else
    PKG_CHECK_MODULES(LASI, [lasi],[
      use_lasi="Yes"
      AC_DEFINE_UNQUOTED(HAVE_LASI,1,
        [Define if you have the lasi library])
      AC_SUBST([LASI_CFLAGS])
      AC_SUBST([LASI_LIBS])
    ],[
      use_lasi="No (lasi library not available)"
    ])
  fi
fi
AM_CONDITIONAL(WITH_LASI, [test "$use_lasi" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for FREETYPE2

AC_ARG_WITH(freetype2,
  [AS_HELP_STRING([--with-freetype2=yes],[freetype2 library])],
  [], [with_freetype2=yes])

if test "$with_freetype2" != "yes"; then
  use_freetype="No (disabled)"
else
  PKG_CHECK_MODULES(FREETYPE2, [freetype2],[
    use_freetype="Yes"
    AC_SUBST([FREETYPE2_CFLAGS])
    AC_SUBST([FREETYPE2_LIBS])
  ],[
    use_freetype="No (missing freetype-config)"
  ])
fi

dnl -----------------------------------
dnl INCLUDES and LIBS for GDK

AC_ARG_WITH(gdk,
  [AS_HELP_STRING([--with-gdk=yes],[gdklibrary])],
  [], [with_gdk=yes])

if test "$with_gdk" != "yes"; then
  use_gdk="No (disabled)"
else
  PKG_CHECK_MODULES(GDK, [gdk-3.0],[
    use_gdk="Yes"
    AC_SUBST([GDK_CFLAGS])
    AC_SUBST([GDK_LIBS])
  ],[
    use_gdk="No (gdk library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GDK, [test "$use_gdk" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GDK_PIXBUF.

AC_ARG_WITH(gdk-pixbuf,
  [AS_HELP_STRING([--with-gdk-pixbuf=yes],[gdk-pixbuf library])],
  [], [with_gdk_pixbuf=yes])

if test "$with_gdk_pixbuf" != "yes"; then
  use_gdk_pixbuf="No (disabled)"
else
  PKG_CHECK_MODULES(GDK_PIXBUF, [gdk-pixbuf-2.0],[
    use_gdk_pixbuf="Yes"
    AC_SUBST([GDK_PIXBUF_CFLAGS])
    AC_SUBST([GDK_PIXBUF_LIBS])
  ],[
    use_gdk_pixbuf="No (gdk_pixbuf library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GDK_PIXBUF, [test "$use_gdk_pixbuf" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GTK.

AC_ARG_WITH(gtk,
  [AS_HELP_STRING([--with-gtk=yes],[gtk+ library])],
  [], [with_gtk=yes])

if test "$with_gtk" != "yes"; then
  use_gtk="No (disabled)"
else
  PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.7.0],[
    use_gtk="Yes"
    AC_DEFINE_UNQUOTED(HAVE_GTK,1,
      [Define if you have the gtk library])
    AC_SUBST([GTK_CFLAGS])
    AC_SUBST([GTK_LIBS])
  ],[
    use_gtk="No (gtk library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GTK, [test "$use_gtk" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GTKGL.

AC_ARG_WITH(gtkgl,
  [AS_HELP_STRING([--with-gtkgl=yes],[gtkgl library])],
  [], [with_gtkgl=yes])

if test "$with_gtkgl" != "yes"; then
  use_gtkgl="No (disabled)"
else
  PKG_CHECK_MODULES(GTKGL, [gtkgl-2.0],[
    use_gtkgl="Yes"
    AC_DEFINE_UNQUOTED(HAVE_GTKGL,1,
      [Define if you have the gtkgl library])
    AC_SUBST([GTKGL_CFLAGS])
    AC_SUBST([GTKGL_LIBS])
  ],[
    use_gtkgl="No (gtkgl library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GTKGL, [test "$use_gtkgl" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GTKGLEXT.

AC_ARG_WITH(gtkglext,
  [AS_HELP_STRING([--with-gtkglext=yes],[gtkglext library])],
  [], [with_gtkglext=yes])

if test "$with_gtkglext" != "yes"; then
  use_gtkglext="No (disabled)"
else
  PKG_CHECK_MODULES(GTKGLEXT, [gtkglext-1.0],[
    use_gtkglext="Yes"
    AC_SUBST([GTKGLEXT_CFLAGS])
    AC_SUBST([GTKGLEXT_LIBS])
  ],[
    use_gtkglext="No (gtkglext library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GTKGLEXT, [test "$use_gtkglext" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GTS.

AC_ARG_WITH(gts,
  [AS_HELP_STRING([--with-gts=yes],[gts library])],
  [], [with_gts=yes])

if test "$with_gts" != "yes"; then
  use_gts="No (disabled)"
else
  PKG_CHECK_MODULES(GTS, [gts],[
    use_gts="Yes"
    AC_DEFINE_UNQUOTED(HAVE_GTS,1,
      [Define if you have the gts library])
    AC_SUBST([GTS_CFLAGS])
    AC_SUBST([GTS_LIBS])
  ],[
    use_gts="No (gts library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GTS, [test "$use_gts" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for ANN.

AC_ARG_WITH(ann,
  [AS_HELP_STRING([--with-ann=yes],[ANN library])],
  [], [with_ann=yes])

if test "$with_ann" != "yes"; then
  use_ann="No (disabled)"
else
  PKG_CHECK_MODULES(ANN, [ann],[
    use_ann="Yes"
    AC_SUBST([ANN_CFLAGS])
    AC_SUBST([ANN_LIBS])
  ],[
    # fall back discovery for the Debian ecosystem which does not ship an ann.pc
    AS_IF([test -f /usr/include/ANN/ANN.h], [
      ANN_CFLAGS= # nothing required
      AC_CHECK_LIB(ann, ann_visit_pts, [
        ANN_LIBS=-lann
        use_ann="Yes"
      ], [
        use_ann="No (no ann.pc or libANN found)"
      ])
    ], [
      # fall back discovery for Macports
      AS_IF([test -f /opt/local/include/ANN/ANN.h], [
        use_ann="Yes"
        ANN_CFLAGS="-I/opt/local/include"
        ANN_LIBS="-L/opt/local/lib -lann"
      ], [
        use_ann="No (no ann.pc or ANN.h found)"
      ])
    ])
  ])
fi
AM_CONDITIONAL(WITH_ANN, [test "${use_ann%% *}" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GLADE.

AC_ARG_WITH(glade,
  [AS_HELP_STRING([--with-glade=yes],[glade library])],
  [], [with_glade=yes])

if test "$with_glade" != "yes"; then
  use_glade="No (disabled)"
else
  PKG_CHECK_MODULES(GLADE, [libglade-2.0],[
    use_glade="Yes"
    AC_SUBST([GLADE_CFLAGS])
    AC_SUBST([GLADE_LIBS])
  ],[
    use_glade="No (glade library not available)"
  ])
fi
AM_CONDITIONAL(WITH_GLADE, [test "$use_glade" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for QT

AC_ARG_WITH([qt],
  [AS_HELP_STRING([--with-qt=yes], [Qt features])])

AS_IF([test "$with_qt" != "no"], [
  # required for linking with QT
  CXXFLAGS="${CXXFLAGS} -fPIE -fPIC"
  AC_CHECK_PROGS([QMAKE], [qmake6 qmake-qt5 qmake5 qmake], [false])
  AS_IF([test "$QMAKE" = "qmake6"], [
    # Qt6
    AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_HEADERS)/QtCore/QtCore, [
      QTCORE_CFLAGS="-I$(${QMAKE} -query QT_INSTALL_HEADERS) -I$(${QMAKE} -query QT_INSTALL_HEADERS)/QtCore"
      AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_LIBS)/libQt6Core.so, [
        QTCORE_LIBS="-L$(${QMAKE} -query QT_INSTALL_LIBS) -lQt6Core"
        AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_HEADERS)/QtWidgets/QtWidgets, [
          AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_HEADERS)/QtPrintSupport/QtPrintSupport, [
            QTGUI_CFLAGS="-I$(${QMAKE} -query QT_INSTALL_HEADERS) -I$(${QMAKE} -query QT_INSTALL_HEADERS)/QtWidgets -I$(${QMAKE} -query QT_INSTALL_HEADERS)/QtPrintSupport -I$(${QMAKE} -query QT_INSTALL_HEADERS)/QtGui"
            AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_LIBS)/libQt6Widgets.so, [
              AC_CHECK_FILE($(${QMAKE} -query QT_INSTALL_LIBS)/libQt6PrintSupport.so, [
                QTGUI_LIBS="-L$(${QMAKE} -query QT_INSTALL_LIBS) -lQt6Widgets -lQt6PrintSupport -lQt6Gui -lQt6Core"
                use_qt="Yes"
              ], [
                use_qt="No (Qt6PrintSupport library not available)"
              ])
            ], [
              use_qt="No (Qt6Widgets library not available)"
            ])
          ], [
            use_qt="No (Qt6PrintSupport header not available)"
          ])
        ], [
          use_qt="No (Qt6Widgets header not available)"
        ])
      ], [
        use_qt="No (Qt6Core library not available)"
      ])
    ], [
      use_qt="No (Qt6Core header not available)"
    ])
  ], [
    # Qt5
    AS_IF([test "$QMAKE" != "false"], [
      PKG_CHECK_MODULES([QTCORE], [Qt5Core], [
        PKG_CHECK_MODULES([QTGUI], [Qt5Widgets Qt5PrintSupport], [
          use_qt="Yes"
        ], [
          use_qt="No (QtGui not available)"
        ])
      ], [
        use_qt="No (QtCore not available)"
      ])
    ], [
      use_qt="No (qmake not found)"
    ])
  ])

  AS_IF([test "$with_qt" = "yes" && test "$use_qt" != "Yes"], [
    AC_MSG_ERROR([You requested Qt, but dependencies could not be found: ${use_qt}])
  ])
], [
  use_qt="No (disabled)"
])
AM_CONDITIONAL([WITH_QT], [test "$use_qt" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for QUARTZ

AC_ARG_WITH(quartz,
  [AS_HELP_STRING([--with-quartz=no],[Quartz framework (Mac OS X)])],
  [], [with_quartz=no])

if test "$with_quartz" != "yes"; then
  use_quartz="No (disabled by default - Mac only)"
else
  if test -d "/System/Library/Frameworks/ApplicationServices.framework"; then
    use_quartz="Yes"
    QUARTZ_CFLAGS=''
    QUARTZ_LIBS='-framework ApplicationServices'
    AC_SUBST([QUARTZ_CFLAGS])
    AC_SUBST([QUARTZ_LIBS])
  else
    use_quartz="No (missing ApplicationServices.framework)"
  fi
fi
AM_CONDITIONAL(WITH_QUARTZ, [test "$use_quartz" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for PLATFORMSDK

AC_ARG_WITH(platformsdkincludedir,
  [AS_HELP_STRING([--with-platformsdkincludedir=DIR],[use Platform SDK (Windows) includes from DIR])],
  [PLATFORMSDKINCLUDE=$withval])

AC_ARG_WITH(platformsdklibdir,
  [AS_HELP_STRING([--with-platformsdklibdir=DIR],[use Platform SDK (Windows) libraries from DIR])],
  [PLATFORMSDKLIB=$withval])

AC_SUBST([PLATFORMSDKINCLUDE])
AC_SUBST([PLATFORMSDKLIB])

dnl -----------------------------------
dnl INCLUDES and LIBS for GDI+

AC_ARG_WITH(gdiplus,
  [AS_HELP_STRING([--with-gdiplus=no],[GDI+ framework (Windows)])],
  [], [with_gdiplus=no])

if test "$with_gdiplus" != "yes"; then
  use_gdiplus="No (disabled by default - Windows only)"
else
  if test -f "$PLATFORMSDKINCLUDE\GdiPlus.h" -a -f "$PLATFORMSDKLIB\GdiPlus.lib"; then
    use_gdiplus="Yes"
    AC_DEFINE_UNQUOTED(HAVE_GDIPLUS,1,[Define if you have the GDI+ framework for Windows])
    GDIPLUS_HEADS=`cd "$PLATFORMSDKINCLUDE" && echo GdiPlus*.h`
    GDIPLUS_CFLAGS=''
    GDIPLUS_LIBS=''
    AC_SUBST([GDIPLUS_HEADS])
    AC_SUBST([GDIPLUS_CFLAGS])
    AC_SUBST([GDIPLUS_LIBS])
  else
    use_gdiplus="No (missing headers)"
    AC_MSG_WARN(--with-gdiplus requires valid --with-platformsdkincludedir and --with-platformsdklibdir.)
  fi
fi
AM_CONDITIONAL(WITH_GDIPLUS, [test "$use_gdiplus" = "Yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GD

AC_ARG_WITH(libgd,
  [AS_HELP_STRING([--with-libgd=yes],[use gd library])],
  [], [with_libgd=yes])

if test "$with_libgd" != "yes"; then
  use_gd="No (disabled)"
fi

if test "$use_gd" = ""; then
   PKG_CHECK_MODULES([GDLIB], [gdlib >= 2.0.33],[
      have_gdlib=1
# FIXME - why no features in the gdlib.pc?
      GD_FEATURES="GD_PNG GD_JPEG GD_XPM GD_FONTCONFIG GD_FREETYPE GD_GIF"
   ],[
      AC_PATH_PROG(GDLIB_CONFIG,gdlib-config)
      if test -n "$GDLIB_CONFIG"; then
         have_gdlib=1
         GDLIB_CFLAGS=`$GDLIB_CONFIG --includes`
         GDLIB_LIBS=`$GDLIB_CONFIG --libs`
         GDLIB_VERSION=`$GDLIB_CONFIG --version 2>/dev/null`
         GD_LDFLAGS=`$GDLIB_CONFIG --ldflags | sed 's/-xarch=[^ ]* //'`
         GD_FEATURES=`$GDLIB_CONFIG --features`

         # GD version check >= 2.0.33  -- centos-5.8 has gd-2.0.33
         AX_COMPARE_VERSION([$GDLIB_VERSION], [lt], [2.0.33], [
           use_gd="No (version too old)"
         ])
        if test "$use_gd" != ""; then
            AC_MSG_WARN(GD library version $GDLIB_VERSION < 2.0.33)
        fi
     else
        AC_MSG_WARN(GD neither gdlib pkgconfig nor gdlib-config was found)
     fi
  ])
  if test "$have_gdlib" != ""; then
      AC_DEFINE_UNQUOTED(HAVE_GDLIB,1,[Define if you have the gdlib library])
      for f in $GD_FEATURES ; do
	    if test "$f" = "GD_PNG"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_PNG,1,[Define if the GD library supports PNG])
	    fi
	    if test "$f" = "GD_JPEG"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_JPEG,1,[Define if the GD library supports JPEG])
	    fi
	    if test "$f" = "GD_XPM"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_XPM,1,[Define if the GD library supports XPM])
	    fi
	    if test "$f" = "GD_FONTCONFIG"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_FONTCONFIG,1,[Define if the GD library supports FONTCONFIG])
		        have_gd_fontconfig=1
	    fi
	    if test "$f" = "GD_FREETYPE"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_FREETYPE,1,[Define if the GD library supports FREETYPE])
		        have_gd_freetype=1
	    fi
	    if test "$f" = "GD_GIF"; then
                AC_DEFINE_UNQUOTED(HAVE_GD_GIF,1,[Define if the GD library supports GIF])
	    fi
      done

	if test "$have_gd_fontconfig" = ""; then
        AC_MSG_WARN(Your libgd was not built with fontconfig support. This may result in problems displaying fonts.)
	fi
	if test "$have_gd_freetype" = ""; then
	    AC_MSG_WARN(Your libgd was not built with freetype support. This may result in problems resolving fonts.)
    fi
  fi

  # prevent explicit use of -I/usr/include as it breaks mingw cross-compiles
  if test "$GDLIB_CFLAGS" = "-I/usr/include"; then
     GDLIB_CFLAGS="";
  fi
  # prevent explicit use of -L/usr/lib or -L/usr/lib64, unnecessary clutter
  if test "$GD_LDFLAGS" = "-L/usr/lib"; then
     GD_LDFLAGS="";
  fi
  if test "$GD_LDFLAGS" = "-L/usr/lib64"; then
     GD_LDFLAGS="";
  fi

  if test "$use_gd" = ""; then
    # see if we can use the external gd lib
    save_CPPFLAGS=$CPPFLAGS
    save_LDFLAGS=$LDFLAGS
    AC_ARG_WITH(gdincludedir,
      [AS_HELP_STRING([--with-gdincludedir=DIR],[use GD includes from DIR])],
      GDLIB_CFLAGS="-I$withval")
    CPPFLAGS="$CPPFLAGS $GDLIB_CFLAGS"
    AC_ARG_WITH(gdlibdir,
      [AS_HELP_STRING([--with-gdlibdir=DIR],[use GD libraries from DIR])],
      [GD_LDFLAGS="-L$withval"])
    GDLIB_LIBS="$GD_LDFLAGS -lgd $GDLIB_LIBS"
    LDFLAGS="$LDFLAGS $GDLIB_LIBS"
    AC_CHECK_HEADER(gd.h,,[
      AC_MSG_WARN(Optional GD library not available - no gd.h)
      use_gd="No (gd headers not found)"
    ])
    if test "$use_gd" = ""; then
      AC_CHECK_LIB(gd,main,[
          AC_DEFINE_UNQUOTED(HAVE_LIBGD,1,[Define if you have the GD library])
          use_gd="Yes"
        ],[
          AC_MSG_WARN(Optional GD library not available)
          use_gd="No (library not found)"
        ])
    fi
    LDFLAGS=$save_LDFLAGS
    CPPFLAGS=$save_CPPFLAGS
  fi
  AC_SUBST([GDLIB_CFLAGS])
  AC_SUBST([GDLIB_LIBS])
fi
if test "$use_gd" != "Yes"; then
  with_libgd="no"
fi
AM_CONDITIONAL(WITH_LIBGD, [test "$with_libgd" = "yes"])

dnl -----------------------------------
dnl INCLUDES and LIBS for GLUT

AC_ARG_WITH(glut,
  [AS_HELP_STRING([--with-glut=yes],[GLUT library])],
  [], [with_glut=yes])

if test "$with_glut" != "yes"; then
  use_glut="No (disabled)"
else
  save_CPPFLAGS=$CPPFLAGS
  save_LDFLAGS=$LDFLAGS
  AC_ARG_WITH(glutincludedir,
    [AS_HELP_STRING([--with-glutincludedir=DIR],[use GLUT includes from DIR])],
    [GLUT_CFLAGS="-I$withval"])
  CPPFLAGS="$CPPFLAGS $GLUT_CFLAGS"
  AC_ARG_WITH(glutlibdir,
    [AS_HELP_STRING([--with-glutlibdir=DIR],[use GLUT libraries from DIR])],
    [GLUT_LIBS="-L$withval"])
  LDFLAGS="$LDFLAGS $GLUT_LIBS"

  AC_CHECK_HEADER(GL/glut.h,
    AC_CHECK_LIB(glut,main,
      [GLUT_LIBS="$GLUT_LIBS -lglut"
      use_glut="Yes"
      AC_DEFINE_UNQUOTED(HAVE_GLUT,1,[Define if you have the GLUT library])],
      use_glut="No (missing libglut)"
      AC_MSG_WARN(Optional glut library not available),
      []),
    use_glut="No (missing GL/glut.h)"
    AC_MSG_WARN(Optional glut library not available - no GL/glut.h))
fi

LDFLAGS=$save_LDFLAGS
CPPFLAGS=$save_CPPFLAGS
AC_SUBST([GLUT_CFLAGS])
AC_SUBST([GLUT_LIBS])

# -----------------------------------
# tclInt.h
if test -f "$TCL_SRC_DIR/generic/tclInt.h"; then
TCLINT_INCLUDES="-I$TCL_SRC_DIR/generic"
else
if test -f "/usr/include/tclInt.h"; then
TCLINT_INCLUDES=""
else
if test -f "/usr/local/include/tclInt.h"; then
TCLINT_INCLUDES="-I/usr/local/include"
fi
fi
fi
AC_SUBST([TCLINT_INCLUDES])

# ------------------------------------
# triangle.[ch]

if test -f "lib/sfdpgen/triangle.c"; then
if test -f "lib/sfdpgen/triangle.h"; then
    AC_DEFINE_UNQUOTED(HAVE_TRIANGLE,1,[Define if triangle.[ch] are available.])
fi
fi

# ----------------------------------
# tcl/tk pkgIndex.tcl generation

TCL_PKGINDEX=""
TCL_PKGINDEX_GD=""
TCL_PKGINDEX_SWIG=""
#TK_PKGINDEX=""
if test "$use_tcl" = "Yes"; then
    TCL_PKGINDEX="tcldot/pkgIndex.tcl tclpathplan/pkgIndex.tcl"
    if test "$with_libgd" = "yes"; then
	TCL_PKGINDEX_GD="gdtclft/pkgIndex.tcl"
    fi
    if test "$SWIG" != ""; then
	TCL_PKGINDEX_SWIG="gv/pkgIndex.tcl"
    fi
fi
AC_SUBST([TCL_PKGINDEX])
AC_SUBST([TCL_PKGINDEX_GD])
AC_SUBST([TCL_PKGINDEX_SWIG])

dnl -----------------------------------
dnl SFDP

AC_ARG_WITH(sfdp,
  [AS_HELP_STRING([--with-sfdp=yes],[sfdp layout engine])],
  [], [with_sfdp=yes])

if test "$with_sfdp" != "yes"; then
  use_sfdp="No (disabled)"
else
  use_sfdp="Yes"
  AC_DEFINE_UNQUOTED(SFDP,1,[Define if you want SFDP])
fi
AM_CONDITIONAL(WITH_SFDP, [test "$use_sfdp" = "Yes"])

dnl -----------------------------------
dnl SMYRNA

AC_ARG_WITH(smyrna,
  [AS_HELP_STRING([--with-smyrna=yes],[SMYRNA large graph viewer])],
  [], [with_smyrna=yes])

if test "$with_smyrna" != "yes"; then
  use_smyrna="No (disabled)"
else
  if test "$use_gtk" != "Yes"; then
     AC_MSG_WARN(SMYRNA requires GTK)
     requires=" gtk+"
  fi
  if test "$use_gtkglext" != "Yes"; then
     AC_MSG_WARN(SMYRNA requires GTKGLEXT)
     requires="$requires gtkglext"
  fi
  if test "$use_glade" != "Yes"; then
     AC_MSG_WARN(SMYRNA requires GLADE)
     requires="$requires glade"
  fi
  if test "$use_gts" != "Yes"; then
     AC_MSG_WARN(SMYRNA requires GTS)
     requires="$requires gts"
  fi
  if test "$use_glut" != "Yes"; then
     AC_MSG_WARN(SMYRNA requires GLUT)
     requires="$requires glut"
  fi
  if test "$requires" != ""; then
     use_smyrna="No (requires:$requires)"
  else
     use_smyrna="Yes"
  fi
fi
AM_CONDITIONAL(WITH_SMYRNA, [test "$use_smyrna" = "Yes"])

dnl -----------------------------------
dnl ORTHO

AC_ARG_WITH(ortho,
  [AS_HELP_STRING([--with-ortho=yes],[ORTHO features in neato layout engine])],
  [], [with_ortho=yes])

if test "$with_ortho" != "yes"; then
  use_ortho="No (disabled)"
else
  use_ortho="Yes"
  AC_DEFINE_UNQUOTED(ORTHO,1,[Define if you want ORTHO])
fi
AM_CONDITIONAL(WITH_ORTHO, [test "$use_ortho" = "Yes"])

dnl -----------------------------------
dnl DIGCOLA

AC_ARG_WITH(digcola,
  [AS_HELP_STRING([--with-digcola=yes],[DIGCOLA features in neato layout engine])],
  [], [with_digcola=yes])

if test "$with_digcola" != "yes"; then
  use_digcola="No (disabled)"
else
  use_digcola="Yes"
  AC_DEFINE_UNQUOTED(DIGCOLA,1,[Define if you want DIGCOLA])
fi

dnl -----------------------------------
dnl IPSEPCOLA   /* enabled by default. C++ issues on some platforms should be resolved by now */
AC_ARG_WITH(ipsepcola,
  [AS_HELP_STRING([--with-ipsepcola=yes],[IPSEPCOLA features in neato layout engine])],
  [], [with_ipsepcola=yes])

if test "$with_ipsepcola" != "yes"; then
  use_ipsepcola="No (disabled)"
else
  if test "$use_digcola" != "Yes"; then
    use_ipsepcola="No (requires: digcola)"
  else
    use_ipsepcola="Yes"
    AC_DEFINE_UNQUOTED(IPSEPCOLA,1,[Define if you want IPSEPCOLA])
    IPSEPCOLA_INCLUDES='-I$(top_srcdir)/lib/vpsc'
    IPSEPCOLA_LIBS='$(top_builddir)/lib/vpsc/libvpsc_C.la'
    AC_SUBST([IPSEPCOLA_INCLUDES])
    AC_SUBST([IPSEPCOLA_LIBS])
  fi
fi
AM_CONDITIONAL(WITH_IPSEPCOLA, [test "$use_ipsepcola" = "Yes"])

# -----------------------------------------------------------------------

# This is a historical artifact ... there are no other choices these days
# This #define is never used in our sources,
#    but apparently may still be in use by others
AC_DEFINE_UNQUOTED(WITH_CGRAPH,1,[Historical artifact - always true])
AM_CONDITIONAL(WITH_CGRAPH, [true])

# -----------------------------------------------------------------------

# Generate Makefiles
#	tests/regression_tests/vuln/Makefile was removed
AC_CONFIG_FILES(Makefile
	debian/changelog
	debian/libgv-php5.install
	doc/Makefile
	doc/info/Makefile
	doc/schema/Makefile
	contrib/Makefile
	contrib/prune/Makefile
	contrib/diffimg/Makefile
	graphs/Makefile
	graphs/directed/Makefile
	graphs/undirected/Makefile
	lib/Makefile
	lib/util/Makefile
	lib/cdt/Makefile
	lib/cdt/libcdt.pc
	lib/cgraph/Makefile
	lib/cgraph/libcgraph.pc
	lib/rbtree/Makefile
	lib/pathplan/Makefile
	lib/pathplan/libpathplan.pc
	lib/ast/Makefile
	lib/sfio/Makefile
	lib/sfio/Sfio_f/Makefile
	lib/vmalloc/Makefile
	lib/dotgen/Makefile
	lib/neatogen/Makefile
	lib/fdpgen/Makefile
	lib/sparse/Makefile
	lib/mingle/Makefile
	lib/label/Makefile
	lib/sfdpgen/Makefile
	lib/sfdpgen/libsfdp.pc
	lib/osage/Makefile
	lib/edgepaint/Makefile
	lib/gvpr/Makefile
	lib/gvpr/libgvpr.pc
	lib/circogen/Makefile
	lib/twopigen/Makefile
	lib/patchwork/Makefile
	lib/pack/Makefile
	lib/ortho/Makefile
	lib/expr/Makefile
	lib/expr/libexpr.pc
	lib/common/Makefile
	lib/vpsc/Makefile
	lib/gvc/Makefile
	lib/gvc/libgvc.pc
	lib/xdot/Makefile
	lib/xdot/libxdot.pc
	lib/topfish/Makefile
	lib/glcomp/Makefile
	macosx/Info.plist
	macosx/build/graphviz.pmdoc/01local.xml
	macosx/build/graphviz.pmdoc/02graphviz.xml
	plugin/Makefile
	plugin/core/Makefile
	plugin/devil/Makefile
	plugin/gd/Makefile
	plugin/gdk/Makefile
	plugin/gdiplus/Makefile
	plugin/gs/Makefile
	plugin/kitty/Makefile
	plugin/lasi/Makefile
	plugin/pango/Makefile
	plugin/poppler/Makefile
	plugin/quartz/Makefile
	plugin/rsvg/Makefile
	plugin/webp/Makefile
	plugin/xlib/Makefile
	plugin/dot_layout/Makefile
	plugin/neato_layout/Makefile
	plugin/vt/Makefile
	cmd/Makefile
	cmd/dot/Makefile
	cmd/tools/Makefile
	cmd/gvpr/Makefile
	cmd/gvpr/lib/Makefile
	cmd/smyrna/Makefile
	cmd/gvmap/Makefile
	cmd/mingle/Makefile
	cmd/edgepaint/Makefile
	cmd/gvedit/Makefile
	cmd/gvedit/gvedit.pro
	cmd/gvedit/ui/Makefile
	cmd/gvedit/images/Makefile
	tclpkg/Makefile
	tclpkg/tclstubs/Makefile
	tclpkg/gdtclft/Makefile
	tclpkg/gdtclft/demo/Makefile
	tclpkg/tcldot/Makefile
	tclpkg/tcldot/demo/Makefile
	tclpkg/tclpathplan/Makefile
	tclpkg/tclpathplan/demo/Makefile
	tclpkg/tclpathplan/demo/pathplan_data/Makefile
	tclpkg/gv/Makefile
	tclpkg/gv/demo/Makefile
	tclpkg/gv/META.gv
	tests/graphs/Makefile
	tests/linux.x86/Makefile
	tests/Makefile
	tests/regression_tests/Makefile
	tests/regression_tests/shapes/Makefile
	tests/regression_tests/shapes/reference/Makefile
	tests/regression_tests/vuln/Makefile
	tests/regression_tests/vuln/input/Makefile
	tests/regression_tests/vuln/reference/Makefile
	share/Makefile
	share/examples/Makefile
	share/gui/Makefile
	redhat/graphviz.spec.fedora
	redhat/graphviz.spec.rhel
	Doxyfile
	)

AC_OUTPUT

dnl ===========================================================================

if test "$use_tcl" != "Yes"; then
  use_tcldot="No (tcl not available)"
  use_gdtclft="No (tcl not available)"
  use_tclpathplan="No (tcl not available)"
  use_gv_tcl="No (tcl not available)"
else
  use_tcldot="Yes"
  use_tclpathplan="Yes"
  if test "$use_swig" != "Yes"; then
    use_gv_tcl="No (swig not available)"
  else
    use_gv_tcl="Yes"
  fi
  if test "$use_gd" != "Yes"; then
    use_gdtclft="No (gd not available)"
  else
    use_gdtclft="Yes"
  fi
fi

echo ""
echo "----------------------------------------------------------------"
echo ""
echo "graphviz-$VERSION will be compiled with the following:"
echo ""
echo "options:"
echo "  cgraph:        Yes (always enabled)"
echo "  digcola:       $use_digcola"
echo "  expat:         $use_expat"
echo "  freetype:      $use_freetype"
echo "  glut:          $use_glut"
echo "  ann:           $use_ann"
echo "  gts:           $use_gts"
echo "  ipsepcola:     $use_ipsepcola"
echo "  ltdl:          $use_ltdl"
echo "  ortho:         $use_ortho"
echo "  sfdp:          $use_sfdp"
echo "  swig:          $use_swig ( $SWIG_VERSION )"
echo "  shared:        $use_shared"
echo "  static:        $use_static"
echo "  qt:            $use_qt"
echo "  x:             $use_xlib"
echo ""
echo "commands:"
echo "  dot:           Yes (always enabled)"
echo "  neato:         Yes (always enabled)"
echo "  fdp:           Yes (always enabled)"
echo "  circo:         Yes (always enabled)"
echo "  twopi:         Yes (always enabled)"
echo "  gvpr:          Yes (always enabled)"
echo "  gvmap:         Yes (always enabled)"
echo "  smyrna:        $use_smyrna"
echo "  gvedit:        $use_qt"
echo ""
echo "plugin libraries:"
echo "  dot_layout:    Yes (always enabled)"
echo "  neato_layout:  Yes (always enabled)"
echo "  core:          Yes (always enabled)"
echo "  devil:         $use_devil"
echo "  gd:            $use_gd"
echo "  gdiplus:       $use_gdiplus"
echo "  gdk:           $use_gdk"
echo "  gdk_pixbuf:    $use_gdk_pixbuf"
echo "  ghostscript:   $use_ghostscript"
echo "  gtk:           $use_gtk"
echo "  lasi:          $use_lasi"
echo "  pangocairo:    $use_pangocairo"
echo "  poppler:       $use_poppler"
echo "  quartz:        $use_quartz"
echo "  rsvg:          $use_rsvg"
echo "  webp:          $use_webp"
echo "  xlib:          $use_xlib"
echo ""
echo "language extensions:"
echo "  gv_sharp:      $use_sharp"
echo "  gv_d:          $use_d"
echo "  gv_go:         $use_go"
echo "  gv_guile:      $use_guile"
echo "  gv_java:       $use_java"
echo "  gv_javascript: $use_javascript"
echo "  gv_lua:        $use_lua"
echo "  gv_perl:       $use_perl"
echo "  gv_php:        $use_php"
echo "  gv_python3:    $use_python3"
echo "  gv_R:          $use_r"
echo "  gv_ruby:       $use_ruby"
echo "  gv_tcl:        $use_gv_tcl"
echo ""
echo "  tcldot:        $use_tcldot"
echo "  tclpathplan:   $use_tclpathplan"
echo "  gdtclft:       $use_gdtclft"
