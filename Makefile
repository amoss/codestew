pycodestew.so: pycodestew.cc codestew.cc codestew.h SimpleMachine.h SimpleMachine.cc
	g++ -fPIC -I/usr/include/python2.6 -shared pycodestew.cc codestew.cc SimpleMachine.cc -o pycodestew.so -lpython2.6
