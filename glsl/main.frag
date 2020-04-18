#version 460 compatibility
/*
Main entry point.
*/

#include "Common.frag"

#include "MathUtils.frag"
#define providesColor
#define providesNormal
#include "DE-Raytracer.frag" 

// animation
uniform float time;
float co = cos(float(M_2PI) * time / 4);
float si = sin(float(M_2PI) * time / 4);
mat3 Rot = mat3(co, si, 0.0, -si, co, 0.0, 0.0, 0.0, 1.0);

// formula definition

void formula(inout Vec3Dual3f z, in Vec3Dual3f c)
{
  MandelbulbTriplex(z, c);
  MengerSponge(z, float(3));
}

void formula(inout Vec3Dual3fx z, in Vec3Dual3fx c)
{
  MandelbulbTriplex(z, c);
  MengerSponge(z, floatx(3));
}

// interface to `DE-Raytracer.frag`

float DE(vec3 pos) {
  vec3 normalV;
  return DistanceEstimate(Rot * pos, normalV);
}

#ifdef providesNormal
vec3 normal(vec3 pos, float d)
{
  vec3 normalV;
  float de = DistanceEstimate(Rot * pos, normalV);
  return normalize(transpose(Rot) * normalV);
}
#endif

#ifdef providesColor
vec3 baseColor(vec3 point, vec3 normal)
{
  return vec3(0.5) + 0.5 * normalize(Rot * point + transpose(Rot) * normal);
}
#endif

// presets

#preset Default
AValue = 3
PValue = 8
FOV = 0.3
Eye = 1,3,2
Target = 0,0,0
Up = 0,0,1
MaxRaySteps = 1000
Detail = -4
OrbitStrength = 1
BackgroundColor = 0.2,0.2,0.2
CamLight = 1,1,1,0
SpotLightDir = 0.3,0.2
HardShadow = 1
#endpreset
