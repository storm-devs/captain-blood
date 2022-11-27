
#ifndef _WaveDecomposer_h_
#define _WaveDecomposer_h_

#include "..\..\SndOptions.h"

class WaveDecomposer
{
public:
	WaveDecomposer(const short * sourceWaveData, dword sourceSamplesCount, bool sourceIsStereo, const char * sourceWaveName);
	~WaveDecomposer();

public:
	void Draw(long x, long y, long w, long h);

private:
	void EncodeBlock(short start, short end, short * src, dword sizePow2);
	void EncodeLerp(short start, short end, short * src, dword size, short * dst);

private:
	short * waveData;
	short * waveDataL;
	short * waveDataH;
	dword samplesCount;
	ExtNameStr waveName;

private:
	long drawWindowOffset;
	long drawWindowWidth;
};


#endif
