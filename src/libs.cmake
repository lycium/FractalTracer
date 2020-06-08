add_library(TracerLib INTERFACE)
target_include_directories(TracerLib INTERFACE .)
target_sources(TracerLib INTERFACE
    maths/Dual.h
    maths/real.h
    maths/triplex.h
    maths/vec.h

    util/stb_image_write.h

    renderer/Material.h
    renderer/Ray.h
    renderer/Renderer.h
    renderer/Scene.h

    scene_objects/AnalyticDEObject.h
    scene_objects/DualDEObject.h
    scene_objects/SceneObject.h
    scene_objects/SimpleObjects.h

    formulas/Amazingbox.h
    formulas/BenesiPine2.h
    formulas/Cubicbulb.h
    formulas/MandalayKIFS.h
    formulas/Mandelbulb.h
    formulas/MengerSponge.h
    formulas/MengerSpongeC.h
    formulas/Octopus.h
    formulas/PseudoKleinian.h
    formulas/QuadraticJuliabulb.h
    formulas/RiemannSphere.h
    formulas/SphereTree.h
    )

