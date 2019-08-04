#include "stdafx.h"
#include "OffsetStatsGenerator.h"

#include <ostream>
#include <fstream>




void OffsetStatsGenerator::saveFile()
{
  std::ofstream stream(m_path);
  stream << "offset;size;writes;reads" << std::endl;
  for(auto& elem: m_offsets)
  {
     stream << 
         std::hex << elem.first.offset << std::dec << ";" << 
         elem.first.size << ";" << 
         elem.second.writes << ";" << 
         elem.second.reads << ";" << 
         std::endl;
  }

  stream.close();
}


OffsetStatsGenerator::OffsetStatsGenerator(const std::filesystem::path& filename): m_path(filename)
{
}

OffsetStatsGenerator::~OffsetStatsGenerator()
{
    saveFile();
}

void OffsetStatsGenerator::reportUnknownOffset(int offset, int size, bool isWrite)
{
    auto& elem = m_offsets[{offset, size}];
    if (isWrite)
        ++elem.writes;
    else
        ++elem.reads;

}
