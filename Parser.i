%module Parser

%{
#include "Parser.h"
%}
%include "std_vector.i"
%include "std_string.i"
namespace std {
	%template(IntVector) vector<int>;
	%template(ProcVector) vector<process>;
	%template(CpuVector) vector<cpu>;
	%template(ThreadVector) vector<thread>;
	%template(CpuDataVector) vector<cpu_data>;
	%template(ProcDataVector) vector<process_data>;


}
%include "Parser.h"