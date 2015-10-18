all:
	swig -c++ -python Parser.i
	python2 setup.py build_ext --inplace
