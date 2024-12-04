#ifndef ENERGY_H
#define ENERGY_H
#include <qlc3d.h>
class Simu;
class SolutionVector;
void CalculateFreeEnergy(FILE *fid,
                         int currentIteration,
                         double currentTime,
                         const LC &lc,
                         Geometry *geom,
                         SolutionVector *v,
                         SolutionVector *q);
void closeEnergyFile(FILE *fid, Simu &simu);

void CalculateNodalFreeEnergy(SolutionVector *tiltE,
                              SolutionVector *twistE,
                              SolutionVector *bendE,
                              SolutionVector *elasticE,
                              SolutionVector *thermoE,
                              SolutionVector *electricE,
                              SolutionVector *totalE,
                              int currentIteration,
                              double currentTime,
                              const LC &lc,
                              Geometry *geom,
                              SolutionVector *v,
                              SolutionVector *q);

#endif
