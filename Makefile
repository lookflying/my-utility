SUB_MAKEFILES=$(shell find . -mindepth 2 -name Makefile)
SUBDIR=$(dir ${SUB_MAKEFILES})
CLEANDIR=${SUBDIR:%=clean-%}
INSTALLDIR=${SUBDIR:%=install-%}
UNINSTALLDIR=${SUBDIR:%=uninstall-%}
all: ${SUBDIR}
${SUBDIR}:
	${MAKE} -C $@

clean: ${CLEANDIR}
${CLEANDIR}:
	${MAKE} -C ${@:clean-%=%} clean

install: ${INSTALLDIR}
${INSTALLDIR}:
	${MAKE} -C ${@:install-%=%} install

uninstall: ${UNINSTALLDIR}
${UNINSTALLDIR}:
	${MAKE} -C ${@:uninstall-%=%} uninstall

.PHONY: all ${SUBDIR}
.PHONY: clean ${CLEANDIR}
.PHONY: install ${INSTALLDIR}
.PHONY: uninstall ${UNINSTALLDIR}
