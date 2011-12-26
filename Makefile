DEBUG=-g
COGENS=build/add64 build/add128
OBJS=build/codestew.o build/SimpleMachine.o build/arm.o build/x86.o build/addProj.o \
     build/commonMain.o
all: ${COGENS} pycodestew.so

build/%.o: src/%.cc Makefile
	g++ ${DEBUG} -c $< -o $@
.PRECIOUS: ${OBJS}

build/%: cogens/%.cc ${OBJS}
	g++ ${DEBUG} -Isrc $< ${OBJS} -o $@

pycodestew.so: src/pycodestew.cc src/codestew.cc src/codestew.h src/SimpleMachine.h \
               src/SimpleMachine.cc
	g++ -fPIC -I/usr/include/python2.6 -shared src/pycodestew.cc src/codestew.cc src/SimpleMachine.cc -o pycodestew.so -lpython2.6

clean:
	rm -rf pycodestew.so build/* results/*

tests: results/add64-orig.pdf results/add64-arm.pdf results/add64-arm-regs.pdf \
       results/add64-orig.pdf results/add64-x86.pdf results/add64-x86-regs.pdf \
       results/add128-orig.pdf results/add128-arm.pdf results/add128-arm-regs.pdf \
       results/add128-orig.pdf results/add128-x86.pdf results/add128-x86-regs.pdf

results/%-orig.dot results/%-orig.asm: build/%
	$< $(subst -orig.asm,, $(subst -orig.dot,,$@)) src
results/%-arm.dot results/%-arm-regs.dot results/%-arm.asm: build/add128
	$< $(subst -arm.dot,, $(subst -arm-regs.asm,, $(subst -arm.asm,,$@))) arm
results/%-x86.dot results/%-x86-regs.dot results/%-x86.asm: build/add128
	$< $(subst -x86.dot,, $(subst -x86-regs.asm,, $(subst -x86.asm,,$@))) x86

results/%.pdf: results/%.dot
	dot -Tpdf $< -o $@

