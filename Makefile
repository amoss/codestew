DEBUG=-g
TESTCASES=add64 add128 add256 add512 xor64 xor128 xor256 xor512 mul64 md5
COGENS=$(foreach tc, ${TESTCASES}, build/$(tc))
OBJS=build/codestew.o build/SimpleMachine.o build/arm.o build/x86.o build/addProj.o \
     build/commonMain.o build/vis.o build/isomorphism.o
TIMEOUT=
#TIMEOUT=timeout 20
all: ${COGENS} pycodestew.so

# Special case as no matching header file
build/commonMain.o: src/commonMain.cc src/arm.h src/x86.h src/vis.h src/isomorphism.h Makefile
	g++ ${DEBUG} -c src/commonMain.cc -o build/commonMain.o
build/%.o: src/%.cc src/%.h Makefile
	g++ ${DEBUG} -c $< -o $@

build/%: cogens/%.cc ${OBJS}
	g++ ${DEBUG} -Isrc $< ${OBJS} -o $@

build/isomorphism.o: src/vis.h

PYNAME=python2.7    # Different install on the mac/linux boxes
pycodestew.so: src/pycodestew.cc src/codestew.cc src/codestew.h src/SimpleMachine.h \
               src/SimpleMachine.cc
	g++ -fPIC -I/usr/include/${PYNAME} -shared src/pycodestew.cc src/codestew.cc src/SimpleMachine.cc -o pycodestew.so -l${PYNAME}

clean:
	rm -rf pycodestew.so build/* results/*

DOTS=$(foreach tc, ${TESTCASES}, results/$(tc)-orig.dot results/$(tc)-arm.dot \
                                 results/$(tc)-x86.dot  results/$(tc)-arm-regs.dot \
                                 results/$(tc)-x86-regs.dot)
PDFS=$(foreach tc, ${TESTCASES}, results/$(tc)-orig.pdf results/$(tc)-arm.pdf \
                                 results/$(tc)-x86.pdf  results/$(tc)-arm-regs.pdf \
                                 results/$(tc)-x86-regs.pdf)

tests: ${PDFS}
results/%-orig.dot results/%-orig.asm: build/%
	cp util/error.dot results/$*-orig.dot
	echo "Error" >results/$*-orig.asm
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
.PRECIOUS: ${OBJS} ${DOTS}

results/%.pdf: results/%.dot
	${TIMEOUT} dot -Tpdf $< -o $@ || true

summary: tests
	util/summarise ${TESTCASES} >summary
	cat summary
