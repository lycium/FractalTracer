add_library(TracerLib INTERFACE)
target_include_directories(TracerLib INTERFACE .)
target_sources(TracerLib INTERFACE
    maths/Dual.h
    maths/real.h
    maths/triplex.h
    maths/vec.h

    util/stb_image.h
    util/stb_image_write.h

    renderer/CameraParams.h
    renderer/ColouringFunction.h
    renderer/FormulaFactory.h
    renderer/HDREnvironment.h
    renderer/Material.h
    renderer/Ray.h
    renderer/Renderer.h
    renderer/Scene.h
    renderer/SceneBuilder.h
    renderer/SceneObjectDesc.h
    renderer/SceneParams.h

    scene_objects/AnalyticDEObject.h
    scene_objects/DualDEObject.h
    scene_objects/SceneObject.h
    scene_objects/SimpleObjects.h

    formulas/Amazingbox.h
    formulas/BenesiPine2.h
    formulas/BurningShip4D.h
    formulas/Cubicbulb.h
    formulas/Hopfbrot.h
    formulas/Lambdabulb.h
    formulas/MandalayKIFS.h
    formulas/Mandelbulb.h
    formulas/MengerSponge.h
    formulas/MengerSpongeC.h
    formulas/AmoserSine.h
    formulas/Octopus.h
    formulas/PseudoKleinian.h
    formulas/QuadraticJuliabulb.h
    formulas/RiemannSphere.h
    formulas/SphereTree.h
    )

