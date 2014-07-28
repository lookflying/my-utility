SUB_MAKEFILES=$(shell find . -mindepth 2 -name Makefile)
SUBDIR=$(dir ${SUB_MAKEFILES})
#all: ${SUBDIR}
#	@echo ${SUBDIR}
	
${SUBDIR}:
		${MAKE} -C $@


