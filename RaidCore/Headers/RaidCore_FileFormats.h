#ifndef RC_FILE_FORMATS_
#define RC_FILE_FORMATS_

/// NOTE(Roman): temp
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;


namespace file_formats {

    /////////////////////////////////////////
    // WAV
    struct WAVE_header {
        uint32 RIFFID;
        uint32 Size;
        uint32 WAVEID;
    };

#define RIFF_CODE(a, b, c, d) (((uint32)(a) << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))
    enum {
        WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
        WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
        WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
        WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
    };
    struct WAVE_chunk {
        uint32 ID;
        uint32 Size;
    };

    struct WAVE_fmt {
        uint16 wFormatTag;
        uint16 nChannels;
        uint32 nSamplesPerSec;
        uint32 nAvgBytesPerSec;
        uint16 nBlockAlign;
        uint16 wBitsPerSample;
        // extra
        uint16 cbSize;
        uint16 wValidBitsPerSample;
        uint32 dwChannelMask;
        uint8 SubFormat[16];
    };


    /////////////////////////////////////////
    // PNG
#define PNG_HEADER_SIG (0x89ULL | (0x50ULL << 8) | (0x4EULL << 16) | (0x47ULL<< 24) | (0x0DULL << 32) | (0x0AULL << 40) | (0x1AULL << 48) | (0x0AULL << 56))
#define PNG_CODE(a,b,c,d) RIFF_CODE(a,b,c,d)
    enum {
        PNG_chunk_IHDR = PNG_CODE('I', 'H', 'D', 'R'),
        PNG_chunk_IDAT = PNG_CODE('I', 'D', 'A', 'T'),
        PNG_chunk_IEND = PNG_CODE('I', 'E', 'N', 'D'),
        PNG_chunk_PLTE = PNG_CODE('P', 'L', 'T', 'E'),
        PNG_chunk_sRGB = PNG_CODE('s', 'R', 'G', 'B'),
        PNG_chunk_aAMA = PNG_CODE('g', 'A', 'M', 'A'),
        PNG_chunk_pHYs = PNG_CODE('p', 'H', 'Y', 's'),
    };

#pragma pack(push, 1)
    struct PNG_header {
        uint8 signature[8];
    };
    union PNG_chunk {
        struct {
            uint32 length;
            uint32 type;
        };
        struct {
            uint32 length;
            uint32 type;
            uint8  data[0];
            // NOTE(Roman): after the data (length sized) we add a 4byte CRC code that includes the chunk type and data only.
        } c;
    };

    struct PNG_IHDR {
        uint32 width;
        uint32 height;
        uint8 bitDepth;
        uint8 colorType;
        uint8 compressionMethod;
        uint8 filterMethod;
        uint8 interlaceMethod;
    };
    struct PNG_sRGB {
        uint8 intent;
    };
    struct PNG_gAMA {
        uint32 gamma;
    };

#pragma pack(pop)
}

#endif//RC_FILE_FORMATS_
