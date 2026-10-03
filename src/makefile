CXX = "g++"
CXXFLAGS = -std=c++11 \
           -I/home/zzy/third_party/cppjieba/include \
		   -lboost_filesystem \
		   -lboost_system

parser:parser.cpp
	${CXX} ${CXXFLAGS} -o parser parser.cpp  


.PHONY: clean
clean:
	rm -rf parser

