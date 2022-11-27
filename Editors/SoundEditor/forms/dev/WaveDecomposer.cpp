

#include "WaveDecomposer.h"


WaveDecomposer::WaveDecomposer(const short * sourceWaveData, dword sourceSamplesCount, bool sourceIsStereo, const char * sourceWaveName)
{
	Assert(sourceWaveData);
	Assert(sourceSamplesCount >= 128);
	samplesCount = sourceSamplesCount;

	waveData = NEW short[sourceSamplesCount];
	if(!sourceIsStereo)
	{
		memcpy(waveData, sourceWaveData, sourceSamplesCount*sizeof(short));
	}else{
		for(dword i = 0; i < samplesCount; i++)
		{
			waveData[i] = sourceWaveData[i*2];
		}
	}	
	drawWindowOffset = sourceSamplesCount/4;
	drawWindowWidth = samplesCount;
}

WaveDecomposer::~WaveDecomposer()
{
	delete waveData;
}


void WaveDecomposer::Draw(long x, long y, long w, long h)
{
	if(w < 8) w = 8;
	if(drawWindowWidth < 8) drawWindowWidth = 8;
	if(drawWindowOffset < 0) drawWindowOffset = 0;
	if(drawWindowOffset > (long)samplesCount - 2) drawWindowOffset = samplesCount - 2;	
	long v1 = waveData[drawWindowOffset];
	long yc = y + h/2;
	long blockMask = 0xf;
	float scaleY = 0.01f;
	for(long i = 1; i < w; i++)
	{
		long offset = drawWindowOffset + i*8;
		long v2 = waveData[offset];

		float key1 = waveData[offset & ~blockMask];
		float key2 = waveData[(offset + blockMask + 1) & ~blockMask];
		float kBlock = (offset & blockMask)*(1.0f/blockMask);
		long blockValue = long(key1 + (key2 - key1)*kBlock);		
		long newVal = long((v2 - blockValue)*scaleY);
		long blockValueDraw = long(blockValue*scaleY);

		GUIHelper::Draw2DLine(x + i - 1, y - long(v1*scaleY), x + i, y - long(v2*scaleY), 0xff0000ff);
		GUIHelper::Draw2DLine(x + i - 1, y - blockValueDraw, x + i, y - blockValueDraw, 0xff00ff00);		
		GUIHelper::Draw2DLine(x + i - 1, y - newVal, x + i, y - newVal, 0xffff0000);
		
		v1 = v2;
	}



}

void WaveDecomposer::EncodeBlock(short start, short end, short * src, dword sizePow2)
{
		

}

void WaveDecomposer::EncodeLerp(short start, short end, short * src, dword size, short * dst)
{	
	long midle = start << 16;
	long step = ((end - start) << 16)/(size - 1);
	for(dword i = 0; i < size; i++, midle += step)
	{
		long v = src[i] - (midle >> 16);
		Assert(v >= -32768 && v <= 32767);
		dst[i] = short(v);
	}
}


