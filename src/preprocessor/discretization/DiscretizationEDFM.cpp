#include "DiscretizationEDFM.hpp"
#include "DiscretizationDFM.hpp"

namespace discretization
{

DiscretizationEDFM::
DiscretizationEDFM(const DoFNumbering & split_dof_numbering,
                   const DoFNumbering & combined_dof_numbering,
                   gprs_data::SimData & data,
                   std::vector<ControlVolumeData> & cv_data,
                   std::vector<ConnectionData> & connection_data)
    : m_split_dofs(split_dof_numbering),
      DiscretizationBase(combined_dof_numbering, data, cv_data, connection_data)
{}

void DiscretizationEDFM::build()
{
  DiscretizationDFM discr_dfm(m_split_dofs, m_data, m_split_cv, m_split_con);
  discr_dfm.build();
  identify_edfm_faces_();
  build_control_volume_data_();
  build_connection_data_();
}

void DiscretizationEDFM::build_control_volume_data_()
{
  for (auto & cv : m_cv_data)
  {
    cv.volume = 0;
    cv.center = {0.0, 0.0, 0.0};
  }

  // first compute parent volumes since some props are weighted by them
  m_dof_mapping.resize(m_split_cv.size());
  for (size_t i = 0; i < m_split_cv.size(); i++)
    if (m_split_cv[i].volume > 0)  // skip inactive cells
    {
      const auto &cv = m_split_cv[i];
      size_t parent_dof;
      if (cv.type == ControlVolumeType::cell)
        parent_dof = m_dofs.cell_dof(cv.master);
      else // if (cv.type == ControlVolumeType::face)
        parent_dof = m_dofs.face_dof(cv.master);
      m_dof_mapping[i] = parent_dof;
      m_cv_data[parent_dof].volume += cv.volume;
    }

  for (size_t i = 0; i < m_split_cv.size(); i++)
    if (m_split_cv[i].volume > 0) // skip inactive cells
    {
      const auto &cv = m_split_cv[i];
      const size_t parent_dof = m_dof_mapping[i];
      auto &parent_cv = m_cv_data[parent_dof];
      parent_cv.type = cv.type;

      std::cout << "cv = " << i << " parent = " << parent_dof << " ";
      if (cv.type == ControlVolumeType::cell)
        std::cout << "cell" << std::endl;
      else
        std::cout << "face" << std::endl;

      const double volume_fraction = cv.volume / parent_cv.volume;
      parent_cv.center += cv.center * volume_fraction;
      parent_cv.porosity += cv.porosity * volume_fraction;
      parent_cv.permeability = cv.permeability; // assume they are the same
      parent_cv.custom = cv.custom;             // assume they are the same
    }
}

void DiscretizationEDFM::build_connection_data_()
{
  create_connections_();

  for (auto it = m_con_data.begin(); it != m_con_data.end(); ++it)
  {
    auto & con = *it;
    const auto edfm_elements = find_edfm_elements_(con);
    if (con.type == ConnectionType::matrix_fracture)
    {
      if (!edfm_elements.empty())
        build_matrix_fracture_(con);
     
    }
    else if (con.type == ConnectionType::fracture_fracture)
    {
      if (edfm_elements.size() == con.elements.size())
        build_edfm_edfm_(con);
      else if (edfm_elements.empty())
        build_edfm_dfm_(con);
      else
      {
        for (size_t i : con.elements)
        abort();
      }
    }
  }
  // convert_flow_map_to_vector_();
}

void DiscretizationEDFM::build_matrix_fracture_(ConnectionData &con)
{
  assert(con.elements.size() == 2);
  // std::cout << con.elements[0] << " " << con.elements[1] << std::endl;
  const auto & cell = m_cv_data[con.elements[1]];
  const auto & frac = m_cv_data[con.elements[0]];
  assert ( cell.type == ControlVolumeType::cell );
  assert ( frac.type == ControlVolumeType::face );
  const double k_d = cell.permeability * con.normal * con.normal;  // directional permeability
  const double T = k_d * con.area / con.distances[1]; // average distance from cell to fracture
  con.coefficients = {-T, T};
}

void DiscretizationEDFM::build_edfm_edfm_(ConnectionData & con)
{
  // assert(con.elements.size() == 2);
  // std::cout << "edfm-edfm " << con.elements[0] << " " << con.elements[1] << std::endl;

  // size_t con_index;
  // if (m_con_map.contains(con.elements[0], con.elements[1]))
  //   con_index = m_con_map.index(con.elements[0], con.elements[1]);
  // else
  //   con_index = m_con_map.insert(con.elements[0], con.elements[1]);
  // auto &new_con = m_con_map.get_data(con_index);
  // new_con = con;
}

void DiscretizationEDFM::build_edfm_dfm_(ConnectionData & con)
{
  // assert (m_min_edfm_index <= con.elements[1] && m_min_edfm_index + m_n_edfm_faces > con.elements[1]);
  // assert(con.elements.size() == 2);
  // std::cout << "edfm-dfm " << con.elements[0] << " " << con.elements[1] << std::endl;

  // // first element is dfm, second is dfm
  // size_t con_index;
  // if (m_con_map.contains(con.elements[0], con.elements[1]))
  //   con_index = m_con_map.index(con.elements[0], con.elements[1]);
  // else
  //   con_index = m_con_map.insert(con.elements[0], con.elements[1]);
  // auto &new_con = m_con_map.get_data(con_index);
  // new_con = con;
}

void DiscretizationEDFM::identify_edfm_faces_()
{
  for (auto face = m_grid.begin_active_faces(); face != m_grid.end_active_faces(); ++face)
  {
    const auto & face_parent = m_grid.ultimate_parent(*face);
    const auto & neighbor = *(face_parent.neighbors()[0]);
    if (neighbor.ultimate_parent() == neighbor)
      m_edfm_faces.insert( face->index() );
  }
}

std::vector<size_t> DiscretizationEDFM::find_edfm_elements_(const ConnectionData & con)
{
  std::vector<size_t> result;
  for (std::size_t i=0; i<con.elements.size(); ++i)
    if (m_edfm_faces.find(con.elements[i]) != m_edfm_faces.end())
      result.push_back(i);
  return result;
}

void DiscretizationEDFM::create_connections_()
{
  hash_algorithms::ConnectionMap<ConnectionData> con_map;
  // first build the connection map
  for (const auto &con : m_split_con)
    if ( !find_edfm_elements_(con).empty() )
    {
      size_t con_index;
      const size_t dof1 = m_dof_mapping[con.elements[0]];
      const size_t dof2 = m_dof_mapping[con.elements[1]];
      if (dof1 != dof2)
      {
        if (con_map.contains(dof1, dof2))
          con_index = con_map.index(dof1, dof2);
        else
        {
          con_index = con_map.insert(dof1, dof2);
          auto &new_con = con_map.get_data(con_index);
          new_con.area = 0;
          new_con.normal = {0,0,0};
          new_con.center = {0,0,0};
          new_con.distances = {0,0};
          new_con.elements = {dof1, dof2};
        }
      }

      auto &new_con = con_map.get_data(con_index);
      new_con.type = con.type;
      if (con.type == discretization::ConnectionType::matrix_fracture)
      {
        new_con.area += 2 * con.area;
        const auto & cell = m_split_cv[con.elements[1]];
        const auto & face = m_split_cv[con.elements[0]];
        const auto & parent_cell = m_cv_data[dof2];
        const double dist = (cell.center - face.center).dot( con.normal );
        const double cell_volume_ratio = cell.volume / parent_cell.volume;
        new_con.distances[0] += 0.0;  // from fracture to connection
        new_con.distances[1] += std::fabs(dist) * cell_volume_ratio;  // from cell to connection
        new_con.normal = con.normal;
        const auto & parent_face = m_cv_data[dof1];
        const double face_volume_ratio = face.volume / parent_face.volume;
        new_con.center += con.center * face_volume_ratio;
      }
      else if (con.type == discretization::ConnectionType::fracture_fracture)
      {
        // new_con.center;
      }
    }

  m_con_data = con_map.get_data();
}

}  // end namespace discretization
