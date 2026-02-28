#pragma once

#include <string>
#include <vector>

#include "renderer/SceneObjectDesc.h"

#include "formulas/AmoserSine.h"
#include "formulas/Amazingbox.h"
#include "formulas/MandalayKIFS.h"
#include "formulas/Lambdabulb.h"
#include "formulas/Octopus.h"
#include "formulas/Cubicbulb.h"
#include "formulas/PseudoKleinian.h"
#include "formulas/RiemannSphere.h"
#include "formulas/BenesiPine2.h"
#include "formulas/MengerSpongeC.h"
#include "formulas/SphereTree.h"


// Returns a new IterationFunction by name, or nullptr for standalone DE formulas
inline IterationFunction * createFormula(const std::string & name)
{
	if (name == "amosersine")     return new DualAmoserSineIteration;
	if (name == "amazingbox")     return new DualAmazingboxIteration;
	if (name == "mandalay")       return new DualMandalayKIFSIteration;
	if (name == "lambdabulb")     return new DualLambdabulbIteration;
	if (name == "octopus")        return new DualOctopusIteration;
	if (name == "cubicbulb")      return new DualCubicbulbIteration;
	if (name == "pseudokleinian") return new DualPseudoKleinianIteration;
	if (name == "riemannsphere")  return new DualRiemannSphereIteration;
	if (name == "benesipine2")    return new DualBenesiPine2Iteration;
	if (name == "mengersponge")   return new DualMengerSpongeCIteration;
	if (name == "spheretree")     return new DualSphereTreeIteration;
	return nullptr;
}

// Returns true if name is a standalone DE formula (no IterationFunction)
inline bool isStandaloneFormula(const std::string & name)
{
	return name == "mandelbulb" || name == "hopfbrot" || name == "burningship4d" || name == "sphere";
}

// Get list of all formula names (for UI combo box)
inline const std::vector<const char *> & getFormulaNames()
{
	static const std::vector<const char *> names = {
		"amosersine", "mandalay", "amazingbox", "amazingbox_mandalay",
		"mandelbulb", "hopfbrot", "burningship4d",
		"lambdabulb", "octopus", "mengersponge", "cubicbulb",
		"pseudokleinian", "riemannsphere", "spheretree", "benesipine2", "sphere"
	};
	return names;
}

// Set up formulas and iteration_sequence for a SceneObjectDesc based on its formula_name.
// Call this when formula_name changes.
inline void setupFormulas(SceneObjectDesc & obj)
{
	// Free existing formulas
	for (auto * f : obj.formulas) delete f;
	obj.formulas.clear();

	if (obj.formula_name == "amazingbox_mandalay")
	{
		obj.formulas.push_back(new DualAmazingboxIteration);
		obj.formulas.push_back(new DualMandalayKIFSIteration);
		obj.iteration_sequence = { 0, 0, 1 };
	}
	else
	{
		IterationFunction * f = createFormula(obj.formula_name);
		if (f)
		{
			obj.formulas.push_back(f);
			obj.iteration_sequence = { 0 };
		}
		else
		{
			// Standalone formula (mandelbulb, hopfbrot, etc.) - no IterationFunction instances
			obj.iteration_sequence.clear();
		}
	}
}
