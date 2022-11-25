#ifndef YSWAVE_FM7UTIL_IS_INCLUDED
#define YSWAVE_FM7UTIL_IS_INCLUDED
/* { */



#include <vector>
#include "yssimplesound.h"

class YsWave_FM7Util
{
public:
	enum
	{
		ERROR_NOERROR,
		ERROR_UNIDENT_WAVELENGTH,
		ERROR_NO_START_BIT,
		ERROR_NO_TERM_BIT,
		ERROR_BYTE_CUT_SHORT,
		ERROR_CHECKSUM,
		ERROR_CORRUPTED_BLOCK,
	};

	class Option
	{
	public:
		long long zeroMinWaveLength,zeroMaxWaveLength;
		long long oneMinWaveLength,oneMaxWaveLength;

		Option()
		{
			zeroMinWaveLength=11;
			zeroMaxWaveLength=27;
			oneMinWaveLength=32;
			oneMaxWaveLength=60;
		}
		bool IsZero(long long waveLength)
		{
			return (zeroMinWaveLength<=waveLength && waveLength<=zeroMaxWaveLength);
		}
		bool IsOne(long long waveLength)
		{
			return (oneMinWaveLength<=waveLength && waveLength<=oneMaxWaveLength);
		}
		// Tolerate longer 10th bit.
		bool IsLong(long long waveLength)
		{
			return (oneMinWaveLength<=waveLength);
		}
	};

	class ByteData
	{
	public:
		unsigned char byteData;
		YSRESULT res;
		int errorCode;
		long long errorPtr;
		long long minmax[2];
	};

	class TapeBlock
	{
	public:
		int errorCode;
		long long errorPtr;
		std::vector <unsigned char> dump;
		long long minmax[2];
		unsigned int checkSum,actualSum;
		long long nLeadFF,nTrailFF;

		/*! A block may be ended with a few 0xFFs and then a gap before the next lead.
		    Or, the trailing 0xFFs is continued to the lead for the next block.
		    If continuousToNext is true, trailing 0xFFs are connected to the next lead
		    (or, the trailing 0xFFs and the next lead cannot be distinguished.)
		*/
		bool continuous;

		bool lastBlock;
	};
	class TapeFile
	{
	public:
		long long minmax[2];
		std::vector <TapeBlock> block;

		bool IsFBASICFile(void) const;
	};

	class Lead
	{
	public:
		long long minmax[2];
		int numFF;
	};

	class ByteAnalysis
	{
	public:
		long long wavePtr[12];
		bool errorBit[11];
		bool bit[11];
		bool highFirst[11];
		bool errorFree;
	};

private:

public:
	ByteAnalysis GetNext11Wave(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
	ByteAnalysis RebalanceWave(ByteAnalysis byteIn,Option opt) const;
	YSRESULT RepairByte(YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
	YSRESULT CanRepairByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
private:
	ByteAnalysis GetRepairedByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;

public:
	ByteData ReadByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
	Lead FindLead(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
	TapeBlock ReadBlock(const YsSoundPlayer::SoundData &wav,int channel,Lead lead,Option opt) const;
	TapeFile ReadFile(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const;
	TapeBlock ReadRawByteSequence(const YsSoundPlayer::SoundData &wav,int channel,Lead lead,Option opt) const;

	std::vector <unsigned char> EncodeT77BitWise(const YsSoundPlayer::SoundData &wav,int channel,Option opt) const;
};



/* } */
#endif
