#pragma once

#include <Shape.hpp>
#include <json.hpp>
#include <vector>
#include <SimdataConfig.hpp>

namespace Parsers
{

class Parser
{
 public:
  Parser();
  void parse_file(const std::string & fname);
  SimdataConfig & get_config();
 private:
  // JSON
  void parse_json(const std::string & fname);
  void domain_props_json(const nlohmann::json::iterator & section_it);
  void embedded_fracs_json(const nlohmann::json::iterator & section_it);
  void boundary_conditions_json(const nlohmann::json::iterator & section_it);
  void boundary_conditions_faces(nlohmann::json::iterator it,
                                 const nlohmann::json::iterator & end);
  void boundary_conditions_nodes(nlohmann::json::iterator it,
                                 const nlohmann::json::iterator & end);
  void boundary_conditions_face(nlohmann::json::iterator it,
                                const nlohmann::json::iterator & end,
                                BCConfig & conf);
  void boundary_conditions_node(nlohmann::json::iterator it,
                                const nlohmann::json::iterator & end,
                                BCNodeConfig & conf);

  std::pair<std::string,std::string>
  get_pair_json(const nlohmann::json::iterator & section_it);
  // ATTRIBUTES
  SimdataConfig config;
  std::string comment = "_comment_";
};

}
