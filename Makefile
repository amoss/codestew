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
	cp util/error.dot results/$*-orig.dot
	cp util/error.dot results/$*-arm-regs.dot
	echo "Error" >results/$*-arm.asm
	-$< results/$* src >results/$*-orig.log
results/%-arm.dot results/%-arm-regs.dot results/%-arm.asm: build/%
	cp util/error.dot results/$*-arm.dot
	cp util/error.dot results/$*-arm-regs.dot
	echo "Error" >results/$*-arm.asm
	-$< results/$* arm >results/$*-arm.log
results/%-x86.dot results/%-x86-regs.dot results/%-x86.asm: build/%
	cp util/error.dot results/$*-x86.dot
	cp util/error.dot results/$*-x86-regs.dot
	echo "Error" >results/$*-x86.asm
	-$< results/$* x86 >results/$*-x86.log

results/%.pdf: results/%.dot
	dot -Tpdf $< -o $@

