DEBUG=-g
COGENS=cogens/xor cogens/add128
all: ${COGENS} pycodestew.so
clean:
	rm -f *.o ${COGENS} pycodestew.so

codestew.o: codestew.cc codestew.h Makefile
	g++ ${DEBUG} -c codestew.cc 
SimpleMachine.o: SimpleMachine.cc SimpleMachine.h Makefile
	g++ ${DEBUG} -c SimpleMachine.cc 
arm.o: arm.cc arm.h Makefile
	g++ ${DEBUG} -c arm.cc 
x86.o: x86.cc x86.h Makefile
	g++ ${DEBUG} -c x86.cc 

cogens/%: cogens/%.cc codestew.o SimpleMachine.o arm.o x86.o
	g++ ${DEBUG} -I. $< SimpleMachine.o codestew.o arm.o x86.o -o $@

pycodestew.so: pycodestew.cc codestew.cc codestew.h SimpleMachine.h SimpleMachine.cc
	g++ -fPIC -I/usr/include/python2.6 -shared pycodestew.cc codestew.cc SimpleMachine.cc -o pycodestew.so -lpython2.6
