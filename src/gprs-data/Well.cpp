#include <Well.hpp>


Well::Well(const WellConfig & config)
    :
    name(config.name),
    radius(config.radius),
    perforated(config.perforated)
{
  assert(!config.coordinates.empty());

  if (config.coordinates.size() == 1)
    coordinate = config.coordinates[0];
  else
  {
    std::cout << "simple" << std::endl;
    segments.reserve(perforated.size());
    segments.emplace_back();
    bool segment_open = false;
    std::size_t i = 0;
    while (i < config.coordinates.size())
    {
      auto & segment = segments.back();
      if (!segment_open)
      {
        segment.first = config.coordinates[i];
        segment_open = true;
        i++;
      }
      else
      {
        segment.second = config.coordinates[i];
        segment_open = false;
      }
    }
  }
}