#pragma once

#include "DiscretizationBase.hpp"

namespace discretization
{

enum tpfa_method
{
  mo = 0,
  kirill = 1
};

class DiscretizationTPFA : public DiscretizationBase
{
 public:
  // DiscretizationTPFA(const mesh::Mesh                       & grid,
  //                    const std::set<int>                    & dfm_markers,
  //                    const std::vector<std::vector<double>> & props,
  //                    const std::vector<std::string>         & keys,
  //                    const size_t                             numbering_shift = 0);
  DiscretizationTPFA(const std::vector<DiscreteFractureConfig> & dfm_fractures,
                     gprs_data::SimData & data);

  virtual void build() override;

 protected:
  void build_kirill(const mesh::Face & face,
                    ConnectionData                  & data);
  void build_mo(const mesh::Face & face,
                ConnectionData                  & data);

  // shift of controle volume indices
  // used i.e. when grid is a subdomain or when
  // domain control volumes follow fracture control volumes
  // in numbering
  size_t m_shift;
  const int m_method;
};

}
