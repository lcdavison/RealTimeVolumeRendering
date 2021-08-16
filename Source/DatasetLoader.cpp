#include "DatasetLoader.h"

#include <stdexcept>

namespace RTVR::IO
{
    #pragma region Dataset_File_Header
    struct DatasetLoader::DatasetFileHeader
    {
        UINT32 GridResolutionX;
        UINT32 GridResolutionY;
        UINT32 GridResolutionZ;

        /*
        *  According to https://web.cs.ucdavis.edu/~okreylos/PhDStudies/Spring2000/ECS277/DataSets.html
        *  this value is not used by the provided datasets, and is set to 0 by default.
        */
        UINT32 VolumeBorderSize;

        FLOAT VolumeExtentX;
        FLOAT VolumeExtentY;
        FLOAT VolumeExtentZ;
    };
    #pragma endregion Dataset_File_Header

    #pragma region Utilities
    inline UINT32 EndianSwap(UINT32 InputValue)
    {
        UINT32 Result = {};

        Result |= ((InputValue & 0x000000FF) << 24);
        Result |= ((InputValue & 0x0000FF00) << 8);
        Result |= ((InputValue & 0x00FF0000) >> 8);
        Result |= ((InputValue & 0xFF000000) >> 24);

        return Result;
    }

    inline UINT32 ReInterpretFloatAsUInt32(FLOAT FloatValue)
    {
        return *(reinterpret_cast<UINT32*>(&FloatValue));
    }

    inline FLOAT ReInterpretUInt32AsFloat(UINT32 IntegerValue)
    {
        return *(reinterpret_cast<FLOAT*>(&IntegerValue));
    }

    inline FLOAT EndianSwapFloat(FLOAT InputValue)
    {
        UINT32 ReinterpretedFloatValue = { ReInterpretFloatAsUInt32(InputValue) };
        UINT32 EndianSwapResult = { EndianSwap(ReinterpretedFloatValue) };

        return ReInterpretUInt32AsFloat(EndianSwapResult);
    }
    #pragma endregion Utilities

    DatasetLoader::DatasetLoader(const std::string& DatasetFilePath)
    {
        FileStream_.open(DatasetFilePath, std::fstream::in | std::fstream::binary);

        if (!FileStream_.is_open())
        {
            throw std::runtime_error("Failed to open dataset file: " + DatasetFilePath);
        }
    }

    DatasetLoader::~DatasetLoader()
    {
        FileStream_.close();
    }

    VOID DatasetLoader::Read(std::byte** DestinationMemoryAddress)
    {
        DatasetFileHeader FileHeader = {};
        ReadFileHeader(FileHeader);

        UINT32 VolumeDataCount = { FileHeader.GridResolutionX * FileHeader.GridResolutionY * FileHeader.GridResolutionZ };

        SIZE_T VolumeDataSizeInBytes = { VolumeDataCount * sizeof(unsigned char) };

        std::byte*& DestinationBuffer = *(DestinationMemoryAddress);
        DestinationBuffer = new std::byte [VolumeDataSizeInBytes];

        FileStream_.read(reinterpret_cast<char*>(DestinationBuffer), VolumeDataSizeInBytes);
    }

    VOID DatasetLoader::ReadFileHeader(DatasetFileHeader& FileHeader)
    {
        FileStream_.read(reinterpret_cast<char*>(&FileHeader), sizeof(FileHeader));

        FileHeader.GridResolutionX = EndianSwap(FileHeader.GridResolutionX);
        FileHeader.GridResolutionY = EndianSwap(FileHeader.GridResolutionY);
        FileHeader.GridResolutionZ = EndianSwap(FileHeader.GridResolutionZ);

        FileHeader.VolumeExtentX = EndianSwapFloat(FileHeader.VolumeExtentX);
        FileHeader.VolumeExtentY = EndianSwapFloat(FileHeader.VolumeExtentY);
        FileHeader.VolumeExtentZ = EndianSwapFloat(FileHeader.VolumeExtentZ);
    }
}