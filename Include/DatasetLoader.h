#pragma once

#include <Windows.h>
#include <string>
#include <fstream>
#include <filesystem>

namespace RTVR::IO
{
    struct Dataset
    {
        UINT32 GridResolutionX;
        UINT32 GridResolutionY;
        UINT32 GridResolutionZ;

        FLOAT VolumeExtentX;
        FLOAT VolumeExtentY;
        FLOAT VolumeExtentZ;

        std::byte* VolumeData;
    };

    class DatasetLoader
    {
        struct DatasetFileHeader;

        std::fstream FileStream_;

    public:
        DatasetLoader(const std::filesystem::path& DatasetFilePath);
        ~DatasetLoader();

        VOID Read(Dataset& Dataset);

    private:
        VOID ReadFileHeader(DatasetFileHeader& FileHeader);
    };
}