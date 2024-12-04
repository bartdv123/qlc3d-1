#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H
#include <solutionvector.h>
#include <electrodes.h>
#include <geometry.h>
#include <eventlist.h>
#include <simu.h>
#include <meshrefinement.h>
#include <lc.h>
#include <solver-settings.h>
#include <resultio.h>

namespace SpaMtrix
{
    class IRCMatrix;
}
class SimulationState;
class ResultOutput;
class PotentialSolver;
class SimulationAdaptiveTimeStep;
// CONVENIENCE STRUCT WITH POINTERS TO THE DIFFERENT GEOMETRY OBJECTS
// NEEDED IN MESH REFINEMENT.
struct Geometries
{
    Geometry *geom_orig; // ORIGINAL, LOADED FROM FILE
    Geometry *geom_prev; // FROM PREVIOUS REFINEMENT ITERATION
    Geometry *geom;      // CURRENT, WORKING GEOMETRY
    Geometries() : geom_orig(NULL), geom_prev(NULL), geom(NULL) {}
};

struct SolutionVectors
{
    SolutionVector *q;         // CURRENT Q-TENSOR
    SolutionVector *qn;        // PREVIOUS Q-TENSOR
    SolutionVector *v;         // POTENTIAL
    SolutionVector *tiltE;     // Tilt energy
    SolutionVector *twistE;    // Twist energy
    SolutionVector *bendE;     // Bend energy
    SolutionVector *elasticE;  // Total elastic energy
    SolutionVector *thermoE;   // Thermotropic energy
    SolutionVector *electricE; // Electric energy
    SolutionVector *totalE;    // Total free energy

    SolutionVectors() : q(NULL), qn(NULL), v(NULL), tiltE(NULL), twistE(NULL), bendE(NULL), elasticE(NULL), thermoE(NULL), electricE(NULL), totalE(NULL) {}
};

void handleInitialEvents(SimulationState &simulationState,
                         EventList &eventList,
                         Electrodes &electr,
                         Alignment &alignment,
                         Simu &simu,
                         Geometries &geometries,
                         SolutionVectors &solutionvectors,
                         const LC &lc,
                         SpaMtrix::IRCMatrix &Kq,
                         ResultOutput &resultOutput,
                         PotentialSolver &potentialSolver,
                         SimulationAdaptiveTimeStep &simulationAdaptiveTimeStep);

void handleEvents(EventList &evel,
                  Electrodes &electr,
                  Alignment &alignment,
                  Simu &simu,
                  SimulationState &simulationState,
                  Geometries &geometries,
                  SolutionVectors &solutionvectors,
                  const LC &lc,
                  SpaMtrix::IRCMatrix &Kq,
                  ResultOutput &resultOutput,
                  PotentialSolver &potentialSolver,
                  SimulationAdaptiveTimeStep &adaptiveTimeStep);

/** return true/false depending on whether mesh was refined or not */
bool handleMeshRefinement(std::list<Event *> &refEvents,
                          Geometries &geometries,
                          SolutionVectors &solutionvectors,
                          Simu &simu,
                          SimulationState &simulationState,
                          Alignment &alignment,
                          Electrodes &electrodes,
                          double S0,
                          SpaMtrix::IRCMatrix &Kq);

void handlePreRefinement(std::list<Event *> &refEvents,
                         Geometries &geometries,
                         SolutionVectors &solutionvectors,
                         Simu &simu,
                         SimulationState &simulationState,
                         Alignment &alignment,
                         Electrodes &electrodes,
                         double S0,
                         SpaMtrix::IRCMatrix &Kq);
#endif // EVENTHANDLER_H
