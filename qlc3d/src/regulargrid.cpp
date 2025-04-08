
#include <regulargrid.h>
#include <util/logging.h>
#include <util/exception.h>
#include <geom/coordinates.h>
#include <geom/vec3.h>
#include <solutionvector.h>
#include <lc-representation.h>
#include <filesystem>

const idx RegularGrid::NOT_AN_INDEX = std::numeric_limits<idx>::max();
const idx RegularGrid::MAX_SIZE_T = std::numeric_limits<idx>::max();

RegularGrid::RegularGrid() : nx_(0), ny_(0), nz_(0),
                             numRegularPoints_(0),
                             dx_(0), dy_(0), dz_(0)

{
  xLimits_[0] = 0;
  xLimits_[1] = 0;
  yLimits_[0] = 0;
  yLimits_[1] = 0;
  zLimits_[0] = 0;
  zLimits_[1] = 0;
}

RegularGrid::RegularGrid(const RegularGrid &rg) : nx_(rg.nx_), ny_(rg.ny_), nz_(rg.nz_),
                                                  numRegularPoints_(rg.numRegularPoints_),
                                                  dx_(rg.dx_), dy_(rg.dy_), dz_(rg.dz_)
{
  xLimits_[0] = rg.xLimits_[0];
  xLimits_[1] = rg.xLimits_[1];
  yLimits_[0] = rg.yLimits_[0];
  yLimits_[1] = rg.yLimits_[1];
  zLimits_[0] = rg.zLimits_[0];
  zLimits_[1] = rg.zLimits_[1];

  lookupList.insert(lookupList.begin(), rg.lookupList.begin(), rg.lookupList.end());
}

double RegularGrid::getGridX(const unsigned int &xi) const
{
  if (nx_ == 1)
    return 0.5 * (xLimits_[0] + xLimits_[1]);
  else
    return xLimits_[0] + xi * dx_;
}

double RegularGrid::getGridY(const unsigned int &yi) const
{
  if (ny_ == 1)
    return 0.5 * (yLimits_[0] + yLimits_[1]);
  else
    return yLimits_[0] + yi * dy_;
}

double RegularGrid::getGridZ(const unsigned int &zi) const
{
  if (nz_ == 1)
    return 0.5 * (zLimits_[0] + zLimits_[1]);
  else
    return zLimits_[0] + zi * dz_;
}

// CONVERSION FROM LINEAR INDEXING WHICH IS USED TO STORE ALL LOOKUP STRUCTS
// IN A VECTOR TO POSITIONAL INDEXING GINVING THE GRID POINT POSITION IN
// X,Y,Z DIMENSIONS
void RegularGrid::linearToGridIndex(const idx li, idx &xi, idx &yi, idx &zi)
{
#ifdef DEBUG
  assert(li < this->lookupList.size()); // MAKE SURE POINT IS IN LIST
#endif

  // REGULAR GRID POINTS ARE ARRANGED STARTING FORM xmin,ymin,zmin
  // THE INDEX INCREASES FIRST IN X-DIRECTION, THEN Y-DIRECTION AND
  // FINALLY Z-DIRECTION

  idx ppxy = nx_ * ny_; // POINTS PER X-Y PLANE

  zi = li / ppxy; // GET Z-LEVEL

  idx c = li % ppxy; // NUMBER OF POINT IN INCOMPLETE LAYER

  yi = c / nx_;
  xi = c % nx_;
}

// void RegularGrid::gridToLinearIndex(const idx xi, const idx yi, const idx zi)
idx RegularGrid::gridToLinearIndex(const idx xi, const idx yi, const idx zi)
{
  // CALCULATE ARRAY POSITION FROM GRID X,Y AND Z INDEXES xi, yi, zi
#ifdef DEBUG
  assert(xi < nx_);
  assert(yi < ny_);
  assert(zi < nz_);
#endif
  const idx nxyp = nx_ * ny_; // NUMBER OF NODES IN X-Y PLANE

  return xi + yi * nx_ + zi * nxyp;
}

bool RegularGrid::createFromTetMesh(const unsigned int &nx,
                                    const unsigned int &ny,
                                    const unsigned int &nz,
                                    Geometry *geom)
{
  // CREATES INTEPOLATION TABLE FROM A TETRAHEDRAL MESH DESCRIBED BY geom
  // SO THAT FAST INTEPOLATION CAN BE PERFORMED LATER ON
  auto &bounds = geom->getBoundingBox();
  xLimits_[0] = bounds.getXMin();
  xLimits_[1] = bounds.getXMax();
  yLimits_[0] = bounds.getYMin();
  yLimits_[1] = bounds.getYMax();
  zLimits_[0] = bounds.getZMin();
  zLimits_[1] = bounds.getZMax();

  // LIMIT MIN NUMBER OF NODES TO 1 PER DIMENSION
  nx_ = nx == 0 ? 1 : nx;
  ny_ = ny == 0 ? 1 : ny;
  nz_ = nz == 0 ? 1 : nz;

  numRegularPoints_ = nx_ * ny_ * nz_;

  dx_ = (xLimits_[1] - xLimits_[0]) / (nx_ - 1);
  dy_ = (yLimits_[1] - yLimits_[0]) / (ny_ - 1);
  dz_ = (zLimits_[1] - zLimits_[0]) / (nz_ - 1);

  // SPECIAL CASE, WHEN ONLY A SINGLE NODE IN A DIRECTION IS REQUIRED, MAKE dx WHOLE WIDTH OF STRUCTURE
  if (nx_ == 1)
    dx_ = xLimits_[1] - xLimits_[0];
  if (ny_ == 1)
    dy_ = yLimits_[1] - yLimits_[0];
  if (nz_ == 1)
    dz_ = zLimits_[1] - zLimits_[0];

  generateLookupList(geom);
  // validateLookupVector();
  return true;
}

bool RegularGrid::generateLookupList(Geometry *geom)
{
  size_t cc = 0; // coordinate counter
  std::vector<Vec3> gridPoints;

  Log::info("Starting grid creation loop");
  for (unsigned int k = 0; k < nz_; k++)
  { // loop over z
    double z = getGridZ(k);

    for (unsigned int j = 0; j < ny_; j++)
    { // loop over y
      double y = getGridY(j);
      for (unsigned int i = 0; i < nx_; i++, cc++)
      { // loop over x
        double x = getGridX(i);
        gridPoints.emplace_back(x, y, z);
      }
    } // end loop over y
  } // end loop over z
  Log::info("Grid creation loop finished");
  Coordinates gridCoordinates(std::move(gridPoints));

  // GENERATE INDEX TO TETS THAT CONTAIN EARCH REGULAR GRID POINT
  // INDEX WILL HAVE SPECIAL VALUE Geom::NOT_AN_INDEX, IF POITN WAS NOT FOUND
  // THIS MAY HAPPEN WHEN THE UNDERLYING TET MESH IS NOT A CUBE
  std::vector<unsigned int> indT; // index to tet containing a regular coordinate

  geom->genIndToTetsByCoords(indT,
                             gridCoordinates,
                             false,  // do NOT terminate app if a coord is not found
                             false); // do NOT require LC element (although it should be preferred, add this option later)

  // NOW CALCULATE WEIGHTS AND NODE INDEXES FOR EACH REGULAR GRID POINT
  lookupList.clear();
  lookupList.reserve(numRegularPoints_);
  const Mesh &t = geom->getTetrahedra();
  // double* p = geom->getPtrTop();
  for (idx i = 0; i < numRegularPoints_; i++)
  {

    lookup lu;                 // NEW LOOKUP TABLE ENTRY
    lu.type = RegularGrid::OK; // INITIALISE TO GOOD

    if (indT[i] != Geometry::NOT_AN_INDEX)
    { // IF CONTAINING TET ELEMENT WAS FOUND
      Vec3 targetPoint = gridCoordinates.getPoint(i);

      // SET INDEXES TO NEIGHBOURING VERTEXES
      lu.ind[0] = t.getNode(indT[i], 0);
      lu.ind[1] = t.getNode(indT[i], 1);
      lu.ind[2] = t.getNode(indT[i], 2);
      lu.ind[3] = t.getNode(indT[i], 3);

      // CALCULATE NEIGHBOUR NODE WEIGHTS - THESE ARE THE LOCAL COORDINATES
      // OF THE VERTEXES OF THE TET CONTAINING THE REGULAR POINT
      t.calcLocCoords(indT[i], geom->getCoordinates(), targetPoint, &lu.weight[0]);

      if (t.getMaterialNumber(indT[i]) > MAT_DOMAIN7)
      {
        lu.type = RegularGrid::NOT_LC;
      }
    }
    else
    {                                   // CONTAINING TET ELEMENT WAS NOT FOUND
      lu.type = RegularGrid::NOT_FOUND; // containing element not found

      lu.ind[0] = NOT_AN_INDEX;
      lu.ind[1] = NOT_AN_INDEX;
      lu.ind[2] = NOT_AN_INDEX;
      lu.ind[3] = NOT_AN_INDEX;
      lu.weight[0] = 0;
      lu.weight[1] = 0;
      lu.weight[2] = 0;
      lu.weight[3] = 0;
    }
    lookupList.push_back(lu);
  }
  return true;
}
double RegularGrid::interpolateNode(const double *valuesIn,
                                    const RegularGrid::lookup &L) const
{
  // USES PRE-CALCULATED WEIGHTS TO INTERPOLATE A SINGLE VALUE
  // WITHIN A SINGLE TET-ELEMENT

  //
  //  SOMETIMES REGULAR GRID NODES ARE NOT FOUND
  //  (NUMERICAL NOISE, HOLE IN MESH, NON CUBOIDAL MESH etc. )
  //  DEAL WITH IT

  if (L.type == RegularGrid::OK)
  {
    return valuesIn[L.ind[0]] * L.weight[0] +
           valuesIn[L.ind[1]] * L.weight[1] +
           valuesIn[L.ind[2]] * L.weight[2] +
           valuesIn[L.ind[3]] * L.weight[3];
  }
  else
  {
    return std::numeric_limits<double>::quiet_NaN(); // OUTPUTS NaN
  }
}


void RegularGrid::interpolateDirNode(const double *vecin,
                                     double *dirout,
                                     const RegularGrid::lookup &L,
                                     const idx npLC) const
{
// Use (0,1,0) as reference director for global alignment
const double ref[3] = { 0.0, 1.0, 0.0 };

// Load director components for the 4 corner nodes
double n1[3] = { vecin[L.ind[0]], vecin[L.ind[0] + npLC], vecin[L.ind[0] + 2 * npLC] };
double n2[3] = { vecin[L.ind[1]], vecin[L.ind[1] + npLC], vecin[L.ind[1] + 2 * npLC] };
double n3[3] = { vecin[L.ind[2]], vecin[L.ind[2] + npLC], vecin[L.ind[2] + 2 * npLC] };
double n4[3] = { vecin[L.ind[3]], vecin[L.ind[3] + npLC], vecin[L.ind[3] + 2 * npLC] };

// STEP 1: First align all directors with global reference
auto alignWithReference = [&](double n[3]) {
  double dot = n[1]; // Simplified since reference is (0,1,0)    
  if (dot < 0) {
    n[0] = -n[0]; n[1] = -n[1]; n[2] = -n[2];
  }
};

// Align all directors with global reference first
alignWithReference(n1);
alignWithReference(n2);
alignWithReference(n3);
alignWithReference(n4);

// Compute weighted sum
double sum[3] = { 
  n1[0]*L.weight[0] + n2[0]*L.weight[1] + n3[0]*L.weight[2] + n4[0]*L.weight[3],
  n1[1]*L.weight[0] + n2[1]*L.weight[1] + n3[1]*L.weight[2] + n4[1]*L.weight[3],
  n1[2]*L.weight[0] + n2[2]*L.weight[1] + n3[2]*L.weight[2] + n4[2]*L.weight[3]
};

double norm = sqrt(sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]);

  // Normalize result
  dirout[0] = sum[0]/norm;
  dirout[1] = sum[1]/norm;
  dirout[2] = sum[2]/norm;
}


void RegularGrid::interpolateToRegular(const double *valIn,
                                       double *&valOut,
                                       const idx np)
{
  // INTERPOLATES A VARIABLE TO REGULAR GRID
  if (!numRegularPoints_)
  {
    RUNTIME_ERROR("Regular grid is not initialised.");
  }

  for (idx i = 0; i < lookupList.size(); i++)
  {
    lookup L = lookupList[i];

    // If trying to interpolate to a non-LC region
    if ((L.type == RegularGrid::NOT_LC) && (np < MAX_SIZE_T))
    {
      valOut[i] = std::numeric_limits<double>::quiet_NaN(); // OUTPUTS NaN
    }
    else
    {
      valOut[i] = interpolateNode(valIn, L);
    }
  }
}

std::vector<double> RegularGrid::interpolateToRegular(const SolutionVector &pot) const
{
  std::vector<double> regPot;
  for (auto &l : lookupList)
  {
    if (l.type == RegularGrid::OK)
    {
      regPot.push_back(
          pot.getValue(l.ind[0]) * l.weight[0] +
          pot.getValue(l.ind[1]) * l.weight[1] +
          pot.getValue(l.ind[2]) * l.weight[2] +
          pot.getValue(l.ind[3]) * l.weight[3]);
    }
    else
    {
      regPot.push_back(std::numeric_limits<double>::quiet_NaN());
    }
  }
  return regPot;
}

std::vector<double> RegularGrid::interpolateToRegularS(const std::vector<qlc3d::Director> &dir) const
{
  std::vector<double> regS;
  for (auto &l : lookupList)
  {
    if (l.type == RegularGrid::NOT_LC || l.type == RegularGrid::NOT_FOUND)
    {
      regS.push_back(std::numeric_limits<double>::quiet_NaN());
    }
    else
    {
      regS.push_back(
          dir[l.ind[0]].S() * l.weight[0] +
          dir[l.ind[1]].S() * l.weight[1] +
          dir[l.ind[2]].S() * l.weight[2] +
          dir[l.ind[3]].S() * l.weight[3]);
    }
  }
  return regS;
}

std::vector<std::vector<double>> RegularGrid::interpolateToRegularQ(const SolutionVector &q) const 
{
    // Create vector of 5 components for Q tensor
    std::vector<std::vector<double>> regQ(5, std::vector<double>());
    
    // For each component
    for (int comp = 0; comp < 5; comp++) {
        for (auto &l : lookupList) {
            if (l.type == RegularGrid::OK) {
                regQ[comp].push_back(
                    q.getValue(l.ind[0], comp) * l.weight[0] + 
                    q.getValue(l.ind[1], comp) * l.weight[1] + 
                    q.getValue(l.ind[2], comp) * l.weight[2] + 
                    q.getValue(l.ind[3], comp) * l.weight[3]);
            } else {
                regQ[comp].push_back(std::numeric_limits<double>::quiet_NaN());
            }
        }
    }
    return regQ;
}


// Computes the principal eigenvector (v) and eigenvalue (lambda) of a symmetric 3x3 matrix Q.
// Assumes Q is traceless (I1 = 0). Returns true if successful.
bool computePrincipalEigenvector(const double Q[3][3], double v[3], double &lambda)
{
    // For a traceless symmetric 3x3 matrix, I1 = trace(Q)=0.
    // Invariants:
    // I2 = Qxx*Qyy + Qxx*Qzz + Qyy*Qzz - (Qxy^2 + Qxz^2 + Qyz^2)
    // I3 = det(Q)
    double I2 = Q[0][0]*Q[1][1] + Q[0][0]*Q[2][2] + Q[1][1]*Q[2][2]
              - (Q[0][1]*Q[0][1] + Q[0][2]*Q[0][2] + Q[1][2]*Q[1][2]);
    double I3 = Q[0][0]*(Q[1][1]*Q[2][2] - Q[1][2]*Q[1][2])
              - Q[0][1]*(Q[0][1]*Q[2][2] - Q[0][2]*Q[1][2])
              + Q[0][2]*(Q[0][1]*Q[1][2] - Q[0][2]*Q[1][1]);

    // When using the closed-form solution for cubic equations the principal eigenvalue is
    //   lambda_max = 2*sqrt(-I2/3)*cos( theta/3 )
    // where theta = arccos( (3*sqrt(3)*I3)/(2*(-I2/3)^(3/2) ) )
    if (I2 >= 0) {
        // Fall back if invariants are numerically degenerate.
        return false;
    }
    double sqrtTerm = std::sqrt(-I2/3.0);
    double cosArg = (3.0*std::sqrt(3.0)*I3) / (2.0 * pow(sqrtTerm, 3));
    // Clamp cosArg to [-1,1] to avoid numerical error.
    if (cosArg > 1) cosArg = 1;
    if (cosArg < -1) cosArg = -1;
    double theta = std::acos(cosArg);
    lambda = 2.0 * sqrtTerm * std::cos(theta/3.0);

    // Next, solve (Q - lambda I)*v = 0.
    // We form two rows (e.g. row0 and row1) and take their cross-product.
    double r0[3] = { Q[0][0] - lambda, Q[0][1], Q[0][2] };
    double r1[3] = { Q[1][0], Q[1][1] - lambda, Q[1][2] };

    // Cross product
    v[0] = r0[1]*r1[2] - r0[2]*r1[1];
    v[1] = r0[2]*r1[0] - r0[0]*r1[2];
    v[2] = r0[0]*r1[1] - r0[1]*r1[0];
    
    double norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (norm < 1e-10) {
        // If rows were nearly dependent, try with row0 and row2.
        double r2[3] = { Q[2][0], Q[2][1], Q[2][2] - lambda };
        v[0] = r0[1]*r2[2] - r0[2]*r2[1];
        v[1] = r0[2]*r2[0] - r0[0]*r2[2];
        v[2] = r0[0]*r2[1] - r0[1]*r2[0];
        norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        if (norm < 1e-10)
            return false;
    }
    
    v[0] /= norm;
    v[1] /= norm;
    v[2] /= norm;
    return true;
}

std::vector<qlc3d::Director> RegularGrid::interpolateToRegularDirector(const SolutionVector &q) const
{

  std::vector<qlc3d::Director> regDir;
    const double rt2 = std::sqrt(2.0);
    const double rt6 = std::sqrt(6.0);

    // Get the interpolated T tensor values (5 components) on the regular grid.
    std::vector<std::vector<double>> regT = interpolateToRegularQ(q);
    // Total number of grid points
    idx totalPoints = nx_ * ny_ * nz_;

    for (idx i = 0; i < totalPoints; i++) {
        const auto &l = lookupList[i];
        if (l.type == RegularGrid::NOT_LC || l.type == RegularGrid::NOT_FOUND) {
            regDir.push_back(qlc3d::Director({1, 0, 0}, std::numeric_limits<double>::quiet_NaN()));
        } else {
            // Convert interpolated T components to Q components.
            double q1 = -regT[0][i] / rt6 + regT[1][i] / rt2;
            double q2 = -regT[0][i] / rt6 - regT[1][i] / rt2;
            double q3 = regT[2][i] / rt2;
            double q4 = regT[3][i] / rt2;
            double q5 = regT[4][i] / rt2;

            // Construct a QTensor from the computed Q components.
            // (Assuming that qlc3d::QTensor has a constructor of the form QTensor(q1, q2, q3, q4, q5))
            qlc3d::QTensor qt(q1, q2, q3, q4, q5);

            // Convert the QTensor to a Director using your tensortovector function.
            regDir.push_back( tensortovector(qt) );
        }
    }

    // Align all directors with reference n = {0, 1, 0}
    const double ref[3] = { 0.0, 1.0, 0.0 };
    for (auto &d : regDir) {
        double dot = d.nx() * ref[0] + d.ny() * ref[1] + d.nz() * ref[2];
        if (dot < 0) {
            d = qlc3d::Director({ -d.nx(), -d.ny(), -d.nz() }, d.S());
        }
    }
    return regDir;
}
void RegularGrid::interpolateDirToRegular(const double *vecIn,
                                          double *&vecOut,
                                          const idx npLC)
{
  // INTERPOLATES DIRECTOR TO REGULAR GRID.
  // DOES DIRECTOR SWAPPING WITHIN ELEMENT TO MAKE SURE THAT
  // ALL ELEMENT ARE ORIENTED IN SAME(ISH) DIRECTION.
  // THIS IS NECESSARY TO MAINTAIN UNIT LENGTH OF DIRECTOR
  // DIRECTOR COMPONENTS ARE ORDERED AS nx,nx,nx..., ny,ny,ny... nz,nz,nz...
  if (!numRegularPoints_)
  {
    throw std::runtime_error(fmt::format("Regular grid is not intialised in {}, {}", __FILE__, __func__));
  }
  for (idx i = 0; i < numRegularPoints_; i++)
  {
    lookup L = lookupList[i];

    // If LC node
    if (L.type == RegularGrid::OK)
    {
      double dir[3];
      interpolateDirNode(vecIn, dir, L, npLC);
      vecOut[i + 0 * numRegularPoints_] = dir[0];
      vecOut[i + 1 * numRegularPoints_] = dir[1];
      vecOut[i + 2 * numRegularPoints_] = dir[2];
    }
    else
    {
      vecOut[i + 0 * numRegularPoints_] = std::numeric_limits<double>::quiet_NaN();
      vecOut[i + 1 * numRegularPoints_] = std::numeric_limits<double>::quiet_NaN();
      vecOut[i + 2 * numRegularPoints_] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}

/**
 * Writes potential, director and order parameter to a VTK grid, this is modifed to also write all nodal energy contributions. TODO: this should be in part of a separate resul IO
 * file/class
 */
bool RegularGrid::writeVTKGrid(const std::filesystem::path &fileName,
                               const SolutionVector &pot,
                               const std::vector<qlc3d::Director> &dir,
                               const SolutionVector &tiltE,
                               const SolutionVector &twistE,
                               const SolutionVector &bendE,
                               const SolutionVector &elasticE,
                               const SolutionVector &thermoE,
                               const SolutionVector &electricE,
                               const SolutionVector &totalE,
                               const SolutionVector &q)
{

  if (numRegularPoints_ == 0)
  {
    RUNTIME_ERROR("Regular grid is not initialised.");
  }
  std::fstream fid;
  fid.open(fileName, std::fstream::out);

  if (!fid.is_open())
  { // EXIT IF COULDN'T OPEN FILE // TODO: throw exception instead?
    return false;
  }
  std::vector<double> regPot = interpolateToRegular(pot);
  std::vector<double> regTiltE = interpolateToRegular(tiltE);
  std::vector<double> regTwistE = interpolateToRegular(twistE);
  std::vector<double> regBendE = interpolateToRegular(bendE);
  std::vector<double> regElasticE = interpolateToRegular(elasticE);
  std::vector<double> regThermoE = interpolateToRegular(thermoE);
  std::vector<double> regElectricE = interpolateToRegular(electricE);
  std::vector<double> regTotalE = interpolateToRegular(totalE);
  std::vector<double> regS = interpolateToRegularS(dir);
  std::vector<qlc3d::Director> regN = interpolateToRegularDirector(q);

  idx num_points[3] = {nx_, ny_, nz_};
  double grid_spacing[3] = {dx_, dy_, dz_};
  double origin[3] = {getGridX(0), getGridY(0), getGridZ(0)};
  vtkIOFun::writeID(fid);

  vtkIOFun::writeHeader(fid,
                        "header string",
                        vtkIOFun::ASCII,
                        num_points,
                        grid_spacing,
                        origin);

  vtkIOFun::writeScalarData(fid, "Potential", regPot);
  vtkIOFun::writeScalarData(fid, "S", regS);
  vtkIOFun::writeVectorData(fid, "Director", regN);
  vtkIOFun::writeScalarData(fid, "Splay_Energy_Density", regTiltE);
  vtkIOFun::writeScalarData(fid, "Twist_Energy_Density", regTwistE);
  vtkIOFun::writeScalarData(fid, "Bend_Energy_Density", regBendE);
  vtkIOFun::writeScalarData(fid, "Elastic_Energy_Density", regElasticE);
  vtkIOFun::writeScalarData(fid, "Thermotropic_Energy_Density", regThermoE);
  vtkIOFun::writeScalarData(fid, "Electric_Energy_Density", regElectricE);
  vtkIOFun::writeScalarData(fid, "Total_Free_Energy_Density", regTotalE);

  fid.close();
  return true;
}

bool RegularGrid::writeVecMat(const std::filesystem::path &fileName,
                              const SolutionVector &pot,
                              const std::vector<qlc3d::Director> &dir,
                              const SolutionVector &q ,
                              const double time)
{
  // WRITES OUTPUT IN A MATLAB FILE.
  // VALUES ARE WRITTEN IN 2D MATRIXES, WHERE EACH ROW CORRESPONDS TO A
  // COLUMN OF VALUES Zmin->Zmax  IN THE MODELLED STRUCTURE

  std::ofstream fid(fileName);
  if (!fid.good())
  {
    return false;
  }

  fid << "grid_size = [" << nx_ << "," << ny_ << "," << nz_ << "];" << std::endl;
  fid << "current_time = " << time << ";" << std::endl;

  std::vector<double> regPot = interpolateToRegular(pot);
  std::vector<double> regS = interpolateToRegularS(dir);
  std::vector<qlc3d::Director> regN = interpolateToRegularDirector(q);

  std::vector<double> nx, ny, nz;
  nx.resize(regS.size(), 0);
  ny.resize(regS.size(), 0);
  nz.resize(regS.size(), 0);

  for (auto &d : regN)
  {
    nx.push_back(d.nx());
    ny.push_back(d.ny());
    nz.push_back(d.nz());
  }

  MatlabIOFun::writeNumberColumns(fid, "V", regPot, nx_, ny_, nz_);
  MatlabIOFun::writeNumberColumns(fid, "nx", nx, nx_, ny_, nz_);
  MatlabIOFun::writeNumberColumns(fid, "ny", ny, nx_, ny_, nz_);
  MatlabIOFun::writeNumberColumns(fid, "nz", nz, nx_, ny_, nz_);
  MatlabIOFun::writeNumberColumns(fid, "S", regS, nx_, ny_, nz_);

  fid.close();

  return true;
}



bool RegularGrid::writeDirStackZ(const std::filesystem::path &fileName,
                                 const std::vector<qlc3d::Director> &dir,
                                 const SolutionVector &q,
                                 double time)
{
  std::ofstream fid(fileName);
  if (!fid.good())
  {
    return false;
  }

  // First line is grid size
  fid << nx_ << ',' << ny_ << ',' << nz_ << ',' << time << std::endl;

  std::vector<qlc3d::Director> regN = interpolateToRegularDirector(q);

  for (idx y = 0; y < ny_; y++)
  {
    for (idx x = 0; x < nx_; x++)
    {
      for (idx z = 0; z < nz_; z++)
      {
        idx i = gridToLinearIndex(x, y, z);

        if (z > 0)
        {
          fid << ",";
        }

        if (std::isnan(regN[i].S()))
        {
          fid << "NaN,NaN,NaN";
        }
        else
        {
          fid << regN[i].nx() << "," << regN[i].ny() << "," << regN[i].nz();
        }
      }
      fid << std::endl;
    }
  }
  fid.close();
  return true;
}

bool RegularGrid::writeNemaktisDirector(const std::filesystem::path &fileName,
                                        const SolutionVector &q)

{
  std::ofstream fid(fileName);
  if (!fid.good())
  {
    return false;
  }

  // First line is grid sizes (how many points in x, y, z)
  fid << nx_ << ',' << ny_ << ',' << nz_ << std::endl;

  // Second line is dimensions of the grid (Lx, Ly, Lz)
  double Lx = getGridX(nx_ - 1) - getGridX(0);
  double Ly = getGridY(ny_ - 1) - getGridY(0);
  double Lz = getGridZ(nz_ - 1) - getGridZ(0);
  fid << Lx << ',' << Ly << ',' << Lz << std::endl;

  // Grab the regular grid director values
  std::vector<qlc3d::Director> regN = interpolateToRegularDirector(q);

  // Write the director values
  for (idx z = 0; z < nz_; z++)
  {
  for (idx y = 0; y < ny_; y++)
    {
    for (idx x = 0; x < nx_; x++)
      {
        idx i = gridToLinearIndex(x, y, z);

        if (std::isnan(regN[i].S()))
        {
          fid << "NaN,NaN,NaN,NaN,NaN,NaN";
        }
        else
        {
          double coordX = getGridX(x);
          double coordY = getGridY(y);
          double coordZ = getGridZ(z);
          fid << coordX << "," << coordY << "," << coordZ << ","
              << regN[i].nx() << "," << regN[i].ny() << "," << regN[i].nz();
        }
        fid << std::endl;
      }
      
    }
  }

  fid.close();
  return true;
}


bool RegularGrid::writeNemaktisQtensor(const std::filesystem::path& fileName, const SolutionVector& q, double S0) 
{
    // T tensor conversion is needed ap
    const double rt2 = std::sqrt(2.);
    const double rt6 = std::sqrt(6.);
  

    std::ofstream fid(fileName);
    if (!fid.good()) {
        return false;
    }
    // First line: grid sizes
    fid << nx_ << ',' << ny_ << ',' << nz_ << std::endl;

    // Second line: grid dimensions
    double Lx = getGridX(nx_ - 1) - getGridX(0);
    double Ly = getGridY(ny_ - 1) - getGridY(0);
    double Lz = getGridZ(nz_ - 1) - getGridZ(0);
    fid << Lx << ',' << Ly << ',' << Lz << std::endl;

    // Third line
    fid << S0 << std::endl;

    // Get interpolated Q tensor values
    std::vector<std::vector<double>> regT = interpolateToRegularQ(q);
    // Initialize regQ vector with 5 components, each sized for grid points
  std::vector<std::vector<double>> regQ(5, std::vector<double>(nx_ * ny_ * nz_));

  // Convert T components to Q components for each grid point
  for (idx i = 0; i < nx_ * ny_ * nz_; i++) {
      // T to Q conversion
      regQ[0][i] = -regT[0][i]/rt6 + regT[1][i]/rt2;  // q1
      regQ[1][i] = -regT[0][i]/rt6 - regT[1][i]/rt2;  // q2
      regQ[2][i] = regT[2][i]/rt2;                     // q3
      regQ[3][i] = regT[3][i]/rt2;                     // q4
      regQ[4][i] = regT[4][i]/rt2;                     // q5
  }

// Continue with existing file writing code...
    // Write Q tensor values

    // regQ = [
    // [Qxx values for all grid points],
    // [Qyy values for all grid points],
    // [Qxy values for all grid points],
    // [Qyz values for all grid points],
    // [Qxz values for all grid points]
    // ]

    for (idx z = 0; z < nz_; z++) {
        for (idx y = 0; y < ny_; y++) {
            for (idx x = 0; x < nx_; x++) {
                idx i = gridToLinearIndex(x, y, z);

                if (std::isnan(regQ[0][i])) {
                    fid << "NaN,NaN,NaN,NaN,NaN,NaN,NaN,NaN,NaN";
                } else {
                    double coordX = getGridX(x);
                    double coordY = getGridY(y);
                    double coordZ = getGridZ(z);
                    fid << coordX << "," << coordY << "," << coordZ << ","
                        << regQ[0][i] << "," << regQ[1][i] << "," << regQ[2][i] << "," << regQ[3][i] << "," << regQ[4][i];
                }
                fid << std::endl;
            }
        }
    }

    fid.close();
    return true;
}
