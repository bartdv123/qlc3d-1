#ifndef PROJECT_QLC3D_RESULT_OUTPUT_H
#define PROJECT_QLC3D_RESULT_OUTPUT_H

#include <string>
#include <vector>
#include <set>
#include <simu.h>
#include <memory>
#include <filesystem>

// forward declarations
class SolutionVector;
class Geometry;
class SimulationState;
namespace qlc3d
{
  struct Director;
}

/**
 * Abstract base class for result output formats. Each output format should implement this interface.
 */
class ResultFormatWriter
{
protected:
  const std::vector<qlc3d::Director> *directors = nullptr;
  const SolutionVector *qTensor = nullptr;
  const SolutionVector *potential = nullptr;
  // add the nodal energy SolutionVectors
  const SolutionVector *tiltE = nullptr;
  const SolutionVector *twistE = nullptr;
  const SolutionVector *bendE = nullptr;
  const SolutionVector *elasticE = nullptr;
  const SolutionVector *thermoE = nullptr;
  const SolutionVector *electricE = nullptr;
  const SolutionVector *totalE = nullptr;
  const std::filesystem::path outputDirectory;

  std::string static iterationAsString(const SimulationState &simulationState);

public:
  explicit ResultFormatWriter(const std::filesystem::path &outputDirectory) : outputDirectory(outputDirectory) {};
  [[nodiscard]] virtual bool isDirectorRequired() const { return false; };
  [[nodiscard]] virtual bool isRegularGridRequired() const { return false; };

  void setDirector(const std::vector<qlc3d::Director> *dir) { this->directors = dir; };
  void setQTensor(const SolutionVector &q) { this->qTensor = &q; };
  void setPotential(const SolutionVector &pot) { this->potential = &pot; };
  void setTiltE(const SolutionVector &tiltE) { this->tiltE = &tiltE; };
  void setTwistE(const SolutionVector &twistE) { this->twistE = &twistE; };
  void setBendE(const SolutionVector &bendE) { this->bendE = &bendE; };
  void setElasticE(const SolutionVector &elasticE) { this->elasticE = &elasticE; };
  void setThermoE(const SolutionVector &thermoE) { this->thermoE = &thermoE; };
  void setElectricE(const SolutionVector &electricE) { this->electricE = &electricE; };
  void setTotalE(const SolutionVector &totalE) { this->totalE = &totalE; };

  /**
   * write the result to the output format.
   * TODO: would be nice to return a result that can contain useful data like created result file name etc.
   */
  virtual void writeResult(const Geometry &geom, const SimulationState &simulationState) = 0;
  virtual const std::string formatName() const = 0;
  virtual ~ResultFormatWriter() = default;
};

class ResultOutput
{
public:
  ResultOutput(const std::set<Simu::SaveFormats> &saveFormats, const std::string &meshName, double S0, const std::filesystem::path &outputDir);
  void writeResults(const Geometry &geom, const SolutionVector &potential, const SolutionVector &qtensor, const SolutionVector &tiltE, const SolutionVector &twistE, const SolutionVector &bendE, const SolutionVector &elasticE, const SolutionVector &thermoE, const SolutionVector &electricE, const SolutionVector &totalE, const SimulationState &simulationState);
  [[nodiscard]] bool isRegularGridRequired() const;

private:
  std::vector<std::shared_ptr<ResultFormatWriter>> outputFormatWriters_;
  /** output directory, relative to the application working directory */
  std::string outputDirectory_;

  [[nodiscard]] bool isDirectorRequired() const;
};

class RegularVTKFormatWriter : public ResultFormatWriter
{
public:
  explicit RegularVTKFormatWriter(const std::filesystem::path &outputDir) : ResultFormatWriter(outputDir) {};
  [[nodiscard]] bool isDirectorRequired() const override { return true; };
  [[nodiscard]] bool isRegularGridRequired() const override { return true; };
  [[nodiscard]] const std::string formatName() const override { return "RegularVTK"; };
  void writeResult(const Geometry &geom, const SimulationState &simulationState) override;
  ~RegularVTKFormatWriter() override = default;
};

class RegularVecMatFormatWriter : public ResultFormatWriter
{
public:
  explicit RegularVecMatFormatWriter(const std::filesystem::path &outputDir) : ResultFormatWriter(outputDir) {};
  [[nodiscard]] bool isDirectorRequired() const override { return true; };
  [[nodiscard]] bool isRegularGridRequired() const override { return true; };
  [[nodiscard]] const std::string formatName() const override { return "RegularVecMat"; };
  void writeResult(const Geometry &geom, const SimulationState &simulationState) override;
  ~RegularVecMatFormatWriter() override = default;
};

class DirStackZFormatWriter : public ResultFormatWriter
{
public:
  explicit DirStackZFormatWriter(const std::filesystem::path &outputDir) : ResultFormatWriter(outputDir) {};
  [[nodiscard]] bool isDirectorRequired() const override { return true; };
  [[nodiscard]] bool isRegularGridRequired() const override { return true; };
  [[nodiscard]] const std::string formatName() const override { return "DirStackZ"; };
  void writeResult(const Geometry &geom, const SimulationState &simulationState) override;
  ~DirStackZFormatWriter() override = default;
};

class CsvUnstructuredFormatWriter : public ResultFormatWriter
{
public:
  explicit CsvUnstructuredFormatWriter(const std::filesystem::path &outputDir) : ResultFormatWriter(outputDir) {};
  [[nodiscard]] const std::string formatName() const override { return "CsvUnstructured"; };
  void writeResult(const Geometry &geom, const SimulationState &simulationState) override;
  ~CsvUnstructuredFormatWriter() override = default;
};

class VtkUnstructuredAsciiGridFormatWriter : public ResultFormatWriter
{
public:
  explicit VtkUnstructuredAsciiGridFormatWriter(const std::filesystem::path &outputDir) : ResultFormatWriter(outputDir) {};
  [[nodiscard]] const std::string formatName() const override { return "VtkUnstructuredAsciiGrid"; };
  void writeResult(const Geometry &geom, const SimulationState &simulationState) override;
  ~VtkUnstructuredAsciiGridFormatWriter() override = default;
};

#endif // PROJECT_QLC3D_RESULT_OUTPUT_H
