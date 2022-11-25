#ifndef YSWAVEKERNEL_IS_INCLUDED
#define YSWAVEKERNEL_IS_INCLUDED
/* { */

#include "ysclass.h"
#include "ysport.h"

#include "yssimplesound.h"



class YsWaveKernel
{
public:
	class Viewport
	{
	public:
		long long int zero,wid;
		inline Viewport()
		{
			zero=0;
			wid=1;
		}
	};
	class Region
	{
	public:
		long long int minmax[2];
		inline Region()
		{
			minmax[0]=0;
			minmax[1]=0;
		}
		long long Min(void) const;
		long long Max(void) const;
		long long GetLength(void) const;
	};
	class Selection : public Region
	{
	public:
	};
	class Peak
	{
	public:
		bool isHigh;
		bool deleted;
		YSSIZE_T idx;
		inline Peak()
		{
			isHigh=false;
			deleted=false;
			idx=0;
		}
	};
	class Envelope
	{
	public:
		int high,low;
	};

protected:
	YsSoundPlayer::SoundData wav;
	Viewport viewport;
	Selection selection;
	std::vector <Peak> peak;
	std::vector <Envelope> envelope;
	std::vector <Region> silentSegment;
public:
	YsWaveKernel();
	~YsWaveKernel();
	void CleanUp(void);

	const YsSoundPlayer::SoundData &GetWave(void) const;

	YSRESULT LoadWav(const YsWString fName);
	YSRESULT SaveWav(const YsWString fName) const;

	YSRESULT AppendWav(const YsSoundPlayer::SoundData &incoming);

	Viewport GetViewport(void) const;
	void SetViewport(Viewport vp);
	void SetViewportZero(int zero);
	void SetViewportWidth(int wid);

	Selection GetSelection(void) const;
	void SetSelection(Selection sel);

	const std::vector <Peak> &GetPeak(void) const;
	void SetPeak(std::vector <Peak> &&peak);
	void SetPeak(const std::vector <Peak> &peak);

	const std::vector <Envelope> &GetEnvelope(void) const;
	void SetEnvelope(std::vector <Envelope> &&env);
	void SetEnvelope(const std::vector <Envelope> &env);

	const std::vector <Region> &GetSilentSegment(void) const;

	bool IsPeak(bool &isHigh,int channel,YSSIZE_T ptr) const;

	long long ErasePeak(int channel,YSSIZE_T peakIdx);
};



/* } */
#endif
