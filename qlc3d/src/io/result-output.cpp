#include <io/result-output.h>
#include <io/lcview-result-output.h>
#include <geometry.h>
#include <solutionvector.h>
#include <filesystem>
#include <simulation-state.h>
#include <qlc3d.h>
#include <resultio.h>
#include <simu.h>
#include <util/logging.h>
#include <lc-representation.h>
#include "util/exception.h"

namespace fs = std::filesystem;

//<editor-fold desc="ResultOutput">
ResultOutput::ResultOutput(const std::set<Simu::SaveFormats> &saveFormats,
                           const std::string &meshName,
                           double S0,
                           const std::filesystem::path &outputDir)
{
  outputFormatWriters_ = std::vector<std::shared_ptr<ResultFormatWriter>>();

  for (const auto &s : saveFormats)
  {
    if (s == Simu::SaveFormats::LCview)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new LcViewBinaryResultFormatWriter(outputDir, meshName, S0)));
    }
    else if (s == Simu::SaveFormats::LCviewTXT)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new LcViewTxtResultFormatWriter(outputDir, meshName, S0)));
    }
    else if (s == Simu::SaveFormats::CsvUnstructured)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new CsvUnstructuredFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::DirStackZ)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new DirStackZFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::RegularVTK)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new RegularVTKFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::RegularVecMat)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new RegularVecMatFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::VTKUnstructuredAsciiGrid)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new VtkUnstructuredAsciiGridFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::RegularNemaktisDirector)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new RegularNemaktisDirectorFormatWriter(outputDir)));
    }
    else if (s == Simu::SaveFormats::RegularNemaktisQTensor)
    {
      outputFormatWriters_.push_back(std::shared_ptr<ResultFormatWriter>(new RegularNemaktisQTensorFormatWriter(outputDir, S0)));
    }
  
    else
    {
      RUNTIME_ERROR("Unknown save format: " + std::to_string(s));
    }
  }
}

void ResultOutput::writeResults(const Geometry &geom,
                                const SolutionVector &potential,
                                const SolutionVector &qtensor,
                                const SolutionVector &tiltE,
                                const SolutionVector &twistE,
                                const SolutionVector &bendE,
                                const SolutionVector &elasticE,
                                const SolutionVector &thermoE,
                                const SolutionVector &electricE,
                                const SolutionVector &totalE,
                                const SimulationState &simulationState,
                                double S0)
{
  // if any of current output format writers requires director, calculate director
  std::vector<qlc3d::Director> directors;
  if (isDirectorRequired())
  {
    directors = qtensor.getDirector();
  }

  for (auto outputFormatWriter : outputFormatWriters_)
  {
    if (outputFormatWriter->isDirectorRequired())
    {
      outputFormatWriter->setDirector(&directors);
    }
    outputFormatWriter->setQTensor(qtensor);
    outputFormatWriter->setPotential(potential);
    outputFormatWriter->setTiltE(tiltE);
    outputFormatWriter->setTwistE(twistE);
    outputFormatWriter->setBendE(bendE);
    outputFormatWriter->setElasticE(elasticE);
    outputFormatWriter->setThermoE(thermoE);
    outputFormatWriter->setElectricE(electricE);
    outputFormatWriter->setTotalE(totalE);
    Log::info("Writing result with format {}", outputFormatWriter->formatName());
    outputFormatWriter->writeResult(geom, simulationState);
  }
}

bool ResultOutput::isDirectorRequired() const
{
  for (auto outputFormatWriter : outputFormatWriters_)
  {
    if (outputFormatWriter->isDirectorRequired())
    {
      return true;
    }
  }
  return false;
}

bool ResultOutput::isRegularGridRequired() const
{
  for (auto outputFormatWriter : outputFormatWriters_)
  {
    if (outputFormatWriter->isRegularGridRequired())
    {
      return true;
    }
  }
  return false;
}
//</editor-fold>

std::string ResultFormatWriter::iterationAsString(const SimulationState &simulationState)
{
  char numberChar[9];
  sprintf(numberChar, "%08d", simulationState.currentIteration());
  string fileNameNumber(numberChar);
  if (simulationState.state() == RunningState::COMPLETED)
  { // after completion, output filename with special counter value "final"
    fileNameNumber = "-final";
  }
  return fileNameNumber;
}

//<editor-fold desc="RegularVtkFormatWriter">
void RegularVTKFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  std::string filename = "regularvtk" + iterationAsString(simulationState) + ".vtk";
  std::filesystem::path filePath = outputDirectory / filename;

  RegularGrid *rGrid = geom.getRegularGrid();
  if (rGrid == nullptr)
  {
    RUNTIME_ERROR("RegularVTKFormatWriter requires regular grid");
  }

  rGrid->writeVTKGrid(filePath.c_str(),
                      *potential,
                      *directors,
                      *tiltE,
                      *twistE,
                      *bendE,
                      *elasticE,
                      *thermoE,
                      *electricE,
                      *totalE,
                      *qTensor);
}
//</editor-fold>

//<editor-fold desc="RegularVecMatFormatWriter">
void RegularVecMatFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  fs::path filePath = outputDirectory / ("regularvec" + iterationAsString(simulationState) + ".m");

  RegularGrid *rGrid = geom.getRegularGrid();
  if (rGrid == nullptr)
  {
    RUNTIME_ERROR("RegularVecMatFormatWriter requires regular grid");
  }
  rGrid->writeVecMat(filePath.c_str(), // WRITE REGULAR GRID RESULT FILE
                     *potential,
                     *directors,
                     *qTensor,
                     simulationState.currentTime().getTime());
}
//</editor-fold>

//<editor-fold desc="DirStackZFormatWriter">
void DirStackZFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  std::string filename = "dirstacksz" + iterationAsString(simulationState) + ".csv";

  RegularGrid *rGrid = geom.getRegularGrid();
  if (rGrid == nullptr)
  {
    RUNTIME_ERROR("DirStackZFormatWriter requires regular grid");
  }
  fs::path filePath = outputDirectory / filename;
  rGrid->writeDirStackZ(filePath.c_str(),
                        *directors,
                        *qTensor,
                        simulationState.currentTime().getTime());
}
//</editor-fold>

//<editor-fold desc="CsvUnstructuredFormatWriter">
void CsvUnstructuredFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  fs::path filePath = outputDirectory / ("unstructured.csv." + std::to_string(simulationState.currentIteration()));
  ResultIO::writeCsvUnstructured(geom.getCoordinates(), *potential, *qTensor, *tiltE, *twistE, *bendE, *elasticE, *thermoE, *electricE, *totalE, filePath.string());
}
//</editor-fold>

//<editor-fold desc="VtkUnstructuredAsciiGridFormatWriter">
void VtkUnstructuredAsciiGridFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  std::string fileName = "unstructured" + iterationAsString(simulationState) + ".vtk";
  std::filesystem::path filePath = outputDirectory / fileName;
  ResultIO::writeVtkUnstructuredAsciiGrid(
      geom.getCoordinates(), geom.getnpLC(), geom.getTetrahedra(), *potential, *qTensor, *tiltE, *twistE, *bendE, *elasticE, *thermoE, *electricE, *totalE, filePath.string());
}
//</editor-fold>

void RegularNemaktisDirectorFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
  if (simulationState.state() == RunningState::COMPLETED) {
  fs::path filePath = outputDirectory / ("interpolated_director_nemaktis" + iterationAsString(simulationState) + ".txt");

  RegularGrid *rGrid = geom.getRegularGrid();
  if (rGrid == nullptr)
  {
    RUNTIME_ERROR("RegularNemaktisDirector writer requires regular grid");
  }
  rGrid->writeNemaktisDirector(filePath.c_str(), // WRITE REGULAR GRID RESULT FILE
                         *qTensor);
                         }
}

void RegularNemaktisQTensorFormatWriter::writeResult(const Geometry &geom, const SimulationState &simulationState)
{
    if (simulationState.state() == RunningState::COMPLETED) {
        fs::path filePath = outputDirectory / ("interpolated_qtensor_nemaktis" + iterationAsString(simulationState) + ".txt");

        RegularGrid *rGrid = geom.getRegularGrid();
        if (rGrid == nullptr)
        {
            RUNTIME_ERROR("RegularNemaktisQTensor writer requires regular grid");
        }

       
        
        rGrid->writeNemaktisQtensor(filePath, *qTensor, S0_);
    }
}