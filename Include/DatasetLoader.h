#pragma once

#include <Windows.h>
#include <string>
#include <fstream>

namespace RTVR::IO
{
    class DatasetLoader
    {
        struct DatasetFileHeader;

        std::fstream FileStream_;

    public:
        DatasetLoader(const std::string& DatasetFilePath);
        ~DatasetLoader();

        VOID Read(std::byte** DestinationMemoryAddress);

    private:
        VOID ReadFileHeader(DatasetFileHeader& FileHeader);
    };
}