CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3 -march=native -fopenmp
CPPFLAGS = -Iformulas -Imaths -Irenderer -Iscene_objects -Iutil

HEADER_FILES = formulas/*.h maths/*.h renderer/*.h scene_objects/*.h util/*.h

FractalTracer: $(HEADER_FILES) demo/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o FractalTracer demo/main.cpp

clean:
	-rm FractalTracer

.PHONY: clean
