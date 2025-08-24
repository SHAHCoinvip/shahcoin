# Copyright (c) 2013-2020 The SHAHCOIN Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

ACLOCAL_AMFLAGS = -I build-aux/m4
SUBDIRS = src
if ENABLE_MAN
SUBDIRS += doc/man
endif

.PHONY: deploy FORCE
.INTERMEDIATE: $(COVERAGE_INFO)

export PYTHONPATH

if BUILD_SHAHCOIN_LIBS
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libshahcoinconsensus.pc
endif

# Binaries
SHAHCOIND_BIN = $(top_builddir)/src/$(SHAHCOIN_DAEMON_NAME)$(EXEEXT)
SHAHCOIN_QT_BIN = $(top_builddir)/src/qt/$(SHAHCOIN_GUI_NAME)$(EXEEXT)
SHAHCOIN_TEST_BIN = $(top_builddir)/src/test/$(SHAHCOIN_TEST_NAME)$(EXEEXT)
SHAHCOIN_CLI_BIN = $(top_builddir)/src/$(SHAHCOIN_CLI_NAME)$(EXEEXT)
SHAHCOIN_TX_BIN = $(top_builddir)/src/$(SHAHCOIN_TX_NAME)$(EXEEXT)
SHAHCOIN_UTIL_BIN = $(top_builddir)/src/$(SHAHCOIN_UTIL_NAME)$(EXEEXT)
SHAHCOIN_WALLET_BIN = $(top_builddir)/src/$(SHAHCOIN_WALLET_TOOL_NAME)$(EXEEXT)
SHAHCOIN_NODE_BIN = $(top_builddir)/src/$(SHAHCOIN_MP_NODE_NAME)$(EXEEXT)
SHAHCOIN_GUI_BIN = $(top_builddir)/src/$(SHAHCOIN_MP_GUI_NAME)$(EXEEXT)
SHAHCOIN_WIN_INSTALLER = $(PACKAGE)-$(PACKAGE_VERSION)-win64-setup$(EXEEXT)

empty :=
space := $(empty) $(empty)

# macOS deployment
OSX_APP = Shahcoin-Qt.app
OSX_VOLNAME = $(subst $(space),-,$(PACKAGE_NAME))
OSX_ZIP = $(OSX_VOLNAME).zip
OSX_DEPLOY_SCRIPT = $(top_srcdir)/contrib/macdeploy/macdeployqtplus
OSX_INSTALLER_ICONS = $(top_srcdir)/src/qt/res/icons/shahcoin.icns
OSX_PLIST = $(top_builddir)/share/qt/Info.plist

# Dist assets
DIST_CONTRIB = \
    $(top_srcdir)/test/sanitizer_suppressions/lsan \
    $(top_srcdir)/test/sanitizer_suppressions/tsan \
    $(top_srcdir)/test/sanitizer_suppressions/ubsan \
    $(top_srcdir)/contrib/linearize/linearize-data.py \
    $(top_srcdir)/contrib/linearize/linearize-hashes.py \
    $(top_srcdir)/contrib/signet/miner

DIST_SHARE = \
    $(top_srcdir)/share/genbuild.sh \
    $(top_srcdir)/share/rpcauth

BIN_CHECKS = $(top_srcdir)/contrib/devtools/symbol-check.py \
              $(top_srcdir)/contrib/devtools/security-check.py \
              $(top_srcdir)/contrib/devtools/utils.py

WINDOWS_PACKAGING = $(top_srcdir)/share/pixmaps/shahcoin.ico \
                     $(top_srcdir)/share/pixmaps/nsis-header.bmp \
                     $(top_srcdir)/share/pixmaps/nsis-wizard.bmp \
                     $(top_srcdir)/doc/README_windows.txt

OSX_PACKAGING = $(OSX_DEPLOY_SCRIPT) $(OSX_INSTALLER_ICONS) \
                 $(top_srcdir)/contrib/macdeploy/detached-sig-create.sh

COVERAGE_INFO = $(COV_TOOL_WRAPPER) baseline.info \
  test_shahcoin_filtered.info total_coverage.info \
  baseline_filtered.info functional_test.info functional_test_filtered.info \
  test_shahcoin_coverage.info test_shahcoin.info fuzz.info fuzz_filtered.info fuzz_coverage.info

dist-hook:
	-$(GIT) archive --format=tar HEAD -- src/clientversion.cpp | $(AMTAR) -C $(top_distdir) -xf -

# Windows deployment
if TARGET_WINDOWS
$(SHAHCOIN_WIN_INSTALLER): all-recursive
	$(MKDIR_P) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIND_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_QT_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_TEST_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_CLI_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_TX_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_WALLET_BIN) $(top_builddir)/release
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM) $(SHAHCOIN_UTIL_BIN) $(top_builddir)/release
	@test -f $(MAKENSIS) && echo 'OutFile "$@"' | cat $(top_builddir)/share/setup.nsi - | $(MAKENSIS) -V2 - || \
	  echo error: could not build $@
	@echo built $@

deploy: $(SHAHCOIN_WIN_INSTALLER)
endif

# macOS deployment
if TARGET_DARWIN
$(OSX_APP)/Contents/PkgInfo:
	$(MKDIR_P) $(@D)
	@echo "APPL????" > $@

$(OSX_APP)/Contents/Resources/empty.lproj:
	$(MKDIR_P) $(@D)
	@touch $@

$(OSX_APP)/Contents/Info.plist: $(OSX_PLIST)
	$(MKDIR_P) $(@D)
	$(INSTALL_DATA) $< $@

$(OSX_APP)/Contents/Resources/shahcoin.icns: $(OSX_INSTALLER_ICONS)
	$(MKDIR_P) $(@D)
	$(INSTALL_DATA) $< $@

$(OSX_APP)/Contents/MacOS/Shahcoin-Qt: all-recursive
	$(MKDIR_P) $(@D)
	STRIPPROG="$(STRIP)" $(INSTALL_STRIP_PROGRAM)  $(SHAHCOIN_QT_BIN) $@

$(OSX_APP)/Contents/Resources/Base.lproj/InfoPlist.strings:
	$(MKDIR_P) $(@D)
	echo '{ CFBundleDisplayName = "$(PACKAGE_NAME)"; CFBundleName = "$(PACKAGE_NAME)"; }' > $@

OSX_APP_BUILT = $(OSX_APP)/Contents/PkgInfo $(OSX_APP)/Contents/Resources/empty.lproj \
  $(OSX_APP)/Contents/Resources/shahcoin.icns $(OSX_APP)/Contents/Info.plist \
  $(OSX_APP)/Contents/MacOS/Shahcoin-Qt $(OSX_APP)/Contents/Resources/Base.lproj/InfoPlist.strings

osx_volname:
	echo $(OSX_VOLNAME) >$@

if BUILD_DARWIN
$(OSX_ZIP): $(OSX_APP_BUILT) $(OSX_PACKAGING)
	$(PYTHON) $(OSX_DEPLOY_SCRIPT) $(OSX_APP) $(OSX_VOLNAME) -translations-dir=$(QT_TRANSLATION_DIR) -zip

deploydir: $(OSX_ZIP)
else !BUILD_DARWIN
APP_DIST_DIR=$(top_builddir)/dist

$(OSX_ZIP): deploydir
	if [ -n "$(SOURCE_DATE_EPOCH)" ]; then find $(APP_DIST_DIR) -exec touch -d @$(SOURCE_DATE_EPOCH) {} +; fi
	cd $(APP_DIST_DIR) && find . | sort | $(ZIP) -X@ $@

$(APP_DIST_DIR)/$(OSX_APP)/Contents/MacOS/Shahcoin-Qt: $(OSX_APP_BUILT) $(OSX_PACKAGING)
	INSTALL_NAME_TOOL=$(INSTALL_NAME_TOOL) OTOOL=$(OTOOL) STRIP=$(STRIP) $(PYTHON) $(OSX_DEPLOY_SCRIPT) $(OSX_APP) $(OSX_VOLNAME) -translations-dir=$(QT_TRANSLATION_DIR)

deploydir: $(APP_DIST_DIR)/$(OSX_APP)/Contents/MacOS/Shahcoin-Qt
endif !BUILD_DARWIN

deploy: $(OSX_ZIP)
endif

# Build rules for each binary
$(SHAHCOIND_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_QT_BIN): FORCE
	$(MAKE) -C src qt/$(@F)

$(SHAHCOIN_CLI_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_TX_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_UTIL_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_WALLET_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_NODE_BIN): FORCE
	$(MAKE) -C src $(@F)

$(SHAHCOIN_GUI_BIN): FORCE
	$(MAKE) -C src $(@F)

# Fuzz and test coverage tools (unchanged from your original file)...

dist_noinst_SCRIPTS = autogen.sh
EXTRA_DIST = $(DIST_SHARE) $(DIST_CONTRIB) $(WINDOWS_PACKAGING) $(OSX_PACKAGING) $(BIN_CHECKS)

EXTRA_DIST += \
    test/functional \
    test/fuzz \
    test/util/test_runner.py \
    test/util/data/*.hex \
    test/util/data/*.json \
    test/util/data/*.py

CLEANFILES = $(OSX_ZIP) $(SHAHCOIN_WIN_INSTALLER)
DISTCHECK_CONFIGURE_FLAGS = --enable-man

doc/doxygen/.stamp: doc/Doxyfile FORCE
	$(MKDIR_P) $(@D)
	$(DOXYGEN) $^
	$(AM_V_at) touch $@

if HAVE_DOXYGEN
docs: doc/doxygen/.stamp
else
docs:
	@echo "error: doxygen not found"
endif

clean-docs:
	rm -rf doc/doxygen

clean-local: clean-docs
	rm -rf coverage_percent.txt test_shahcoin.coverage/ total.coverage/ fuzz.coverage/ test/tmp/ cache/ $(OSX_APP)
	rm -rf test/functional/__pycache__ test/functional/test_framework/__pycache__ test/cache share/rpcauth/__pycache__
	rm -rf osx_volname dist/

test-security-check:
if TARGET_DARWIN
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-security-check.py TestSecurityChecks.test_MACHO
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-symbol-check.py TestSymbolChecks.test_MACHO
endif
if TARGET_WINDOWS
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-security-check.py TestSecurityChecks.test_PE
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-symbol-check.py TestSymbolChecks.test_PE
endif
if TARGET_LINUX
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-security-check.py TestSecurityChecks.test_ELF
	$(AM_V_at) CC='$(CC)' CFLAGS='$(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(PYTHON) $(top_srcdir)/contrib/devtools/test-symbol-check.py TestSymbolChecks.test_ELF
endif
