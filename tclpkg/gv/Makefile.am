## Process this file with automake to produce Makefile.in

SUBDIRS = demo

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gvc \
	-I$(top_srcdir)/lib/common \
	-I$(top_srcdir)/lib/cgraph \
	-I$(top_srcdir)/lib/cdt \
	-I$(top_srcdir)/lib/pathplan

LIBS = -lc

BASESOURCES = gv.cpp gv.i gv_builtins.c gv_channel.h
BASELIBS = $(top_builddir)/lib/gvc/libgvc.la \
	$(top_builddir)/lib/cgraph/libcgraph.la \
	$(top_builddir)/lib/cdt/libcdt.la
BASECPPFLAGS = $(AM_CPPFLAGS) -DDEMAND_LOADING=1

pkgsharpdir = $(pkglibdir)/sharp
SHARP_data = gv.cs gvPINVOKE.cs SWIGTYPE_p_Agraph_t.cs SWIGTYPE_p_Agnode_t.cs SWIGTYPE_p_Agedge_t.cs SWIGTYPE_p_Agsym_t.cs SWIGTYPE_p_FILE.cs
nodist_libgv_sharp_la_SOURCES = gv_sharp.cpp $(SHARP_data)
libgv_sharp_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_sharp_la_LIBADD = $(BASELIBS)
libgv_sharp_la_LDFLAGS = -module -avoid-version
libgv_sharp_la_CPPFLAGS = $(BASECPPFLAGS)
$(SHARP_data): gv_sharp.cpp
gv_sharp.cpp: gv.i
	$(SWIG) -c++ -csharp -namespace gv -o $@ $(srcdir)/gv.i

pkgddir = $(pkglibdir)/d
D_data =
nodist_libgv_d_la_SOURCES = gv_d.cpp $(D_data)
libgv_d_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_d_la_LIBADD = $(BASELIBS)
libgv_d_la_LDFLAGS = -module -avoid-version
libgv_d_la_CPPFLAGS = $(BASECPPFLAGS)
# $(D_data): gv_d.cpp
gv_d.cpp: gv.i
	$(SWIG) -c++ -d -o gv_d.cpp $(srcdir)/gv.i

pkggodir = $(pkglibdir)/go
GO_data = runtime.h gv.go
nodist_libgv_go_la_SOURCES = gv_go.cpp runtime.h gv.go
libgv_go_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_go_la_LIBADD = $(BASELIBS) $(GO_LIBS)
libgv_go_la_LDFLAGS = -module -avoid-version
libgv_go_la_CPPFLAGS = $(BASECPPFLAGS) $(GO_INCLUDES)
libgv_go_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-function
gv_gc.c gv.go: gv_go.cpp
gv_go.cpp: gv.i
	$(SWIG) -c++ -go -intgosize $(INTGOSIZE) -o $@ $(srcdir)/gv.i
runtime.h:
	$(SWIG) -c++ -go -intgosize $(INTGOSIZE) -external-runtime $@
gv.go:
	echo "package gv" >$@

pkgguiledir = $(pkglibdir)/guile
GUILE_data =
nodist_libgv_guile_la_SOURCES = gv_guile.cpp $(GUILE_data)
libgv_guile_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_guile_la_LIBADD = $(BASELIBS) $(GUILE_LIBS)
libgv_guile_la_LDFLAGS = -module -avoid-version
libgv_guile_la_CPPFLAGS = $(BASECPPFLAGS) $(GUILE_CFLAGS)
libgv_guile_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
# $(GUILE_data): gv_guile.cpp
gv_guile.cpp: gv.i
	$(SWIG) -c++ -guile -o $@ $(srcdir)/gv.i

libjavadir = $(pkglibdir)/java
pkgjavadir = $(libjavadir)/org/graphviz
JAVA_data = gv.java gvJNI.java SWIGTYPE_p_Agraph_t.java SWIGTYPE_p_Agnode_t.java SWIGTYPE_p_Agedge_t.java SWIGTYPE_p_Agsym_t.java SWIGTYPE_p_FILE.java
nodist_libgv_java_la_SOURCES = gv_java.cpp
libgv_java_la_SOURCES = $(BASESOURCES) gv_java_init.c
libgv_java_la_LIBADD = $(BASELIBS) $(JAVA_LIBS)
libgv_java_la_LDFLAGS = -module -avoid-version $(JNI_EXTRA_LDFLAGS)
libgv_java_la_CPPFLAGS = $(BASECPPFLAGS) $(SWIGJAVA_CPPFLAGS) $(JAVA_INCLUDES)
$(JAVA_data): gv_java.cpp
gv_java.cpp: gv.i
	$(SWIG) -package org.graphviz -java -c++ -o $@ $(srcdir)/gv.i

pkgjavascriptdir = $(pkglibdir)/javascript
JAVASCRIPT_data =
nodist_libgv_javascript_la_SOURCES = gv_javascript.cpp $(JAVASCRIPT_data)
libgv_javascript_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_javascript_la_LIBADD = $(BASELIBS)
libgv_javascript_la_LDFLAGS = -module -avoid-version
libgv_javascript_la_CPPFLAGS = $(BASECPPFLAGS)
gv_javascript.cpp: gv.i
	$(SWIG) -c++ -javascript -jsc -o $@ $(srcdir)/gv.i

pkgluadir = $(pkglibdir)/lua
LUA_data =
nodist_libgv_lua_la_SOURCES = gv_lua.cpp $(LUA_data)
libgv_lua_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_lua_la_LIBADD = $(BASELIBS) $(LUA_LIBS)
libgv_lua_la_LDFLAGS = -module -avoid-version
libgv_lua_la_CPPFLAGS = $(BASECPPFLAGS) $(LUA_INCLUDES)
# $(LUA_data): gv_lua.cpp
gv_lua.cpp: gv.i
	$(SWIG) -c++ -lua -o $@ $(srcdir)/gv.i

pkgperldir = $(pkglibdir)/perl
PERL ?= perl
PERL_CC = `$(PERL) -MExtUtils::Embed -e ccopts`
PERL_LD = `$(PERL) -MExtUtils::Embed -e ldopts`
VENDOR_ARCH = `$(PERL) -MConfig -e 'print $$Config{installvendorarch};'`
PERL_data = gv.pm
nodist_libgv_perl_la_SOURCES = gv_perl.cpp $(PERL_data)
libgv_perl_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_perl_la_LIBADD = $(BASELIBS) $(PERL_LIBS)
libgv_perl_la_LDFLAGS = -module -avoid-version $(PERL_LD)
libgv_perl_la_CPPFLAGS = $(BASECPPFLAGS) $(PERL_INCLUDES) $(PERL_CC)
libgv_perl_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
$(PERL_data): gv_perl.cpp
gv_perl.cpp: gv.i
	$(SWIG) -c++ -perl5 -o $@ $(srcdir)/gv.i

pkgphpdir = $(pkglibdir)/php
PHP_data = $(SWIG_PHP_DATA)
nodist_libgv_php_la_SOURCES = gv_php.cpp php_gv.h
libgv_php_la_SOURCES = $(BASESOURCES) gv_php_init.c
libgv_php_la_LIBADD = $(BASELIBS) $(PHP_LIBS)
libgv_php_la_LDFLAGS = -module -avoid-version
libgv_php_la_CPPFLAGS = $(BASECPPFLAGS) $(PHP_INCLUDES:-I%=-isystem%)
libgv_php_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-label -Wno-unused-parameter
if WITH_SWIG_PHP_DATA
gv.php: gv_php.cpp
endif
php_gv.h: gv_php.cpp
gv_php.cpp: gv.i
	$(SWIG) -c++ $(SWIG_PHP_OPT) -o $@ $(srcdir)/gv.i

pkgpython3dir = $(pkglibdir)/python3
PYTHON3_data = gv.py
nodist_libgv_python3_la_SOURCES = gv_python3.cpp $(PYTHON3_data)
libgv_python3_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_python3_la_LIBADD = $(BASELIBS) $(PYTHON3_LIBS)
libgv_python3_la_LDFLAGS = -module -avoid-version
libgv_python3_la_CPPFLAGS = $(BASECPPFLAGS) $(PYTHON3_INCLUDES)
$(PYTHON3_data): gv_python3.cpp
gv_python3.cpp: gv.i
	$(SWIG) -c++ -python -o $@ $(srcdir)/gv.i

pkgRdir = $(pkglibdir)/R
R_data =
nodist_libgv_R_la_SOURCES = gv_R.cpp $(R_data)
libgv_R_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_R_la_LIBADD = $(BASELIBS) $(R_LIBS)
libgv_R_la_LDFLAGS = -module -avoid-version
libgv_R_la_CPPFLAGS = $(BASECPPFLAGS) $(R_CFLAGS)
libgv_R_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-cast-function-type -Wno-unused-parameter
# $(R_data): gv_R.cpp
gv_R.cpp: gv.i
	$(SWIG) -c++ -r -o $@ $(srcdir)/gv.i

pkgrubydir = $(pkglibdir)/ruby
RUBY_data =
nodist_libgv_ruby_la_SOURCES = gv_ruby.cpp $(RUBY_data)
libgv_ruby_la_SOURCES = $(BASESOURCES) gv_dummy_init.c
libgv_ruby_la_LIBADD = $(BASELIBS) $(RUBY_LIBS)
libgv_ruby_la_LDFLAGS = -module -avoid-version
libgv_ruby_la_CPPFLAGS = $(BASECPPFLAGS) $(RUBY_CFLAGS)
libgv_ruby_la_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
# $(RUBY_data): gv_ruby.cpp
gv_ruby.cpp: gv.i
	$(SWIG) -c++ -ruby -o $@ $(srcdir)/gv.i

pkgtcldir = $(pkglibdir)/tcl
TCL_data =
nodist_libgv_tcl_la_SOURCES = gv_tcl.cpp $(TCL_data)
libgv_tcl_la_SOURCES = $(BASESOURCES) gv_tcl_init.c
libgv_tcl_la_LIBADD = $(top_builddir)/tclpkg/tclstubs/libtclstubs_C.la $(BASELIBS) $(TCL_LIBS)
libgv_tcl_la_LDFLAGS = -module -avoid-version
libgv_tcl_la_CPPFLAGS = $(BASECPPFLAGS) $(TCL_INCLUDES)
# $(TCL_data): gv_tcl.cpp
gv_tcl.cpp: gv.i
	$(SWIG) -c++ -tcl -namespace -o $@ $(srcdir)/gv.i

pkglib_LTLIBRARIES =
man_MANS =
pdf_DATA =
DEVTSTS =
if WITH_SWIG
if WITH_SHARP
pkgsharp_DATA = $(SHARP_data)
man_MANS += gv.3sharp
if ENABLE_MAN_PDFS
pdf_DATA += gv.3sharp.pdf
endif
pkgsharp_LTLIBRARIES = libgv_sharp.la
DEVTSTS += test_sharp
endif
if WITH_D
pkgd_DATA = $(D_data)
#man_MANS += gv.3d
pkgd_LTLIBRARIES = libgv_d.la
#DEVTSTS += test_d
endif
if WITH_GO
pkggo_DATA = $(GO_data)
man_MANS += gv.3go
if ENABLE_MAN_PDFS
pdf_DATA += gv.3go.pdf
endif
pkggo_LTLIBRARIES = libgv_go.la
DEVTSTS += test_go
endif
if WITH_GUILE
pkgguile_DATA = $(GUILE_data)
man_MANS += gv.3guile
if ENABLE_MAN_PDFS
pdf_DATA += gv.3guile.pdf
endif
pkgguile_LTLIBRARIES = libgv_guile.la
DEVTSTS += test_guile
endif
if WITH_JAVA
pkgjava_DATA = $(JAVA_data)
man_MANS += gv.3java
if ENABLE_MAN_PDFS
pdf_DATA += gv.3java.pdf
endif
pkgjava_LTLIBRARIES = libgv_java.la
DEVTSTS += test_java
endif
if WITH_JAVASCRIPT
pkgd_DATA = $(JAVASCRIPT_data)
#man_MANS += gv.3javascript
pkgjavascript_LTLIBRARIES = libgv_javascript.la
#DEVTSTS += test_javascript
endif
if WITH_LUA
pkglua_DATA = $(LUA_data)
man_MANS += gv.3lua
if ENABLE_MAN_PDFS
pdf_DATA += gv.3lua.pdf
endif
pkglua_LTLIBRARIES = libgv_lua.la
DEVTSTS += test_lua
endif
if WITH_PERL
pkgperl_DATA = $(PERL_data)
man_MANS += gv.3perl
if ENABLE_MAN_PDFS
pdf_DATA += gv.3perl.pdf
endif
pkgperl_LTLIBRARIES = libgv_perl.la
DEVTSTS += test_perl
endif
if WITH_PHP
pkgphp_DATA = $(PHP_data)
man_MANS += gv.3php
if ENABLE_MAN_PDFS
pdf_DATA += gv.3php.pdf
endif
pkgphp_LTLIBRARIES = libgv_php.la
DEVTSTS += test_php
endif
if WITH_PYTHON3
pkgpython3_DATA = $(PYTHON3_data)
pkgpython3_LTLIBRARIES = libgv_python3.la
DEVTSTS += test_python3
endif
man_MANS += gv.3python
if ENABLE_MAN_PDFS
pdf_DATA += gv.3python.pdf
endif
if WITH_R
pkgR_DATA = $(R_data)
man_MANS += gv.3r
if ENABLE_MAN_PDFS
pdf_DATA += gv.3r.pdf
endif
pkgR_LTLIBRARIES = libgv_R.la
DEVTSTS += test_R
endif
if WITH_RUBY
pkgruby_DATA = $(RUBY_data)
man_MANS += gv.3ruby
if ENABLE_MAN_PDFS
pdf_DATA += gv.3ruby.pdf
endif
pkgruby_LTLIBRARIES = libgv_ruby.la
DEVTSTS += test_ruby
endif
if WITH_TCL
BUILT_SOURCES = pkgIndex.tcl
pkgtcl_DATA = $(TCL_data)
man_MANS += gv.3tcl
if ENABLE_MAN_PDFS
pdf_DATA += gv.3tcl.pdf
endif
pkgtcl_LTLIBRARIES = libgv_tcl.la
DEVTSTS += test_tcl

all-local: pkgIndex.tcl

pkgIndex.tcl:
	python3 $(top_srcdir)/tclpkg/mkpkgindex.py \
	  --file libgv_tcl$(SHARED_LIBRARY_SUFFIX) \
	  --name gv \
	  --version 0
endif
endif

man = gv.3sharp gv.3go gv.3guile gv.3java gv.3lua \
	gv.3perl gv.3php gv.3python gv.3r gv.3ruby gv.3tcl

# this rule has a trailing semi-colon for the Make cache to work correctly
# https://stackoverflow.com/questions/37873522/unreliable-parallel-builds-in-a-makefile-with-intermediate
$(man): gv_doc_writer.intermediate ;

.INTERMEDIATE: gv_doc_writer.intermediate
gv_doc_writer.intermediate: gv.i gv_doc_writer.py
	python3 $(srcdir)/gv_doc_writer.py

.3sharp.3sharp.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3go.3go.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3guile.3guile.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3java.3java.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3lua.3lua.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3perl.3perl.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3php.3php.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3python.3python.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3r.3r.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3ruby.3ruby.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
.3tcl.3tcl.pdf:
	rm -f $@; pdffile=$@; psfile=$${pdffile%pdf}ps; \
	$(GROFF) -Tps -man $< > $$psfile || { rm -f $$psfile; exit 1; }; \
	$(PS2PDF) $$psfile && rm -f $$psfile || { rm -f $$psfile; exit 1; }
SUFFIXES = .3sharp .3sharp.pdf .3go .3go.pdf .3guile .3guile.pdf .3java .3java.pdf .3lua .3lua.pdf .3perl .3perl.pdf .3php .3php.pdf .3python .3python.pdf .3r .3r.pdf .3ruby .3ruby.pdf .3tcl .3tcl.pdf

install-data-hook:
if WITH_JAVA
	(cd $(DESTDIR)$(pkgjavadir); $(LN_S) -f org/graphviz/libgv_java.$(JSHEXT) ../../libgv.$(JSHEXT); javac -classpath $(DESTDIR)$(libjavadir) gv.java;)
endif
if WITH_LUA
	(cd $(DESTDIR)$(pkgluadir); rm -f gv.so; $(LN_S) libgv_lua.so gv.so;)
endif
if WITH_PERL
	(cd $(DESTDIR)$(pkgperldir); rm -f gv.so; $(LN_S) libgv_perl.so gv.so;)
endif
if WITH_PHP
	(cd $(DESTDIR)$(pkgphpdir); rm -f gv.so; $(LN_S) libgv_php.so gv.so;)
endif
if WITH_PYTHON3
	(cd $(DESTDIR)$(pkgpython3dir); rm -f _gv.so; $(LN_S) libgv_python3.so _gv.so;)
endif
if WITH_R
	(cd $(DESTDIR)$(pkgRdir); rm -f gv.so; $(LN_S) libgv_R.so gv.so;)
endif
if WITH_RUBY
	(cd $(DESTDIR)$(pkgrubydir); rm -f gv.so; $(LN_S) libgv_ruby.so gv.so;)
endif

devtsts: $(DEVTSTS)

.PHONY: test_sharp
test_sharp: libgv_sharp.la
	-(mkdir -p test_sharp; cd test_sharp; \
		ln -fs ../.libs/libgv_sharp.so libgv_sharp.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.cs .; \
		$(MCS) *.cs -out:test.exe; \
		mono test.exe)

.PHONY: test_go
test_go: libgv_go.la
	-(mkdir -p test_go; cd test_go; \
		ln -fs ../.libs/libgv_go.so libgv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.go .; \
		$(GO) *.go
		LD_LIBRARY_PATH=. go test)

.PHONY: test_guile
test_guile: libgv_guile.la
	-(mkdir -p test_guile; cd test_guile; \
		ln -fs ../.libs/libgv_guile.so libgv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.guile .; \
		$(GUILE) -s test.guile)

.PHONY: test_java
test_java: libgv_java.la
	-(mkdir -p test_java/org/graphviz; cd test_java; \
		ln -f ../.libs/libgv_java.so libgv.jnilib; \
		ln -f ../$(srcdir)/*.java org/graphviz; \
		ln -f ../$(srcdir)/test.java .; \
		ln -f ../$(srcdir)/hello.gv .; \
		ln -f ../$(srcdir)/test.java .; \
		CLASSPATH=. javac *.java; \
		LD_LIBRARY_PATH=. CLASSPATH=. java test)

.PHONY: test_lua
test_lua: libgv_lua.la
	-(mkdir -p test_lua; cd test_lua; \
		ln -fs ../.libs/libgv_lua.so gv.so;\
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.lua .; \
		$(LUA) test.lua)

.PHONY: test_perl
test_perl: libgv_perl.la
	-(mkdir -p test_perl; cd test_perl; \
		ln -fs ../.libs/libgv_perl.so gv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.p[lm] .; \
		$(PERL) ./test.pl)

.PHONY: test_php
test_php: libgv_php.la
	-(mkdir -p test_php; cd test_php; \
		ln -fs ../.libs/libgv_php.so libgv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.php .; \
		$(PHP) ./test.php)

.PHONY: test_python3
test_python3: libgv_python3.la
	-(mkdir -p test_python3; cd test_python3; \
		ln -fs ../.libs/libgv_python3.so _gv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.py .; \
		PYTHONPATH=. $(PYTHON3) test.py)

.PHONY: test_R
test_R: libgv_R.la
	-(mkdir -p test_R; cd test_R; \
		ln -fs ../.libs/libgv_R.so gv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.R .; \
		$(R) test.R)

.PHONY: test_ruby
test_ruby: libgv_ruby.la
	-(mkdir -p test_ruby; cd test_ruby; \
		ln -fs ../.libs/libgv_ruby.so gv.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.rb .; \
		$(RUBY) test.rb)

.PHONY: test_tcl
test_tcl: libgv_tcl.la
	-(mkdir -p test_tcl; cd test_tcl; \
		ln -fs ../.libs/libgv_tcl.so libgv_tcl.so; \
		ln -fs ../$(srcdir)/*.gv ../$(srcdir)/*.tcl .; \
		$(TCLSH) ./test.tcl)

EXTRA_DIST = $(man) gv_doc_writer.py

CLEANFILES = test_*/* \
	gv_sharp.cpp $(SHARP_data) \
	gv_go.cpp $(GO_data) \
	gv_guile.cpp $(GUILE_data) \
	gv_java.cpp $(JAVA_data) \
	gv_javascript.cpp \
	gv_lua.cpp $(LUA_data) \
	gv_perl.cpp $(PERL_data) \
	gv_php.cpp $(PHP_data) \
	gv_python.cpp $(PYTHON_data) gv.pyc \
	gv_python3.cpp $(PYTHON3_data) gv.pyc \
	gv_R.cpp $(R_data) \
	gv_ruby.cpp $(RUBY_data) \
	gv_tcl.cpp $(TCL_data)
DISTCLEANFILES = $(pdf_DATA) pkgIndex.tcl