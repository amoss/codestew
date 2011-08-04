pycodestew.so: pycodestew.cc codestew.cc codestew.h
	g++ -fPIC -I/usr/include/python2.6 -shared pycodestew.cc codestew.cc -o build/lib.linux-x86_64-2.6/pycodestew.so -lpython2.6
