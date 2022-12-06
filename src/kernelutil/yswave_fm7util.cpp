#include <stdint.h>
#include "yswave_fm7util.h"



#include "yswavekernel.h"
#include "yswave_waveutil.h"



bool YsWave_FM7Util::TapeFile::IsFBASICFile(void) const
{
	if(2<=block.size() && 
	   0x01==block[0].dump[0] &&
	   0x3c==block[0].dump[1] &&
	   0x00==block[0].dump[2] && // Header block
	   16<=block[0].dump.size())
	{
		if(0x02==block[0].dump[13] && 0x00==block[0].dump[14] && 0x00==block[0].dump[15])
		{
			// Machine-Language
		}
		else if(0x00==block[0].dump[13] && 0x00==block[0].dump[14] && 0x00==block[0].dump[15])
		{
			// F-BASIC Program (Binary)
		}
		else if(0x00==block[0].dump[13] && 0xff==block[0].dump[14] && 0xff==block[0].dump[15])
		{
			// F-BASIC Program (ASCII)
		}
		else if(0x01==block[0].dump[13] && 0xff==block[0].dump[14] && 0xff==block[0].dump[15])
		{
			// ASCII Data (ASCII)
		}
		else
		{
			// Unknown data
			return false;
		}

		if(0xff==block.back().dump[2]) // Last block
		{
			return true;
		}
	}
	return false;
}


YsWave_FM7Util::ByteAnalysis YsWave_FM7Util::GetNext11Wave(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	ByteAnalysis next11;
	YsWave_WaveUtil waveUtil;

	next11.errorFree=true;
	next11.wavePtr[0]=ptr;
	for(int i=0; i<11; ++i)
	{
		if(YSOK==waveUtil.DetectWave(wav,channel,ptr))
		{
			next11.bit[i]=false;
			next11.errorBit[i]=false;
			next11.highFirst[i]=waveUtil.HighFirst();

			auto waveLen=waveUtil.GetWaveLength();
			if(true==opt.IsZero(waveLen))
			{
				next11.errorBit[i]=false;
				next11.bit[i]=false;
			}
			else if(true==opt.IsOne(waveLen) || (10==i && true==opt.IsLong(waveLen)))
			{
				next11.errorBit[i]=false;
				next11.bit[i]=true;
			}
			else
			{
				next11.errorBit[i]=true;
				next11.errorFree=false;
			}

			next11.wavePtr[i+1]=waveUtil.GetRegion().Max()+1;
			ptr=waveUtil.GetRegion().Max()+1;
		}
		else
		{
			next11.wavePtr[i+1]=next11.wavePtr[i];
		}
	}

	if(true==next11.errorBit[0] || true==next11.bit[0] ||
	   true==next11.errorBit[9] || true!=next11.bit[9] ||
	   true==next11.errorBit[10] || true!=next11.bit[10])
	{
		next11.errorFree=false;
	}
	return next11;
}

YsWave_FM7Util::ByteAnalysis YsWave_FM7Util::RebalanceWave(ByteAnalysis byteIn,Option opt) const
{
	long long diff[11];
	for(int i=0; i<11; ++i)
	{
		diff[i]=byteIn.wavePtr[i+1]-byteIn.wavePtr[i];
	}

	for(int i=0; i<11; ++i)
	{
		if(diff[i]<opt.zeroMinWaveLength)
		{
			long long canTakeLeft=0,canTakeRight=0;
			auto needIncrease=opt.zeroMinWaveLength-diff[i];
			if(0<i && true!=byteIn.errorBit[i-1])
			{
				if(true==byteIn.bit[i-1])
				{
					canTakeLeft=diff[i-1]-opt.oneMinWaveLength;
				}
				else
				{
					canTakeLeft=diff[i-1]-opt.zeroMinWaveLength;
				}
			}
			if(i<10 && true!=byteIn.errorBit[i+1])
			{
				if(true==byteIn.bit[i+1])
				{
					canTakeRight=diff[i+1]-opt.oneMinWaveLength;
				}
				else
				{
					canTakeRight=diff[i+1]-opt.zeroMinWaveLength;
				}
			}
			if(needIncrease<=canTakeRight+canTakeLeft)
			{
				printf("%lld %lld\n",canTakeRight,canTakeLeft);
				while(0<needIncrease && (0<canTakeLeft || 0<canTakeRight))
				{
					if(canTakeRight<canTakeLeft)
					{
						diff[i-1]--;
						diff[i]++;
						canTakeLeft--;
						needIncrease--;
					}
					else
					{
						diff[i+1]--;
						diff[i]++;
						canTakeRight--;
						needIncrease--;
					}
				}
			}
		}
		else if(opt.oneMaxWaveLength<diff[i])
		{
			long long canAddLeft=0,canAddRight=0;
			auto needDecrease=diff[i]-opt.oneMaxWaveLength;
			if(0<i && true!=byteIn.errorBit[i-1])
			{
				if(true==byteIn.bit[i-1])
				{
					canAddLeft=opt.oneMaxWaveLength-diff[i-1];
				}
				else
				{
					canAddLeft=opt.zeroMaxWaveLength-diff[i-1];
				}
			}
			if(i<10 && true!=byteIn.errorBit[i+1])
			{
				if(true==byteIn.bit[i+1])
				{
					canAddRight=opt.oneMaxWaveLength-diff[i+1];
				}
				else
				{
					canAddRight=opt.zeroMaxWaveLength-diff[i+1];
				}
			}
			if(needDecrease<=canAddRight+canAddLeft)
			{
				while(0<needDecrease && (0<canAddLeft || 0<canAddRight))
				{
					if(canAddRight<canAddLeft)
					{
						diff[i-1]++;
						diff[i]--;
						canAddLeft--;
						needDecrease--;
					}
					else
					{
						diff[i+1]++;
						diff[i]--;
						canAddRight--;
						needDecrease--;
					}
				}
			}
		}
	}

	ByteAnalysis newByte=byteIn;
	newByte.errorFree=true;
	for(int i=0; i<11; ++i)
	{
		newByte.wavePtr[i+1]=newByte.wavePtr[i]+diff[i];
		if(true==opt.IsZero(diff[i]))
		{
			newByte.bit[i]=false;
			newByte.errorBit[i]=false;
		}
		else if(true==opt.IsOne(diff[i]))
		{
			newByte.bit[i]=true;
			newByte.errorBit[i]=false;
		}
		else
		{
			newByte.errorBit[i]=true;
			newByte.errorFree=false;
		}
	}

	if(newByte.wavePtr[11]!=byteIn.wavePtr[11])
	{
		newByte.errorFree=false;
		printf("Error! Rebalance failed.\n");
	}

	if(true==newByte.errorBit[0] || true==newByte.bit[0] ||
	   true==newByte.errorBit[9] || true!=newByte.bit[9] ||
	   true==newByte.errorBit[10] || true!=newByte.bit[10])
	{
		newByte.errorFree=false;
	}

	return newByte;
}

YSRESULT YsWave_FM7Util::CanRepairByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	auto initial11=GetNext11Wave(wav,channel,ptr,opt);
	auto next11=GetRepairedByte(wav,channel,ptr,opt);
	if(true!=initial11.errorFree && true==next11.errorFree)
	{
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsWave_FM7Util::RepairByte(YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	auto initial11=GetNext11Wave(wav,channel,ptr,opt);
	if(true==initial11.errorFree)
	{
		return YSOK;
	}

	auto next11=GetRepairedByte(wav,channel,ptr,opt);
	if(true==next11.bit[0] || true!=next11.bit[9] || true!=next11.bit[10])
	{
		printf("No start bit or terminal bits.\n");
		return YSERR;
	}

	for(int i=0; i<11; ++i)
	{
		printf("[%d] %d ",i,initial11.wavePtr[i+1]-initial11.wavePtr[i]);
		if(true==initial11.errorBit[i])
		{
			printf("Error!");
		}
		printf("\n");
	}
	for(int i=0; i<11; ++i)
	{
		printf("[%d] %d ",i,next11.wavePtr[i+1]-next11.wavePtr[i]);
		if(true==next11.errorBit[i])
		{
			printf("Error!");
		}
		printf("\n");
	}

	if(true!=initial11.errorFree && true==next11.errorFree)
	{
		for(int i=0; i<11; ++i)
		{
			if(initial11.wavePtr[i]!=next11.wavePtr[i] || initial11.wavePtr[i+1]!=next11.wavePtr[i+1])
			{
				YsWave_WaveUtil waveUtil;
				waveUtil.MakeSineWaveNoZero(wav,channel,next11.highFirst[i],next11.wavePtr[i],next11.wavePtr[i+1],30000);
			}
		}

		printf("Repaired a byte.\n");
		return YSOK;
	}
	return YSERR;
}

YsWave_FM7Util::ByteAnalysis YsWave_FM7Util::GetRepairedByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	auto next11=GetNext11Wave(wav,channel,ptr,opt);

	int nErrBit=0;
	for(auto b : next11.errorBit)
	{
		if(true==b)
		{
			++nErrBit;
			break;
		}
	}
	if(0<nErrBit)
	{
		next11=RebalanceWave(next11,opt);
	}
	return next11;
}

YsWave_FM7Util::ByteData YsWave_FM7Util::ReadByte(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	ByteData byteData;

	byteData.byteData=0;
	byteData.res=YSOK;
	byteData.errorCode=ERROR_NOERROR;
	byteData.errorPtr=0;
	byteData.minmax[0]=ptr;
	byteData.minmax[1]=ptr;

	// Find 01111111111
	YsWave_WaveUtil waveUtil;
	if(YSOK==waveUtil.DetectWave(wav,channel,ptr) && true==opt.IsZero(waveUtil.GetWaveLength()))
	{
		// Found first zero.
		byteData.minmax[0]=waveUtil.GetRegion().Min();

		unsigned int addBit=1;
		for(int i=0; i<8; ++i)
		{
			auto nextPtr=waveUtil.GetRegion().Max()+1;
			if(YSOK==waveUtil.DetectWave(wav,channel,nextPtr))
			{
				if(true==opt.IsOne(waveUtil.GetWaveLength()))
				{
					byteData.byteData+=addBit;
				}
				else if(true==opt.IsZero(waveUtil.GetWaveLength()))
				{
				}
				else
				{
					byteData.res=YSERR;
					byteData.errorCode=ERROR_BYTE_CUT_SHORT;
					byteData.errorPtr=byteData.minmax[0];
					return byteData;
				}
			}
			else
			{
				byteData.res=YSERR;
				byteData.errorCode=ERROR_BYTE_CUT_SHORT;
				byteData.errorPtr=byteData.minmax[0];
				return byteData;
			}
			addBit<<=1;
		}

		for(int i=0; i<2; ++i)
		{
			auto nextPtr=waveUtil.GetRegion().Max()+1;
			if(YSOK==waveUtil.DetectWave(wav,channel,nextPtr))
			{
				if((0==i && true!=opt.IsOne(waveUtil.GetWaveLength())) ||
				   (1==i && true!=opt.IsLong(waveUtil.GetWaveLength())))
				{
					byteData.res=YSERR;
					byteData.errorCode=ERROR_NO_TERM_BIT;
					byteData.errorPtr=byteData.minmax[0];
					return byteData;
				}
			}
		}

		byteData.minmax[1]=waveUtil.GetRegion().Max();
		return byteData;
	}
	else
	{
		byteData.res=YSERR;
		byteData.errorCode=ERROR_NO_START_BIT;
		byteData.errorPtr=ptr;
	}

	return byteData;
}



YsWave_FM7Util::Lead YsWave_FM7Util::FindLead(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	Lead lead;
	lead.minmax[0]=wav.GetNumSamplePerChannel()-1;
	lead.minmax[1]=wav.GetNumSamplePerChannel()-1;
	lead.numFF=0;

	const int minimumFFcount=4;

	long long minPtr=0;
	lead.numFF=0;
	while(ptr<wav.GetNumSamplePerChannel())
	{
		while(ptr<wav.GetNumSamplePerChannel() && 0==wav.GetSignedValue16(channel,ptr))
		{
			++ptr;
		}
		if(wav.GetNumSamplePerChannel()<=ptr)
		{
			lead.numFF=0;
			return lead;
		}

		auto byteData=ReadByte(wav,channel,ptr,opt);

		if(YSOK==byteData.res && 0xff==byteData.byteData)
		{
			if(0==lead.numFF)
			{
				lead.minmax[0]=byteData.minmax[0];
			}
			lead.minmax[1]=byteData.minmax[1];
			ptr=byteData.minmax[1]+1;
			++lead.numFF;
		}
		else
		{
			if(minimumFFcount<=lead.numFF)
			{
				return lead;
			}
			else
			{
				lead.numFF=0;

				auto value16=wav.GetSignedValue16(channel,ptr);
				while(ptr<wav.GetNumSamplePerChannel() && 0<value16*wav.GetSignedValue16(channel,ptr))
				{
					++ptr;
				}
			}
		}
	}

	return lead;
}

YsWave_FM7Util::TapeBlock YsWave_FM7Util::ReadBlock(const YsSoundPlayer::SoundData &wav,int channel,Lead lead,Option opt) const
{
	TapeBlock block;
	ByteData byteData[3];

	block.errorCode=ERROR_NOERROR;
	block.minmax[0]=lead.minmax[0];
	block.minmax[1]=lead.minmax[1]+1;
	block.nLeadFF=lead.numFF;
	block.nTrailFF=0;

	byteData[0]=ReadByte(wav,channel,lead.minmax[1]+1,opt);
	if(byteData[0].res!=YSOK)
	{
		block.errorCode=byteData[0].errorCode;
		block.errorPtr=byteData[0].errorPtr;
		return block;
	}

	byteData[1]=ReadByte(wav,channel,byteData[0].minmax[1]+1,opt);
	if(byteData[1].res!=YSOK)
	{
		block.errorCode=byteData[1].errorCode;
		block.errorPtr=byteData[1].errorPtr;
		return block;
	}

	byteData[2]=ReadByte(wav,channel,byteData[1].minmax[1]+1,opt);
	if(byteData[2].res!=YSOK)
	{
		block.errorCode=byteData[2].errorCode;
		block.errorPtr=byteData[2].errorPtr;
		return block;
	}

	for(auto bd : byteData)
	{
		block.dump.push_back(bd.byteData);
	}


	if(0==byteData[0].byteData)
	{
		printf("Not a F-BASIC format block\n");

		// General data.  PlazmaLine main program was like this.
		// Just read as much as it can go.

		auto ptr=byteData[2].minmax[1]+1;
		for(;;)
		{
			ByteData bd=ReadByte(wav,channel,ptr,opt);
			if(bd.res!=YSOK)
			{
				break;
			}
			block.dump.push_back(bd.byteData);
			block.minmax[1]=bd.minmax[1]+1;
			ptr=bd.minmax[1]+1;
		}

		printf("Read %lld (%llx) bytes.\n",(long long)block.dump.size(),(long long)block.dump.size());

		block.lastBlock=true;
	}
	else if(1==byteData[0].byteData && 0x3c==byteData[1].byteData)
	{
		// F-BASIC format.
		auto typeByte=byteData[2].byteData;
		switch(typeByte)
		{
		default:
			printf("Unknown Block.\n");
			block.lastBlock=false;
			break;
		case 0x00:
			printf("Header Block.\n");
			block.lastBlock=false;
			break;
		case 0x01:
			printf("Data Block.\n");
			block.lastBlock=false;
			break;
		case 0xff:
			printf("Last Block.\n");
			block.lastBlock=true;
			break;
		}

		std::vector <unsigned char> blockData;
		blockData.push_back(typeByte);


		ByteData bd=ReadByte(wav,channel,byteData[2].minmax[1]+1,opt);
		if(bd.res!=YSOK)
		{
			printf("Cannot read the block size.\n");
			printf("Pointer %lld\n",byteData[2].minmax[1]+1);
			block.errorCode=ERROR_CORRUPTED_BLOCK;
			block.errorPtr=bd.minmax[1]+1;
			block.minmax[1]=bd.minmax[1]+1;
			return block;
		}

		auto sizeByte=bd.byteData;
		printf("%d bytes\n",sizeByte);

		blockData.push_back(bd.byteData);
		block.dump.push_back(bd.byteData);

		for(int i=0; i<sizeByte; ++i)
		{
			bd=ReadByte(wav,channel,bd.minmax[1]+1,opt);
			if(bd.res!=YSOK)
			{
				printf("Data block does not finish.\n");
				printf("Byte read error.\n");
				printf("Pointer %lld\n",bd.minmax[1]+1);
				block.errorCode=ERROR_CORRUPTED_BLOCK;
				block.errorPtr=bd.minmax[1]+1;
				block.minmax[1]=bd.minmax[1]+1;
				return block;
			}
			block.dump.push_back(bd.byteData);
			blockData.push_back(bd.byteData);
		}


		if(0x00==typeByte)
		{
			if(0x02==blockData[10] && 0x00==blockData[11] && 0x00==blockData[12])
			{
				for(int i=2; i<10; ++i)
				{
					printf("%c",blockData[i]);
				}
				printf("\n");
				printf("Machine-Language\n");
			}
			else if(0x00==blockData[10] && 0x00==blockData[11] && 0x00==blockData[12])
			{
				for(int i=2; i<10; ++i)
				{
					printf("%c",blockData[i]);
				}
				printf("\n");
				printf("F-BASIC Program (Binary)\n");
			}
			else if(0x00==blockData[10] && 0xff==blockData[11] && 0xff==blockData[12])
			{
				for(int i=2; i<10; ++i)
				{
					printf("%c",blockData[i]);
				}
				printf("\n");
				printf("F-BASIC Program (ASCII)\n");
			}
			else if(0x01==blockData[10] && 0xff==blockData[11] && 0xff==blockData[12])
			{
				for(int i=2; i<10; ++i)
				{
					printf("%c",blockData[i]);
				}
				printf("\n");
				printf("ASCII Data (ASCII)\n");
			}
			else
			{
				for(int i=2; i<10; ++i)
				{
					printf("%02x ",blockData[i]);
				}
				printf("\n");
				printf("Unknown Data %02x %02x %02x\n",blockData[10],blockData[11],blockData[12]);
			}
		}



		bd=ReadByte(wav,channel,bd.minmax[1]+1,opt);
		if(bd.res!=YSOK)
		{
			printf("Cannot read checksum byte.\n");
			printf("Pointer %lld\n",bd.minmax[1]+1);
			block.errorCode=ERROR_CORRUPTED_BLOCK;
			block.errorPtr=bd.minmax[1]+1;
			block.minmax[1]=bd.minmax[1]+1;
			return block;
		}
		auto checkSum=bd.byteData;
		block.dump.push_back(bd.byteData);

		unsigned int sum=0;
		for(auto c : blockData)
		{
			sum+=c;
		}

		if(checkSum!=(sum&255))
		{
			block.errorCode=ERROR_CHECKSUM;
			block.errorPtr=lead.minmax[0];
		}

		printf("Checksum: %02x %02x\n",checkSum,sum&255);
		block.checkSum=checkSum;
		block.actualSum=sum&255;


		// May be 0xFFs all the way to the next block,
		// or, a few 0xFFs and a gap.
		std::vector <long long> trailingFFPtr;

		int nTrailingFF=0;
		for(;;) // Run as long as it can read 0xFF
		{
			bool enoughZero=true;
			for(long long idx=0; idx<opt.zeroMinWaveLength; ++idx)
			{
				if(0!=wav.GetSignedValue16(channel,bd.minmax[1]+1+idx))
				{
					enoughZero=false;
					break;
				}
			}
			if(true==enoughZero)
			{
				block.continuous=false;
				block.minmax[1]=bd.minmax[1];
				block.nTrailFF=nTrailingFF;
				break;
			}

			trailingFFPtr.push_back(bd.minmax[1]+1);
			bd=ReadByte(wav,channel,bd.minmax[1]+1,opt);
			if(YSOK!=bd.res)
			{
				block.continuous=false;
				block.minmax[1]=bd.minmax[1];
				block.nTrailFF=nTrailingFF;
				break;
			}
			else if(bd.byteData!=0xFF)
			{
				block.continuous=true;
				block.minmax[1]=trailingFFPtr[trailingFFPtr.size()/2];
				block.nTrailFF=trailingFFPtr.size()/2;
				break;
			}
			++nTrailingFF;
		}
	}

	return block;
}

YsWave_FM7Util::TapeFile YsWave_FM7Util::ReadFile(const YsSoundPlayer::SoundData &wav,int channel,long long ptr,Option opt) const
{
	TapeFile f;
	f.minmax[0]=0;
	f.minmax[1]=0;
	for(;;)
	{
		auto lead=FindLead(wav,channel,ptr,opt);
		if(0<lead.numFF)
		{
			auto blk=ReadBlock(wav,channel,lead,opt);
			f.block.push_back(blk);
			if(true==blk.lastBlock || ERROR_NOERROR!=blk.errorCode)
			{
				break;
			}
			ptr=blk.minmax[1]+1;
		}
		else
		{
			break;
		}
	}

	if(0<f.block.size())
	{
		f.minmax[0]=f.block.front().minmax[0];
		f.minmax[1]=f.block.back().minmax[1];
	}

	return f;
}

YsWave_FM7Util::TapeBlock YsWave_FM7Util::ReadRawByteSequence(const YsSoundPlayer::SoundData &wav,int channel,Lead lead,Option opt) const
{
	TapeBlock block;

	block.errorCode=ERROR_NOERROR;
	block.minmax[0]=lead.minmax[0];
	block.minmax[1]=lead.minmax[1]+1;
	block.nLeadFF=lead.numFF;
	block.nTrailFF=0;

	// Raw byte sequence.  Courageous Perseus's 3rd file is like this.

	auto ptr=block.minmax[1];
	for(;;)
	{
		ByteData bd=ReadByte(wav,channel,ptr,opt);
		if(bd.res!=YSOK)
		{
			break;
		}
		block.dump.push_back(bd.byteData);
		block.minmax[1]=bd.minmax[1]+1;
		ptr=bd.minmax[1]+1;
	}

	printf("Read %lld (%llx) bytes.\n",(long long)block.dump.size(),(long long)block.dump.size());

	block.lastBlock=true;

	return block;
}

std::vector <unsigned char> YsWave_FM7Util::EncodeT77BitWise(const YsSoundPlayer::SoundData &wav,int channel,Option opt) const
{
	std::vector <unsigned char> t77;

	for(int i=0; i<14; ++i)
	{
		const char *s="XM7 TAPE IMAGE";
		t77.push_back(s[i]);
	}
	t77.push_back(0x20);
	t77.push_back(0x30);
	t77.push_back(0);
	t77.push_back(0);
	t77.push_back(0x7f);
	t77.push_back(0xff);

	int waveLength=0;
	int sign=-1;
	bool firstNonZero=false;
	uint64_t microsec=0;
	int timeBalance=0; // microsec moves 1000000 while ptr moves wav.PlayBackRate()
	uint64_t pulseStartMicrosec=0;
	for(long long int ptr=0; ptr<wav.GetNumSamplePerChannel(); ++ptr)
	{
		int value=wav.GetSignedValue16(channel,ptr);

		timeBalance+=1000000;
		while(0<timeBalance)
		{
			microsec++;
			timeBalance-=wav.PlayBackRate();
		}

		if(true!=firstNonZero && 0==value)
		{
			continue;
		}
		firstNonZero=true;


		if((sign<0 && 0<value) || (value<0 && 0<sign))
		{
			if(true==opt.standardWaveLength)
			{
				// 44KHz sampling -> 19 samples=0x30 in T77.
				int t77Length=0x30*waveLength/18;
				if(t77Length<0)
				{
					t77Length=1;
				}
				else if(0x7f<t77Length)
				{
					t77Length=0x7f;
				}

				// Filter >>
				if(t77Length<0x28)
				{
					t77Length=0x20;
				}
				else
				{
					t77Length=0x30;
				}
				// Filter <<

				if(0<sign)
				{
					t77.push_back(0);
				}
				else
				{
					t77.push_back(0x80);
				}
				t77.push_back(t77Length);
			}
			else
			{
				auto t77Count=(microsec-pulseStartMicrosec)/MICROSEC_PER_ONE_COUNT_T77;
				pulseStartMicrosec+=t77Count*MICROSEC_PER_ONE_COUNT_T77;

				if(t77Count<1)
				{
					t77Count=1;
				}
				else if(0x7F<t77Count)
				{
					t77Count=0x7F;
				}

				if(0<sign)
				{
					t77.push_back(0);
				}
				else
				{
					t77.push_back(0x80);
				}
				t77.push_back(t77Count);
			}
			waveLength=0;
			sign=-sign;
		}
		else
		{
			++waveLength;
		}
	}

	return t77;
}
