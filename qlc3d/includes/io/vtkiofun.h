#ifndef VTKIOFUN_H
#define VTKIOFUN_H
//
//  FUNCTIONS FOR INPUT/OUPUT OF VTK FILES
//

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

class Mesh;
class SolutionVector;
class Coordinates;
namespace qlc3d
{
    class Director;
}

namespace vtkIOFun
{

    extern const char *const ID_STRING;
    enum FileFormat
    {
        ASCII,
        BINARY
    };

    bool writeID(std::fstream &fid); // writes file identifier

    bool writeHeader(std::fstream &fid,
                     const char *headerString,
                     const FileFormat &format,
                     const unsigned int num_points[3],
                     const double grid_spacing[3],
                     const double origin[3]);

    bool writeScalarData(std::fstream &fid,
                         const char *data_name,
                         const std::vector<double> &data);

    bool writeVectorData(std::fstream &fid,
                         const char *data_name,
                         const std::vector<qlc3d::Director> &data);

    /**
     * Writes VTK unstructured grid compatible file. See https://kitware.github.io/vtk-examples/site/VTKFileFormats/
     * for more info about the format.
     */
    class UnstructuredGridWriter
    {
        void writePoints(std::ostream &os, const Coordinates &coordinates) const;
        void writeTetrahedra(std::ostream &os, const Mesh &tetrahedra, size_t numPoints) const;
        void writePotentials(std::ostream &os, size_t numPotentials, const SolutionVector &potentials) const;
        void writeLiquidCrystal(std::ostream &os, size_t numPoints, size_t numLcPoints, const SolutionVector &q) const;
        void writeEnergy(std::ostream &os, size_t numPoints, const std::string &data_name, const SolutionVector &energy) const;

    public:
        void write(const std::filesystem::path &fileName,
                   size_t numLcPoints,
                   const Coordinates &coordinates,
                   const Mesh &tetrahedra,
                   const SolutionVector &potentials,
                   const SolutionVector &q,
                   const SolutionVector &tiltE,
                   const SolutionVector &twistE,
                   const SolutionVector &bendE,
                   const SolutionVector &elasticE,
                   const SolutionVector &thermoE,
                   const SolutionVector &electricE,
                   const SolutionVector &totalE) const;
    };
} // end namespace

#endif // VTKIOFUN_H
