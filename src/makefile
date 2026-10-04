CXX = g++

CXXFLAGS = -std=c++11 \
           -I/home/zzy/third_party/cppjieba/include

PARSER_LIBS = -lboost_filesystem \
              -lboost_system

SERVER_LIBS = -ljsoncpp

.PHONY: all
all: parser server

parser: parser.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(PARSER_LIBS)

server: server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SERVER_LIBS)

.PHONY: clean
clean:
	rm -rf parser server