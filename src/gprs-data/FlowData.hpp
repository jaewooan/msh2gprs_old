#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class FlowData
{
 public:
  FlowData(const std::size_t max_connections = 1e10);
  std::size_t insert_connection(const std::size_t ielement,
                                const std::size_t jelement);
  std::size_t connection_index(const std::size_t ielement,
                               const std::size_t jelement) const;
  // get connection index
  // std::size_t connection()
  // get two elements from hash value
  std::pair<std::size_t,std::size_t>
  invert_hash(const std::size_t hash) const;
  void merge_elements(const std::size_t updated_element,
                      const std::size_t merged_element);
  void clear_connection(const std::size_t ielement,
                        const std::size_t jelement);



 private:
  std::size_t hash_value(const std::size_t ielement,
                         const std::size_t jelement) const;

 public:
  std::vector<double> volumes, poro, depth;
  // regular transmissibilities
  // std::vector<std::size_t> ielement, jelement;
  std::unordered_map<std::size_t, std::size_t> map_connection;
  std::unordered_map<std::size_t, std::vector<std::size_t>> element_connection;

  std::vector<double>      trans_ij, conduct_ij;
  // connections
  // std::vector<int> connection_type;
  // std::vector<int> connection_n;

  // user-defined cell data
  std::vector<std::vector<double>> custom_data;
  std::vector<std::string>         custom_names;

 private:
  std::size_t max_connections;
};


inline
std::size_t FlowData::insert_connection(const std::size_t ielement,
                                        const std::size_t jelement)
{
  const std::size_t hash = hash_value(ielement, jelement);
  if (map_connection.find(hash) != map_connection.end())
    throw std::runtime_error("connection exists");
  map_connection.insert({hash, map_connection.size()});

  // element connections
  // ielement
  auto it = element_connection.find(ielement);
  if (it != element_connection.end())
    it->second.push_back(jelement);
  else
  {
    std::vector<std::size_t> new_connection_vector = {jelement};
    element_connection.insert({ielement, new_connection_vector});
  }

  // jelement
  it = element_connection.find(jelement);
  if (it != element_connection.end())
    it->second.push_back(ielement);
  else
  {
    std::vector<std::size_t> new_connection_vector = {ielement};
    element_connection.insert({jelement, new_connection_vector});
  }

  return map_connection.size();
}


inline
std::size_t FlowData::hash_value(const std::size_t ind1,
                                 const std::size_t ind2) const
{
  if (ind1 < ind2)
    return max_connections * ind1 + ind2;
  else
    return max_connections * ind2 + ind1;
}


inline
std::pair<std::size_t,std::size_t>
FlowData::invert_hash(const std::size_t hash) const
{
  std::pair<std::size_t,std::size_t> pair;
  pair.first = hash % max_connections;
  pair.second = hash - pair.first;

  if (map_connection.find(hash) == map_connection.end())
    throw std::runtime_error("element does not exist");

  return std::move(pair);
}
