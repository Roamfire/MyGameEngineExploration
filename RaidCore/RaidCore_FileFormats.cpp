#include "Headers\RaidCore_Common.h"
#include "Headers\RaidCore_FileFormats.h"

namespace file_formats {
	namespace helpers {
		/* Table of CRCs of all 8-bit messages. */
		unsigned long crc_table[256];
		/* Flag: has the table been computed? Initially false. */
		int crc_table_computed = 0;
		/* Make the table for a fast CRC. */
		void make_crc_table(void)
		{
			unsigned long c;
			int n, k;
			for (n = 0; n < 256; n++) {
				c = (unsigned long)n;
				for (k = 0; k < 8; k++) {
					if (c & 1)
						c = 0xedb88320L ^ (c >> 1);
					else
						c = c >> 1;
				}
				crc_table[n] = c;
			}
			crc_table_computed = 1;
		}
		/* Update a running CRC with the bytes buf[0..len-1]--the CRC
		should be initialized to all 1's, and the transmitted value
		is the 1's complement of the final running CRC (see the
		crc() routine below)). */
		unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
		{
			unsigned long c = crc;
			int n;
			if (!crc_table_computed)
				make_crc_table();
			for (n = 0; n < len; n++) {
				c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
			}
			return c;
		}
		/* Return the CRC of the bytes buf[0..len-1]. */
		unsigned long crc(unsigned char *buf, int len)
		{
			return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
		}
	}

	///////////////////////////////////////////////////////////////////
	// NOTE(Roman): Audio functions
	inline uint32 maxBufferSize(WAVE_fmt* wav) {
		return (wav->nSamplesPerSec * (wav->wBitsPerSample >> 3) * wav->nChannels);
	}

	uint32 getWAVDataSize(uint8 * data, uint32 length, file_formats::WAVE_fmt * wav, uint8 ** pAudio) {
		file_formats::WAVE_header * wHdr = (file_formats::WAVE_header *)data;

		uint8* parse = (data + sizeof(file_formats::WAVE_header));
		file_formats::WAVE_chunk* c;

		// find the format
		do {
			c = (file_formats::WAVE_chunk*) parse;
			parse += sizeof(file_formats::WAVE_chunk);
		} while (c->ID != file_formats::WAVE_ChunkID_fmt);

		*wav = (file_formats::WAVE_fmt)*(file_formats::WAVE_fmt*)parse;
		// NOTE(Roman): don't do + sizeof(file_formats::WAVE_fmt), 
		// because the WAV file might not be in the version with the extended data
		// use the chunk size instead to advance to the next chunk
		parse += c->Size;

		// NOTE(Roman): to avoid confusion, eliminate extra values
		setBytes(((uint8*)wav) + c->Size, 0, sizeof(file_formats::WAVE_fmt) - c->Size);

		// find the data
		do {
			c = (file_formats::WAVE_chunk*) parse;
			parse += sizeof(file_formats::WAVE_chunk);
		} while (c->ID != file_formats::WAVE_ChunkID_data);

		// NOTE(Roman): now we are at the actual audio data
		*pAudio = parse;
		return (c->Size);
	}

	///////////////////////////////////////////////////////////////////
	// NOTE(Roman): PNG functions
}
