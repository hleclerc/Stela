MC      = metil_comp -DDEBUG -Wall -g3 -Isrc -Iext/PrepArg/src -Iext/Celo/src
INSTALL = `pwd`
# SRC     = tests/test.met
# MAIN    = stela
	SRC     = --code-for-class "PI8" -o PI8.js tests/test_Ast.met
MAIN    = stela_ast
# tests/test.met

all: cmp

exe: cmp
	./${MAIN} ${SRC}
	cat PI8.js
	nodejs test.js
	
# 	metil_comp out.cpp

cmp: ext/PrepArg install_dir
	${MC} -o ${MAIN} -ne src/${MAIN}.cpp


install:
	sudo cp met.xml /usr/share/kde4/apps/katepart/syntax
	
install_dir:
	export D="#define INSTALL_DIR \"${INSTALL}\""; grep "$$D" src/Stela/System/InstallDir.h || echo "$$D" > src/Stela/System/InstallDir.h

	
lex: ext/PrepArg
	${MC} src/${MAIN}.cpp --disp-lexems ${SRC}

gr: ext/PrepArg
	${MC} src/${MAIN}.cpp --disp-inst-g ${SRC}
	cat out.cpp

tok: ext/PrepArg
	${MC} src/${MAIN}.cpp --disp-tokens ${SRC}
	
gra: ext/PrepArg
	${MC} src/${MAIN}.cpp --disp-inst-g-wo-phi ${SRC}
	cat out.cpp

gravg: ext/PrepArg
	${MC} --valgrind src/${MAIN}.cpp --disp-inst-g-wo-phi ${SRC}

grag: ext/PrepArg
	${MC} --gdb src/${MAIN}.cpp --disp-inst-g-wo-phi ${SRC}


test_%:
	${MC} tests/$@.cpp

gest_%:
	${MC} --gdb tests/test_$*.cpp

vg:
	${MC} --exec-using "valgrind --num-callers=24 --db-attach=yes" src/${MAIN}.cpp ${SRC}
	# 	 --leak-check=full

gdb:
	${MC} --gdb src/${MAIN}.cpp ${SRC}

ext/PrepArg:
	mkdir -p ext; cd ext; test -e PrepArg || git clone git@github.com:hleclerc/PrepArg.git

gen:
	python bin/generation.py
	# ${MC} bin/gen_base_class_wrapper.cpp met/base.met src/Stela/Vm/BaseStructs.h
	
.PHONY: cmp exe 
