#include "simdata.hpp"
#include "femout.hpp"
#include "transes.hpp"
#include <Parser.hpp>

#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
using Path = filesystem::path;

// class SimData;
// class CalcTranses;
// class OutputData;
// class tetgenio;
// class tetrahedralize;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "please specify a config file."
              << std::endl
              << "Example: "
              << "msh2gprs config.json"
              << std::endl
              << "an example config is distributed with this code"
              << std::endl;
    return 0;
  }
  if (argc > 2)
  {
    std::cout << "what the hell did you pass?" << std::endl;
    return 1;
  }

  const std::string fname_config = argv[1];  // config file

  Parsers::Parser parser;
  std::cout << "parsing " << fname_config << std::endl;
  parser.parse_file(fname_config);
  const SimdataConfig config = parser.get_config();

  // get path of the config file -- mesh is searched for in relative path
  const Path path_config(fname_config);
  const Path config_dir_path = path_config.parent_path();
  Path path_gmsh = config_dir_path / config.mesh_file;

  // check if mesh file exists
  if (!filesystem::exists(path_gmsh))
  {
    std::cout << "msh file does not exist:" << std::endl;
    std::cout << filesystem::absolute(path_gmsh) << std::endl;
    return 0;
  }

  std::string outstream;
  SimData * pSimData;
  pSimData = new SimData(filesystem::absolute(path_gmsh), config);

  cout << "Read gmsh data" << endl;
  pSimData->readGmshFile();

  cout << "Extract all polygons (slow)" << endl;
  pSimData->extractInternalFaces();

  cout << "Convert GMSH FEM mesh into SIM data" << endl;
  pSimData->convertGmsh2Sim();

  cout << "Fill 3D rock properties" << endl;
  pSimData->defineRockProperties();

  cout << "Make SDA properties" << endl;
  pSimData->defineEmbeddedFractureProperties();

  cout << "Create physical facets" << endl;
  cout << " ( bnd & frac faces )" << endl;
  pSimData->definePhysicalFacets();

  cout << "Create bc stress & disp" << endl;
  pSimData->defineStressAndDispOnBoundary();

  cout << endl << "Convert FEM mesh into FVM mesh" << endl;
  pSimData->handleConnections();

  std::cout << "computing reservoir transes" << std::endl;
  pSimData->computeReservoirTransmissibilities();

  cout << "Compute cell clipping and EDFM transmissibilities" << endl;
  pSimData->computeCellClipping();

  // cout << "Create simple wells" << endl;
  // pSimData->createSimpleWells();

  cout << "Split FEM mesh on internal surfaces" << endl;
  pSimData->splitInternalFaces();

  cout << "Write FEM mesh data\n";
  // OutputData * pOut;
  // pOut = new OutputData(pSimData);
  OutputData output_data(pSimData);

  const std::string output_dir =  std::string(filesystem::absolute(config_dir_path)) + "/";
  std::cout << "output directory: " << output_dir << std::endl;
  output_data.write_output(output_dir);

  delete pSimData;
  return 0;
}
