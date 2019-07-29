#pragma once

#include <filesystem>
#include <map>

class OffsetStatsGenerator
{
public:


    OffsetStatsGenerator(const std::filesystem::path& filename);
    ~OffsetStatsGenerator();

    void reportUnknownOffset(int offset, int size, bool isWrite);

    void saveFile();



private:

    struct Offset
    {
        int offset;
        int size;

        bool operator<(const Offset& other) const
        {
            if (offset != other.offset) 
                return (offset < other.offset);
            return size < other.size;
        }
    };

    struct OffsetStatistics
    {
        int reads = 0;
        int writes = 0;
    };

    std::map<Offset, OffsetStatistics> m_offsets;
    std::filesystem::path m_path;

};

