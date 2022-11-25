#include <ysclass.h>
#include <ysport.h>
#include <yscompilerwarning.h>
#include <ysgl.h>
#include <ysglcpp.h>
#include <ysglslcpp.h>

#include "fsguiapp.h"





void FsGuiMainCanvas::Draw(void)
{
	// Do this at the beginning of Draw funtion.  This will allow one of the elements set SetNeedRedraw(YSTRUE) 
	// within drawing function so that Draw function will be called again in the next iteragion. >>
	SetNeedRedraw(YSFALSE);
	// <<

	glDisable(GL_DEPTH_TEST);

	glUseProgram(0);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);


	int wid,hei;
	FsGetWindowSize(wid,hei);
	glViewport(0,0,wid,hei);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);



	// 2D Drawing
	glDisable(GL_DEPTH_TEST);

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
	YsGLSLUseWindowCoordinateInPlain2DDrawing(YsGLSLSharedPlain2DRenderer(),YSTRUE);
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	{
		auto &wavEdit=GetCurrentWav();
		auto selection=wavEdit.GetSelection();
		auto viewport=wavEdit.GetViewport();
		int fontSize=hei/60;

		YsGLSLBitmapFontRendererClass renderer;
		renderer.RequestFontSize(fontSize*2/3,fontSize);
		int y=hei-1;

		renderer.SetUniformColor(0,0,0,1);;
		renderer.SetViewportDimension(wid,hei);
		renderer.SetViewportOrigin(YSGLSL_BMPFONT_TOPLEFT_AS_ORIGIN);

		YsString msg;

		msg.Printf("Selection  From:%lld  To:%lld  Length:%lld",
		   (long long int)(selection.minmax[0]),
		   (long long int)(selection.minmax[1]),
		   (long long int)(1+selection.minmax[1]-selection.minmax[0]));
		renderer.DrawString(0,y,msg.c_str());

		y-=fontSize;

		msg.Printf("Looking At:%lld  From:%lld  To:%lld",
		   (long long int)(viewport.zero+viewport.wid/2),
		   (long long int)(viewport.zero),
		   (long long int)(viewport.zero+viewport.wid));
		renderer.DrawString(0,y,msg.c_str());

		y-=fontSize;

		msg.Printf("Channel: %d",GetCurrentChannel());
		renderer.DrawString(0,y,msg.c_str());

		y-=fontSize;
	}

	DrawHighlightSelection();
	DrawWave();

	glUseProgram(0);
	FsGuiCanvas::Show();

	FsSwapBuffers();
}

void FsGuiMainCanvas::DrawHighlightSelection(void) const
{
	YsGLSL2DRenderer renderer;

	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto selection=wavEdit.GetSelection();
	auto viewport=wavEdit.GetViewport();
	auto waveRect=GetWaveDrawingRect();

	long long x[2]=
	{
		(selection.minmax[0]-viewport.zero)*wid/viewport.wid,
		(selection.minmax[1]-viewport.zero)*wid/viewport.wid,
	};
	for(auto &X : x)
	{
		YsMakeGreater<long long>(X,waveRect.Min().x());
		YsMakeSmaller<long long>(X,waveRect.Max().x());
	}


	const float gray[4]=
	{
		0.8f,0.8f,0.8f,1.0f
	};
	renderer.SetUniformColor(gray);

	std::vector <float> vtx;

	vtx.push_back((float)(x[1]));vtx.push_back((float)waveRect.Min().y());
	vtx.push_back((float)(x[1]));vtx.push_back((float)waveRect.Max().y());
	vtx.push_back((float)(x[0]));vtx.push_back((float)waveRect.Max().y());
	vtx.push_back((float)(x[0]));vtx.push_back((float)waveRect.Min().y());

	renderer.DrawVtx(GL_TRIANGLE_FAN,4,vtx.data());
}

void FsGuiMainCanvas::DrawWave(void) const
{
	if(nullptr!=drawAllChannel && YSTRUE==drawAllChannel->GetCheck())
	{
		for(int i=0; i<GetCurrentWav().GetWave().GetNumChannel(); ++i)
		{
			if(i!=GetCurrentChannel())
			{
				YsColor lightRed;
				lightRed.SetDoubleRGB(1.0,0.75,0.75);
				DrawWaveChannel(
				    i,
				    GetWaveDrawingRect(),
				    0,
				    lightRed);
			}
		}
	}
	DrawWaveChannel(GetCurrentChannel(),GetWaveDrawingRect(),GetTemporaryPhaseOffset(),YsBlack());
}

void FsGuiMainCanvas::DrawWaveChannel(
    int channel,YsRect2i waveRect,long long int offset,YsColor col) const
{
	long long int vScale=waveRect.GetHeight()/2;
	long long int level0=waveRect.GetCenter().y();

	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto &wavRaw=wavEdit.GetWave();
	auto viewport=wavEdit.GetViewport();

	YsGLSL2DRenderer renderer;

	{
		std::vector <float> vtx;
		auto &envelope=wavEdit.GetEnvelope();

		const float col[4]={0.6f,0.6f,0.6f,1.0f};
		renderer.SetUniformColor(col);

		for(int x=0; x<wid; ++x)
		{
			unsigned long long ptr=viewport.zero+x*viewport.wid/wid;
			if(0<=ptr && ptr<envelope.size())
			{
				int level=envelope[ptr].high;
				vtx.push_back(x);
				vtx.push_back((float)(level0-level*vScale/32768));
			}
		}
		renderer.DrawVtx(GL_LINE_STRIP,vtx.size()/2,vtx.data());


		vtx.clear();
		for(int x=0; x<wid; ++x)
		{
			unsigned long long ptr=viewport.zero+x*viewport.wid/wid;
			if(0<=ptr && ptr<envelope.size())
			{
				int level=envelope[ptr].low;
				vtx.push_back(x);
				vtx.push_back((float)(level0-level*vScale/32768));
			}
		}
		renderer.DrawVtx(GL_LINE_STRIP,vtx.size()/2,vtx.data());
	}



	{
		const float col[4]={0,0,1,1};
		renderer.SetUniformColor(col);
		const float vtx[2*4]=
		{
			(float)0  ,  (float)hei/2,
			(float)wid,  (float)hei/2,
			(float)wid/2,(float)hei/2-vScale,
			(float)wid/2,(float)hei/2+vScale
		};
		renderer.DrawVtx(GL_LINES,4,vtx);
	}
	{
		const float col[4*4]={0,0,0,1};
		renderer.SetUniformColor(col);
		const float vtx[2*4]=
		{
			(float)0,  (float)(hei/2-vScale),
			(float)wid,(float)(hei/2-vScale),
			(float)wid,(float)(hei/2+vScale),
			(float)0,  (float)(hei/2+vScale),
		};
		renderer.DrawVtx(GL_LINE_LOOP,4,vtx);
	}



	{
#ifdef GL_PROGRAM_POINT_SIZE
		glEnable(GL_PROGRAM_POINT_SIZE);
#endif

		renderer.SetUniformPointSize(3.0f);

		std::vector <float> highVtx,lowVtx;
		auto &peak=wavEdit.GetPeak();
		if(viewport.wid<wid)
		{
			for(auto p : peak)
			{
				if(viewport.zero<=p.idx && 
				   p.idx<viewport.zero+viewport.wid &&
				   0<=p.idx && p.idx<wavRaw.GetNumSamplePerChannel())
				{
					int level=wavRaw.GetSignedValue16(channel,p.idx);
					long long x=(p.idx-viewport.zero)*wid/viewport.wid;
					if(true==p.isHigh)
					{
						highVtx.push_back(x);
						highVtx.push_back(level0-level*vScale/32768);
					}
					else
					{
						lowVtx.push_back(x);
						lowVtx.push_back(level0-level*vScale/32768);
					}
				}
			}
		}
		else
		{
			long long int peakPtr=0;
			while(peakPtr<peak.size() && peak[peakPtr].idx<viewport.zero)
			{
				++peakPtr;
			}
			for(long long x=0; x<wid; ++x)
			{
				long long int nextPtr=viewport.zero+(x+1)*viewport.wid/wid;

				bool hasHigh=false,hasLow=false;
				int high=-32768,low=32768;
				while(peakPtr<peak.size() && peak[peakPtr].idx<nextPtr)
				{
					if(true==peak[peakPtr].isHigh)
					{
						hasHigh=true;
						YsMakeGreater(high,wavRaw.GetSignedValue16(channel,peak[peakPtr].idx));
					}
					else
					{
						hasLow=true;
						YsMakeSmaller(low,wavRaw.GetSignedValue16(channel,peak[peakPtr].idx));
					}
					++peakPtr;
				}
				if(true==hasHigh)
				{
					highVtx.push_back(x);
					highVtx.push_back(level0-high*vScale/32768);
				}
				if(true==hasLow)
				{
					lowVtx.push_back(x);
					lowVtx.push_back(level0-low*vScale/32768);
				}
			}
		}
		renderer.SetUniformColor(0.0f,0.0f,1.0f,1.0f);
		renderer.DrawVtx(GL_POINTS,highVtx.size()/2,highVtx.data());
		renderer.SetUniformColor(1.0f,0.0f,0.0f,1.0f);
		renderer.DrawVtx(GL_POINTS,lowVtx.size()/2,lowVtx.data());
	}


	{
		std::vector <float> vtx;
		vtx.resize(wid*2);

		renderer.SetUniformColor(col.Rf(),col.Gf(),col.Bf(),1.0f);

		int nSample=0;
		for(long long int x=0; x<wid; ++x)
		{
			long long ptr=viewport.zero+x*viewport.wid/wid-offset;
			if(0<=ptr && ptr<wavRaw.GetNumSamplePerChannel())
			{
				int level=wavRaw.GetSignedValue16(channel,ptr);
				vtx[2*nSample  ]=(float)x;
				vtx[2*nSample+1]=(float)(level0-level*vScale/32768);
				++nSample;
			}
		}
		renderer.DrawVtx(GL_LINE_STRIP,nSample,vtx.data());
	}


	{
		std::vector <float> triVtx,lineVtx;

		renderer.SetUniformColor(1.0f,0.5f,0.5f,0.5f);
		auto &silentSeg=wavEdit.GetSilentSegment();
		for(auto seg : silentSeg)
		{
			if(seg.Max()<viewport.zero || viewport.zero+viewport.wid<seg.Min())
			{
				continue;
			}

			auto x0=YsGreater<long long>(seg.Max(),viewport.zero);
			auto x1=YsSmaller<long long>(seg.Min(),viewport.zero+viewport.wid);

			x0=waveRect.Min().x()+(long long)waveRect.GetWidth()*(x0-viewport.zero)/viewport.wid;
			x1=waveRect.Min().x()+(long long)waveRect.GetWidth()*(x1-viewport.zero)/viewport.wid;

			triVtx.push_back(x0); triVtx.push_back(waveRect.Min().y());
			triVtx.push_back(x1); triVtx.push_back(waveRect.Min().y());
			triVtx.push_back(x1); triVtx.push_back(waveRect.Max().y());

			triVtx.push_back(x0); triVtx.push_back(waveRect.Min().y());
			triVtx.push_back(x1); triVtx.push_back(waveRect.Max().y());
			triVtx.push_back(x0); triVtx.push_back(waveRect.Max().y());

			lineVtx.push_back(x0); lineVtx.push_back(waveRect.Min().y());
			lineVtx.push_back(x0); lineVtx.push_back(waveRect.Max().y());
		}

		renderer.DrawVtx(GL_TRIANGLES,triVtx.size()/2,triVtx.data());
		renderer.DrawVtx(GL_LINES,lineVtx.size()/2,lineVtx.data());
	}
}

YsRect2i FsGuiMainCanvas::GetWaveDrawingRect(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto vScale=hei/3;

	YsVec2i min,max;
	min.Set(  0,hei/2-vScale);
	max.Set(wid,hei/2+vScale);

	return YsRect2i(min,max);
}
