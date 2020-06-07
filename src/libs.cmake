add_library(maths INTERFACE)
target_include_directories(maths INTERFACE maths)
target_sources(maths INTERFACE
    maths/Dual.h
    maths/real.h
    maths/triplex.h
    maths/vec.h
    )

add_library(util INTERFACE)
target_include_directories(util INTERFACE util)
target_sources(util INTERFACE
    util/stb_image_write.h
    )

add_library(renderer INTERFACE)
target_include_directories(renderer INTERFACE renderer)
target_link_libraries(renderer INTERFACE maths)
target_sources(renderer INTERFACE
    renderer/Material.h
    renderer/Ray.h
    renderer/Renderer.h
    renderer/Scene.h
)

add_library(scene_objects INTERFACE)
target_include_directories(scene_objects INTERFACE scene_objects)
target_sources(scene_objects INTERFACE
    scene_objects/AnalyticDEObject.h
    scene_objects/DualDEObject.h
    scene_objects/SceneObject.h
    scene_objects/SimpleObjects.h
    )

add_library(formulas INTERFACE)
target_include_directories(formulas INTERFACE formulas)
target_link_libraries(formulas INTERFACE maths)
target_sources(formulas INTERFACE
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

