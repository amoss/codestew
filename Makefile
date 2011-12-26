DEBUG=-g
COGENS=cogens/add128
OBJS=build/codestew.o build/SimpleMachine.o build/arm.o build/x86.o build/addProj.o \
     build/commonMain.o
all: ${COGENS} pycodestew.so

build/%.o: src/%.cc Makefile
	g++ ${DEBUG} -c $< -o $@
.PRECIOUS: ${OBJS}

cogens/%: cogens/%.cc ${OBJS}
	g++ ${DEBUG} -Isrc $< ${OBJS} -o $@

pycodestew.so: src/pycodestew.cc src/codestew.cc src/codestew.h src/SimpleMachine.h \
               src/SimpleMachine.cc
	g++ -fPIC -I/usr/include/python2.6 -shared src/pycodestew.cc src/codestew.cc src/SimpleMachine.cc -o pycodestew.so -lpython2.6

clean:
	rm -f ${COGENS} pycodestew.so build/* results/*

#tests: results/add128orig.pdf results/add128x86.pdf results/add128arm.pdf \
#       results/add128x86regs.pdf results/add128armregs.pdf
tests: results/add128-orig.pdf results/add128-arm.pdf results/add128-arm-regs.pdf

results/add128-orig.dot results/add128-orig.asm: cogens/add128
	cogens/add128 results/add128 src
results/add128-arm.dot results/add128-arm-regs.dot results/add128-arm.asm: cogens/add128
	cogens/add128 results/add128 arm


results/%.pdf: results/%.dot
	dot -Tpdf $< -o $@

results/add128x86regs.dot: results/add128x86.dot
results/add128x86.dot: cogens/add128
	cogens/add128 --x86


results/add128x86regs.dot: results/add128x86.dot
results/add128x86.dot: cogens/add128
	cogens/add128 --x86
