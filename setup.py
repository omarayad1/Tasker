from distutils.core import setup, Extension

extension_mod = [Extension("_Parser",["Parser_wrap.cxx", "Parser.cpp"])]

setup(name = "tasker", ext_modules=extension_mod)