#include <catch.h>
#include <io/vtkiofun.h>
#include <mesh.h>
#include <test-util.h>
#include <solutionvector.h>
#include <lc-representation.h>
#include <geom/coordinates.h>
#include <geom/vec3.h>

TEST_CASE("write VTK unstructured ascii grid")
{
    using namespace vtkIOFun;
    using namespace std;

    // ARRANGE:
    // Set-up create dummy data consisting of a single 4-node tetrahedral mesh
    // and define some potential values and director vectors for each of the
    // mesh node values

    size_t numPoints = 4;
    size_t numLcPoints = 4;

    // 4 mesh node coordinates
    Coordinates coordinates(
        {{0, 0, 0},
         {1, 0, 0},
         {0, 1, 0},
         {0, 0, 1}});

    // 4 directors
    SolutionVector q(4, 5);

    q.setValue(0, qlc3d::Director(1, 0, 0, 0.1));
    q.setValue(1, qlc3d::Director(0, 1, 0, 0.2));
    q.setValue(2, qlc3d::Director(0, 0, 1, 0.3));

    qlc3d::Director d = qlc3d::Director::fromDegreeAngles(45, 45, 0.4);
    q.setValue(3, d);

    SolutionVector potentials(4, 1);
    potentials[0] = 0;
    potentials[1] = 0.1;
    potentials[2] = 0.2;
    potentials[3] = 0.3;

    // create mesh consisting of a single tetrahedron
    Mesh tetrahedra(3, 4);
    tetrahedra.setElementData({0, 1, 2, 3}, {4});
}
