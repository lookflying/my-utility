SUB_MAKEFILES=$(shell find . -mindepth 2 -name Makefile)
SUBDIR=$(dir ${SUB_MAKEFILES})
CLEANDIR=${SUBDIR:%=clean-%}
all: ${SUBDIR}
${SUBDIR}:
	${MAKE} -C $@

clean: ${CLEANDIR}
${CLEANDIR}:
	${MAKE} -C ${@:clean-%=%} clean

.PHONY: all ${SUBDIR}
.PHONY: clean ${CLEANDIR}
#${SUBDIR}:
#		${MAKE} -C $@
#
#clean:	${CLEANDIR}
#
#${CLEANDIR}:
#		${MAKE} -C $@ clean
#
#
#.PHONY: all ${SUBDIR} ${CLEANDIR} clean
