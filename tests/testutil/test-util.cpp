#include "test-util.h"

#include <cmath>
#include <filesystem>
#include <vector>
#include <fstream>

using namespace TestUtil;
namespace fs = std::filesystem;
//<editor-fold desc=TemporaryFile>
TemporaryFile::TemporaryFile() {
    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    //std::tmpnam()
    char* tempFileName = tempnam(tempDir.string().c_str(), "qlc3d-temp");
    name_ = tempFileName;
    if (fs::exists(name_)) {
      throw std::runtime_error("Temporary file already exists: " + name_.string());
    }
}

TemporaryFile::~TemporaryFile() {
    using namespace std::filesystem;
    if (exists(name_)) {
        remove(name_);
    }
}

TestUtil::TemporaryFile TemporaryFile::empty() {
    return TemporaryFile::withContents("");
}

TestUtil::TemporaryFile TemporaryFile::withContents(const std::string &fileContents) {
    TemporaryFile f;
    FILE *fid = fopen(f.name().string().c_str(), "wt");
    fprintf(fid, "%s", fileContents.c_str());
    fclose(fid);
    return f;
}

std::vector<std::string> TemporaryFile::readContentsAsText() const {
    using namespace std;
    ifstream fin(name());

    vector<string> lines;
    string line;
    while (getline(fin, line)) {
        lines.emplace_back(string(line));
    }
    fin.close();
    return lines;
}

bool TestUtil::isEquivalentAngleDegrees(double a, double b, double epsilon) {
  auto normalize = [](double angle) {
    angle = std::fmod(angle, 360.0);
    if (angle < 0) angle += 360.0;
    return angle;
  };

  double normAngle1 = normalize(a);
  double normAngle2 = normalize(b);

  return std::abs(normAngle1 - normAngle2) < epsilon;
}
//</editor-fold>

//<editor-fold desc=TemporaryDirectory>
TemporaryDirectory::TemporaryDirectory() {
    path_ = std::filesystem::temp_directory_path() / std::filesystem::path(std::tmpnam(nullptr));
    if (!std::filesystem::create_directory(path_)) {
        throw std::runtime_error("Could not create temporary directory " + path_.string());
    }
}

TemporaryDirectory::~TemporaryDirectory() {
  if (fs::exists(path_)) {
    fs::remove_all(path_);
  }
}

std::vector<std::filesystem::path> TemporaryDirectory::listFiles() const {
    std::vector<fs::path> files;
    for (const auto &entry : fs::directory_iterator(path_)) {
        files.emplace_back(entry.path());
    }
    return files;
}
//</editor-fold>