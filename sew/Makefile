MC = metil_comp -DDEBUG -Wall -g3 -Isrc -Iext/PrepArg/src
SRC = tests/test.met
INSTALL = `pwd`

all: exe

exe: std
	
# 	

std: ext/PrepArg install_dir
	#${MC} -O3 src/stela.cpp ${SRC}
	${MC} src/stela.cpp ${SRC}
	cat out.cpp
# 	metil_comp out.cpp

install:
	sudo cp met.xml /usr/share/kde4/apps/katepart/syntax
	
install_dir:
	export D="#define INSTALL_DIR \"${INSTALL}\""; grep "$$D" src/Stela/System/InstallDir.h || echo "$$D" > src/Stela/System/InstallDir.h

	
lex: ext/PrepArg
	${MC} src/stela.cpp --disp-lexems ${SRC}

gr: ext/PrepArg
	${MC} src/stela.cpp --disp-inst-g ${SRC}
	cat out.cpp

tok: ext/PrepArg
	${MC} src/stela.cpp --disp-tokens ${SRC}
	
gra: ext/PrepArg
	${MC} src/stela.cpp --disp-inst-g-wo-phi ${SRC}
	cat out.cpp

gravg: ext/PrepArg
	${MC} --valgrind src/stela.cpp --disp-inst-g-wo-phi ${SRC}

grag: ext/PrepArg
	${MC} --gdb src/stela.cpp --disp-inst-g-wo-phi ${SRC}


test_%:
	${MC} tests/$@.cpp

vg:
	${MC} --exec-using "valgrind --num-callers=24 --db-attach=yes" src/stela.cpp ${SRC}
	# 	 --leak-check=full

gdb:
	${MC} --gdb src/stela.cpp ${SRC}

ext/PrepArg:
	mkdir -p ext; cd ext; test -e PrepArg || git clone git@github.com:hleclerc/PrepArg.git

gen:
	python bin/generation.py
	# ${MC} bin/gen_base_class_wrapper.cpp met/base.met src/Stela/Vm/BaseStructs.h
	
