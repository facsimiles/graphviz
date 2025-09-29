# tools

MAKE=make
XCODEBUILD=xcodebuild
PRODUCTBUILD=productbuild
PKGBUILD=pkgbuild

# prefix

PREFIX=/usr/local/graphviz
ifeq "$(PREFIX)" "/Applications/Graphviz.app/Contents/Frameworks"
  RPATH=@executable_path/../Frameworks/lib
else
  RPATH="$(PREFIX)/lib"
endif

# base

GV_DIR=../..
DOT=dot

UNAME_M:=$(shell uname -m)
ARCH=-arch $(UNAME_M)

#
# Graphviz and App installer package
#
# Note: the check function in Distribution.xml specifies the minimum macOS
# version, which should match the Graphviz.app project's deployment target.
#
# Note: the Component.plist file specifies BundleIsRelocatable = false,
# which forces the app bundle into /Applications. Otherwise, the macOS
# installer, finding the Release build of Graphviz.app registered and
# acceptable, DOES NOT install Graphviz.app into /Applications, and WORSE
# sets the Release build app's user/group to root/wheel!! Subsequently,
# cleanup requires sudo to remove the Release build >:( 
#

graphviz-$(UNAME_M).pkg: Distribution.xml app.pkg dot.pkg
	@echo
	@echo PACKAGING GRAPHVIZ...
	@echo
	"$(PRODUCTBUILD)" --package-path . --distribution $< $@

app.pkg: Component.plist Release/Graphviz.app
	@echo
	@echo PACKAGING APP...
	@echo
	rm -rf Release/Graphviz.app.dSYM
	"$(PKGBUILD)" --root Release --install-location /Applications --identifier com.att.graphviz.app --component-plist $< $@

dot.pkg: .$(PREFIX)/bin/$(DOT) Scripts/postinstall
	@echo
	@echo PACKAGING DOT...
	@echo
	"$(PKGBUILD)" --root .$(PREFIX) --install-location $(PREFIX) --scripts Scripts --identifier com.att.graphviz $@

.$(PREFIX)/bin/$(DOT): $(GV_DIR)/cmd/dot/.libs/$(DOT)
	@echo
	@echo INSTALLING GRAPHVIZ...
	@echo
	$(MAKE) DESTDIR=$(CURDIR) -C $(GV_DIR) install-strip
	rm -rf .$(PREFIX)/lib/*.la
	rm -rf .$(PREFIX)/lib/graphviz/*.la
	rm -rf .$(PREFIX)/lib/graphviz/tcl/libtcldot_builtin.*
	cd .$(PREFIX)/bin; find . -type f -maxdepth 1 | while read a;do dyld_info -linked_dylibs $$a | sed -n -E "s|($(PREFIX))(.*)|\1\2 @executable_path/..\2 $$a|p";done | xargs -t -L1 install_name_tool -change
	cd .$(PREFIX)/lib; find . -type f -maxdepth 1 | while read a;do dyld_info -linked_dylibs $$a | sed -n -E "s|($(PREFIX)/lib)(.*)|\1\2 @loader_path\2 $$a|p";done | xargs -t -L1 install_name_tool -change
	cd .$(PREFIX)/lib/graphviz; find . -type f -maxdepth 1 | while read a;do dyld_info -linked_dylibs $$a | sed -n -E "s|($(PREFIX)/lib)(.*)|\1\2 @loader_path/..\2 $$a|p";done | xargs -t -L1 install_name_tool -change

Release/Graphviz.app: ../*.m ../*.h $(GV_DIR)/cmd/dot/.libs/$(DOT)
	@echo
	@echo BUILDING GRAPHVIZ GUI...
	@echo
	"$(XCODEBUILD)" -project ../graphviz.xcodeproj -configuration Release VALID_ARCHS=$(UNAME_M) ALWAYS_SEARCH_USER_PATHS=NO MACOSX_DEPLOYMENT_TARGET=10.13 LD_RUNPATH_SEARCH_PATHS=$(RPATH)
	cd $@/Contents/MacOS; dyld_info -linked_dylibs Graphviz | sed -n -E "s|($(PREFIX)/lib)(.*)|\1\2 @rpath\2 Graphviz|p" | xargs -t -L1 install_name_tool -change
	codesign -s "-" -fv $@/Contents/MacOS/Graphviz

Scripts/postinstall:
	@echo
	@echo SCRIPTING POSTINSTALL...
	@echo
	mkdir -p $(@D)
	echo '#!/bin/sh' >$@
	echo 'logger -is -t "Graphviz Install" "register dot plugins"' >>$@
	echo '"$(PREFIX)/bin/dot" -c' >>$@
ifeq "$(PREFIX)" "/usr/local"
	echo 'rm -f /etc/paths.d/graphviz' >>Scripts/postinstall
else
	echo 'echo "$(PREFIX)/bin" >/etc/paths.d/graphviz' >>Scripts/postinstall
endif
	chmod 755 $@

$(GV_DIR)/cmd/dot/.libs/$(DOT): $(GV_DIR)/Makefile
	@echo
	@echo MAKING GRAPHVIZ...
	@echo
	if [[ -e $@ ]]; then $(MAKE) -C $(GV_DIR) clean; fi
	$(MAKE) -C $(GV_DIR)/tclpkg/tclstubs
	$(MAKE) -C $(GV_DIR)
	
$(GV_DIR)/Makefile Distribution.xml: $(GV_DIR)/configure Distribution.xml.in
	@echo
	@echo CONFIGURING GRAPHVIZ...
	@echo
	cd $(GV_DIR) && ./configure --prefix=$(PREFIX) CFLAGS="-O2 $(ARCH)" CXXFLAGS="-O2 $(ARCH)" OBJCFLAGS="-O2 $(ARCH)" LDFLAGS="$(ARCH) -Wl,-dead_strip"

.PHONY: clean
clean:
	rm -rf Distribution.xml graphviz.build Scripts Release SharedPrecompiledHeaders XCBuildData .$(PREFIX) *.pkg
	$(MAKE) -C $(GV_DIR) clean

.PHONY: distclean
distclean: clean
	$(MAKE) -C $(GV_DIR) distclean
