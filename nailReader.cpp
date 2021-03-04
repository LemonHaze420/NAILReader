#include <iostream>
#include <fstream>
#include <istream>
#include <vector>

class Vector2 
{
public:
    Vector2(float x, float y) : X(x), Y(y) {}
    float X;
    float Y;
};

/// <summary>
/// NAIL file format implementation
/// </summary>
class NAIL 
{
    #define PRECISION               0.00050000002
    #define STORE_PIN_COORD(x)      (uint16_t)((x / 0.00050000000))
    #define READ_PIN_COORD(x)       ((float)x * PRECISION)

    struct nailFileData_t
    {
        __int16 NX1;
        __int16 NY1;
        __int16 NX2;
        __int16 NY2;
        __int16 NX3;
        __int16 NY3;
        __int16 NX4;
        __int16 NY4;
        char char10;
    };
    struct nailFileHeader_t
    {
        char identifier[4]; // 0x4C49414E == 'LIAN'
        nailFileData_t data;
    };
    
public:
    NAIL() = default;
    NAIL(std::string filepath) 
    {
        read(filepath);
    }
    ~NAIL() = default;

    int read(std::string filepath);
    void write(std::string filepath, std::vector<Vector2> outNails);

    /* Stores the nail positions after being processed */
    std::vector<Vector2> nails;

protected:
    /* Stores the original data unmodified */
    std::vector<nailFileData_t> rawData;
};

int NAIL::read(std::string filepath)
{
    int result = -1;

    if (!filepath.empty())
    {
        std::ifstream stream(filepath, std::ios::binary);
        if (stream.good()) 
        {
            int identifier = -1;
            stream.read(reinterpret_cast<char*>(&identifier), 4);
            
            // 'NAIL'
            if (identifier == 0x4C49414E) 
            {
                // now get the total size...
                stream.seekg(0, std::ios::end);
                int size = (int)stream.tellg();
                stream.seekg(4, std::ios::beg);

                int entryBlocks = size / 4;
                int totalEntries = entryBlocks - 1;
                int processedBlocks = 0;

#ifdef _DEBUG
                printf("size = %d\nentryBlocks = %d\ntotalEntries = %d\n", size, entryBlocks, totalEntries);
#endif
                if (totalEntries >= 4) 
                {
                    unsigned __int64 nailCnt = ((unsigned __int64)((int)(entryBlocks - 1) - 4i64) >> 2) + 1;
                    processedBlocks = 4 * nailCnt;
                    
                    do
                    {
                        // read 4 nails at a time..
                        nailFileData_t fileData;
                        stream.read(reinterpret_cast<char*>(&fileData.NX1), 2);
                        stream.read(reinterpret_cast<char*>(&fileData.NY1), 2);

                        stream.read(reinterpret_cast<char*>(&fileData.NX2), 2);
                        stream.read(reinterpret_cast<char*>(&fileData.NY2), 2);

                        stream.read(reinterpret_cast<char*>(&fileData.NX3), 2);
                        stream.read(reinterpret_cast<char*>(&fileData.NY3), 2);

                        stream.read(reinterpret_cast<char*>(&fileData.NX4), 2);
                        stream.read(reinterpret_cast<char*>(&fileData.NY4), 2);

                        float x = READ_PIN_COORD(fileData.NX1);
                        float y = READ_PIN_COORD(fileData.NY1);
                        nails.push_back(Vector2(x, y));

                        x = READ_PIN_COORD(fileData.NX2);
                        y = READ_PIN_COORD(fileData.NY2);
                        nails.push_back(Vector2(x, y));

                        x = READ_PIN_COORD(fileData.NX3);
                        y = READ_PIN_COORD(fileData.NY3);
                        nails.push_back(Vector2(x, y));

                        x = READ_PIN_COORD(fileData.NX4);
                        y = READ_PIN_COORD(fileData.NY4);
                        nails.push_back(Vector2(x, y));

                        rawData.push_back(fileData);

                        --nailCnt;
                    } while (nailCnt);
                }
                if (processedBlocks < totalEntries)
                {
                    int remainingBlocks = totalEntries - processedBlocks;
                    do
                    {
                        // just read two..
                        nailFileData_t fileData;
                        stream.read(reinterpret_cast<char*>(&fileData.NX1), 2);
                        stream.read(reinterpret_cast<char*>(&fileData.NY1), 2);

                        float x = READ_PIN_COORD(fileData.NX1);
                        float y = READ_PIN_COORD(fileData.NY1);
                        nails.push_back(Vector2(x, y));

                        rawData.push_back(fileData);

                        --remainingBlocks;
                    } while (remainingBlocks);
                }
                result = 48 * entryBlocks + 4;
            }
            else printf("Invalid NAIL file!\n");
        }
        printf("Finished reading @0x%X\n", (int)stream.tellg());
        stream.close();
    }
    return result;
}
void NAIL::write(std::string filepath, std::vector<Vector2> outNails)
{
    if (!filepath.empty()) 
    {
        std::ofstream stream(filepath, std::ios::binary);
        if (stream.good()) {
            int header = 0x4C49414E;
            stream.write(reinterpret_cast<char*>(&header), 4);
            for (auto& nail : outNails) 
            {
                __int16 X = STORE_PIN_COORD(nail.X);
                __int16 Y = STORE_PIN_COORD(nail.Y);

                stream.write(reinterpret_cast<char*>(&X), 2);
                stream.write(reinterpret_cast<char*>(&Y), 2);
            }
            stream.close();
        }
    }
}
int main(int argc, char ** argp)
{
    if (!argp[1])
        return 0;
    else if (argp[1] != 0x00)    {
        printf("Reading %s\n", argp[1]);

        NAIL tmpNail;
        int res = tmpNail.read(argp[1]);
        printf("read() returned %d\n", res);

        for (auto& nail : tmpNail.nails) 
            printf("X: %f\nY: %f\n", nail.X, nail.Y);

        printf("Total Nails: %d\n", (int)tmpNail.nails.size());

        NAIL newNail;
        std::string outPath = std::string(argp[1]).append("_custom");
        newNail.write(outPath, tmpNail.nails);
        printf("Written '%s'.\n", outPath.c_str());
    }
}