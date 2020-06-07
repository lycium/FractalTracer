#FractalTracer: Amazingbox.h AnalyticDEObject.h BenesiPine2.h Cubicbulb.h Dual.h DualDEObject.h main.cpp MandalayKIFS.h Mandelbulb.h MengerSponge.h MengerSpongeC.h Material.h Octopus.h PseudoKleinian.h QuadraticJuliabulb.h Ray.h real.h Renderer.h RiemannSphere.h Scene.h SceneObject.h SimpleObjects.h SphereTree.h stb_image_write.h triplex.h vec.h
	#g++ -std=c++17 -Wall -Wextra -pedantic -O3 -march=native -fopenmp  -o FractalTracer main.cpp

CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3 -march=native -fopenmp
CPPFLAGS = -Imath -Irenderer -Istb_image_write -Iscene_objects -Iformulas

HEADER_FILES = math/*.h renderer/*.h stb_image_write/*.h scene_objects/*.h formulas/*.h

FractalTracer: $(HEADER_FILES) demo/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o FractalTracer demo/main.cpp

clean:
	-rm FractalTracer

.PHONY: clean
