## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

export TAR_OPTIONS = --owner=0 --group=0 --exclude=windows/dependencies/*/*

dist_doc_DATA = AUTHORS COPYING CHANGELOG.md NEWS cpl1.0.txt

pkginclude_HEADERS = $(top_builddir)/graphviz_version.h

dist_man_MANS = graphviz.7

# $(subdirs) contains the list from: AC_CONFIG_SUBDIRS
SUBDIRS = $(subdirs) lib plugin cmd tclpkg doc contrib share graphs tests

.PHONY: doxygen
doxygen:
	doxygen

BUILT_SOURCES = $(top_builddir)/graphviz_version.h

$(top_builddir)/graphviz_version.h: config.h
	printf '#pragma once\n' > $(top_builddir)/graphviz_version.h
	$(EGREP) 'PACKAGE|VERSION|GVPLUGIN' config.h >> $(top_builddir)/graphviz_version.h
	printf '\n/// Graphviz version constants\n///\n' >> $(top_builddir)/graphviz_version.h
	printf '/// These are only accurate for releases, not inter-release Graphviz commits\n' >> $(top_builddir)/graphviz_version.h
	printf '/// built from source.\n' >> $(top_builddir)/graphviz_version.h
	printf '#define GRAPHVIZ_VERSION_MAJOR @GRAPHVIZ_VERSION_MAJOR@\n' >> $(top_builddir)/graphviz_version.h
	printf '#define GRAPHVIZ_VERSION_MINOR @GRAPHVIZ_VERSION_MINOR@\n' >> $(top_builddir)/graphviz_version.h
	printf '#define GRAPHVIZ_VERSION_PATCH @GRAPHVIZ_VERSION_PATCH@\n' >> $(top_builddir)/graphviz_version.h
	printf '\n/// threading library Graphviz was built with and uses internally\n' >> $(top_builddir)/graphviz_version.h
	printf '///   0 – none; internal operations assume your program is single-threaded\n' >> $(top_builddir)/graphviz_version.h
	printf '///   43 – C++ threads\n' >> $(top_builddir)/graphviz_version.h
	printf '///   99 – C11 threads\n' >> $(top_builddir)/graphviz_version.h
	printf '///   112 – POSIX threads\n' >> $(top_builddir)/graphviz_version.h
	printf '#define GRAPHVIZ_THREADS @GRAPHVIZ_THREADS@\n' >> $(top_builddir)/graphviz_version.h

EXTRA_DIST = graphviz.spec \
	config/depcomp \
	builddate.h \
	m4/README \
	Doxyfile.in Doxyfile \
	config/config_perl.pl \
	config/config_ruby.rb dot.demo plugin.demo \
	macosx windows debian redhat

.PHONY: rpm
rpm: dist
	mkdir -p $(HOME)/rpmbuild/$(HOSTNAME)/SPECS $(HOME)/rpmbuild/$(HOSTNAME)/SOURCES
	cp graphviz-$(VERSION).tar.gz $(HOME)/rpmbuild/$(HOSTNAME)/SOURCES/
	case `cut -d' ' -f1 /etc/system-release` in \
	Fedora) cp redhat/graphviz.spec.fedora $(HOME)/rpmbuild/$(HOSTNAME)/SPECS/graphviz.spec;; \
	CentOS) cp redhat/graphviz.spec.rhel $(HOME)/rpmbuild/$(HOSTNAME)/SPECS/graphviz.spec;; \
	Red) cp redhat/graphviz.spec.rhel $(HOME)/rpmbuild/$(HOSTNAME)/SPECS/graphviz.spec;; \
	esac
	rpmbuild -ba $(HOME)/rpmbuild/$(HOSTNAME)/SPECS/graphviz.spec

.PHONY: deb
deb: dist
	rm -rf graphviz-$(VERSION)
	zcat graphviz-$(VERSION).tar.gz | tar xf -
	(cd graphviz-$(VERSION); fakeroot make -f debian/rules binary)

.PHONY: pkg
pkg: dist
	(dir=`mktemp -d` ;\
 tar xzf graphviz-$(VERSION).tar.gz -C $${dir} --strip-components 1 ;\
 make -C $${dir}/macosx/build PREFIX=${prefix} ;\
 mv $${dir}/macosx/build/graphviz-*.pkg . ;\
 rm -rf $${dir}\
)

DISTCLEANFILES = config.cache builddate.h FEATURE/* graphviz-$(VERSION)* graphviz-*.pkg
