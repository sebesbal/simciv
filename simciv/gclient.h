#pragma once

#ifdef GDEBUG
	#include <stdio.h>
	#include <windows.h>
	#pragma warning(disable:4996)
	#include <math.h>
#endif

//res<<8|phase<<16|layer<<12|code

typedef unsigned char byte;

#define M_NONE  0x000//(0)
#define M_RESET 0x001//(0)
#define M_MAXPH 0x002//(0)
#define M_MINPH 0x003//(0)

#define M_TEXT  0x00C//(8(x, y, color, char[TEXTMAXLEN]))

#define M_BACK0 0x010//(0)
#define M_BACK1 0x011//(0)
#define M_BACK2 0x012//(0)
#define M_BACK3 0x013//(0)

#define M_TIL08 0x018//(4(x0, y0, xs|ys<<16, transp)+64)
#define M_TIL16 0x019//(4(x0, y0, xs|ys<<16, transp)+256)
#define M_TIL32 0x01A//(4(x0, y0, xs|ys<<16, transp)+1024)
#define M_TIL64 0x01B//(4(x0, y0, xs|ys<<16, transp)+4096)

#define M_PNT16 0x020//x|y<<16, size<<24|color (2)
#define M_LIN16 0x021//y0<<16|x0, y1<<16|x1, size<<24|color (3)
#define M_TRI16 0x022//y0<<16|x0, y1<<16|x1, y2<<16|x2, size<<24|color (4)
#define M_VEC16 0x023//y0<<16|x0, vy<<16|vx, size<<24|color (3)

#define M_PNT32 0x040//x, y, size<<24|color (3)
#define M_LIN32 0x041//x0, y0, x1, y1, size<<24|color (5)
#define M_TRI32 0x042//x0, y0, x1, y1, x2, y2, size<<24|color (7)
#define M_VEC32 0x043//x0, y0, vx, vy, size<<24|color (5)

////

const int NLAYER=15;
const int NBACKG=4;
const int TEXTMAXLEN=24;
// const int TEXTMAXLEN=256;
const int MEGA=0x100000;
const int BUFSIZE=12*MEGA;
const int IMESIZE=4*MEGA;

__inline int pack16(int x, int y) { return (x&0xFFFF)|y<<16; }

__inline int __d2f16(double d) { return int(d*65536.+.5); }

////

#ifdef GDEBUG
	DWORD WINAPI gdebug_thproc(LPVOID pprm);
#endif
class gdebug_client
{
public:
#ifdef GDEBUG
	HANDLE hmutex;
	HANDLE hmap;

	int* pobj;
	int* pimg;

	int locked;

	HANDLE hthread;
	int* pbuf;
	int nbuf;

	CRITICAL_SECTION cs;
#endif

public:
#ifdef GDEBUG
	gdebug_client() : hmutex(0), hmap(0), pobj(0), pimg(0), locked(0), hthread(0), pbuf(0), nbuf(0) {}
	~gdebug_client() { close(); }
#endif

	int create(const char* postfix="")
	{
#ifdef GDEBUG
		if (hmutex) return 0;

		//pbuf=(int*)malloc(BUFSIZE*sizeof(int));
		pbuf=new int[BUFSIZE];
		if (!pbuf) return 1;
		nbuf=0;

		char smut[0x80];
		char smap[0x80];
		sprintf(smut, "GDEBUG_M_%s", postfix);
		sprintf(smap, "GDEBUG_F_%s", postfix);

		LPTSTR smut1=(LPTSTR)smut;

		hmutex=OpenMutexA(MUTEX_ALL_ACCESS, 0, smut);
		if (!hmutex)
		{
			DWORD err=GetLastError();
			close(); return 2;
		}
		hmap=OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, smap);
		if (!hmap) { close(); return 3; }
		pobj=(int*)MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, BUFSIZE*sizeof(int));
		if (!pobj) { close(); return 4; }
		pimg=(int*)MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, BUFSIZE*sizeof(int), IMESIZE*sizeof(int));
		if (!pimg) { close(); return 5; }

		InitializeCriticalSection(&cs);
		hthread=CreateThread(0, 0, gdebug_thproc, this, 0, 0);
		if (!hthread) { close(); return 6; }
#endif
		return 0;
	}

	void close()
	{
#ifdef GDEBUG
		if (hthread)
		{
			EnterCriticalSection(&cs);
			TerminateThread(hthread, 0);//kesobb msg-vel??
			__flush();
			LeaveCriticalSection(&cs);
			DeleteCriticalSection(&cs);
			hthread=0;
		}

		if (pimg) { UnmapViewOfFile(pimg); pimg=0; }
		if (pobj) { UnmapViewOfFile(pobj); pobj=0; }
		if (hmap) { CloseHandle(hmap); hmap=0; }
		if (hmutex) { CloseHandle(hmutex); hmutex=0; }

		if (pbuf)
		{
			//free(pbuf);
			delete[] pbuf;
			pbuf=0; nbuf=0;
		}
#endif
	}

	void lock()
	{
#ifdef GDEBUG
		locked=1;
#endif
	}

	void unlock()
	{
#ifdef GDEBUG
		locked=0;
#endif
	}

	bool isvalid() const
	{
#ifdef GDEBUG
		return hmutex!=0;
#endif
	}

	void flush()
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;

		EnterCriticalSection(&cs);

		if (WaitForSingleObject(hmutex, 2000)==WAIT_OBJECT_0)
		{
			__flush();
			ReleaseMutex(hmutex);
//			Beep(1000, 200);
		}

		LeaveCriticalSection(&cs);
#endif
	}

private:
	void __flush()
	{
#ifdef GDEBUG
		if (nbuf)
		{
			if (*pobj+nbuf<BUFSIZE)
			{
				memcpy(pobj+*pobj, pbuf, nbuf*sizeof(int));
				*pobj+=nbuf;
			}
			nbuf=0;
		}
#endif
	}

#ifdef GDEBUG
	friend DWORD WINAPI gdebug_thproc(LPVOID pprm);
#endif

public:
	void clearbg(int index, int bgcolor=0)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (index<0||index>3) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			int x, y, *p=pimg+index*0x100000;
			for (y=0; y<0x400; y++)
			for (x=0; x<0x400; x++, p++)
			{
				*p=bgcolor;
			}

			pbuf[nbuf++]=(M_BACK0+index)|15<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void clearbg(int index, int x0, int y0, int xs, int ys, int bgcolor=0)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (index<0||index>3) return;
		if (x0<0||y0<0||x0+xs>1024||y0+ys>1024||xs<0||ys<0) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			int x, y, *p=pimg+(x0+y0*0x400)+index*0x100000;
			for (y=0; y<ys; y++)
			{
				for (x=0; x<xs; x++, p++)
				{
					*p=bgcolor;
				}
				p+=0x400-x;
			}

			pbuf[nbuf++]=(M_BACK0+index)|15<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void setbg(int index, int* bg, int x0, int y0, int xs, int ys, int pitch_on_src=0)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (index<0||index>3) return;
		if (x0<0||y0<0||x0+xs>1024||y0+ys>1024||xs<0||ys<0) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			int *g=bg;
			int x, y, *p=pimg+(x0+y0*0x400)+index*0x100000;
			for (y=0; y<ys; y++)
			{
				for (x=0; x<xs; x++, p++, g++) *p=*g;
				p+=0x400-x;
				if (pitch_on_src) g+=pitch_on_src-x;
			}
			pbuf[nbuf++]=(M_BACK0+index)|15<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void setbg(int index, byte* bg, int x0, int y0, int xs, int ys, int pitch_on_src=0)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (index<0||index>3) return;
		if (x0<0||y0<0||x0+xs>1024||y0+ys>1024||xs<0||ys<0) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			byte *g=bg;
			int x, y, *p=pimg+(x0+y0*0x400)+index*0x100000;
			for (y=0; y<ys; y++)
			{
				for (x=0; x<xs; x++, p++, g++) { *p=int(*g); *p|=*p<<8|*p<<16; }
				p+=0x400-x;
				if (pitch_on_src) g+=pitch_on_src-x;
			}

			pbuf[nbuf++]=(M_BACK0+index)|15<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

public:
	void reset(int layer)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			pbuf[nbuf++]=M_RESET|layer<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void resetall()
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;

		EnterCriticalSection(&cs);
		if (nbuf+NLAYER<=BUFSIZE)
		{
			for (int li=0; li<NLAYER; li++) pbuf[nbuf++]=M_RESET|li<<12;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void setmaxphase(int phase)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (phase<0 || phase>254) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			pbuf[nbuf++]=M_MAXPH|phase<<16;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void setminphase(int phase)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (phase<0 || phase>254) return;

		EnterCriticalSection(&cs);
		if (nbuf+1<=BUFSIZE)
		{
			pbuf[nbuf++]=M_MINPH|phase<<16;
		}
		LeaveCriticalSection(&cs);
#endif
	}

public:
	void point(int layer, int x, int y, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+3<=BUFSIZE)
		{
			pbuf[nbuf++]=M_PNT16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x, y);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void point(int layer, double x, double y, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+4<=BUFSIZE)
		{
			pbuf[nbuf++]=M_PNT32|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x);
			pbuf[nbuf++]=__d2f16(y);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void pointd(int layer, double x, double y, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		point(layer, x+.5, y+.5, color, size, phase);
#endif
	}

	void line(int layer, int x0, int y0, int x1, int y1, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+4<=BUFSIZE)
		{
			pbuf[nbuf++]=M_LIN16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x0, y0);
			pbuf[nbuf++]=pack16(x1, y1);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void line(int layer, double x0, double y0, double x1, double y1, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+6<=BUFSIZE)
		{
			pbuf[nbuf++]=M_LIN32|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x0);
			pbuf[nbuf++]=__d2f16(y0);
			pbuf[nbuf++]=__d2f16(x1);
			pbuf[nbuf++]=__d2f16(y1);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void lined(int layer, double x0, double y0, double x1, double y1, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		line(layer, x0+.5, y0+.5, x1+.5, y1+.5, color, size, phase);
#endif
	}

	//later> nativ type
	void rect(int layer, int x0, int y0, int x1, int y1, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+4*4<=BUFSIZE)
		{
			pbuf[nbuf++]=M_LIN16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x0, y0);
			pbuf[nbuf++]=pack16(x1, y0);
			pbuf[nbuf++]=color|size<<24;
			pbuf[nbuf++]=M_LIN16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x1, y0);
			pbuf[nbuf++]=pack16(x1, y1);
			pbuf[nbuf++]=color|size<<24;
			pbuf[nbuf++]=M_LIN16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x1, y1);
			pbuf[nbuf++]=pack16(x0, y1);
			pbuf[nbuf++]=color|size<<24;
			pbuf[nbuf++]=M_LIN16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x0, y1);
			pbuf[nbuf++]=pack16(x0, y0);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void vector(int layer, int x0, int y0, int vx, int vy, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+4<=BUFSIZE)
		{
			pbuf[nbuf++]=M_VEC16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x0, y0);
			pbuf[nbuf++]=pack16(vx, vy);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void vector(int layer, double x0, double y0, double vx, double vy, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+6<=BUFSIZE)
		{
			pbuf[nbuf++]=M_VEC32|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x0);
			pbuf[nbuf++]=__d2f16(y0);
			pbuf[nbuf++]=__d2f16(vx);
			pbuf[nbuf++]=__d2f16(vy);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void tri(int layer, int x0, int y0, int x1, int y1, int x2, int y2, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+5<=BUFSIZE)
		{
			pbuf[nbuf++]=M_TRI16|layer<<12|phase<<16;
			pbuf[nbuf++]=pack16(x0, y0);
			pbuf[nbuf++]=pack16(x1, y1);
			pbuf[nbuf++]=pack16(x2, y2);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void tri(int layer, double x0, double y0, double x1, double y1, double x2, double y2, int color, int size=0, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+8<=BUFSIZE)
		{
			pbuf[nbuf++]=M_TRI32|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x0);
			pbuf[nbuf++]=__d2f16(y0);
			pbuf[nbuf++]=__d2f16(x1);
			pbuf[nbuf++]=__d2f16(y1);
			pbuf[nbuf++]=__d2f16(x2);
			pbuf[nbuf++]=__d2f16(y2);
			pbuf[nbuf++]=color|size<<24;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void tile(int layer, double x0, double y0, int* pi, int xs, int ys, int pt=0, int transp=-1, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+4100<=BUFSIZE)
		{
			if (!pt) pt=xs;

			int L=max(xs, ys);
			if (L>32) L=64; else if (L>16) L=32; else if (L>8) L=16; else L=8;

			pbuf[nbuf++]=(L==8?M_TIL08:L==16?M_TIL16:L==32?M_TIL32:M_TIL64)|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x0);
			pbuf[nbuf++]=__d2f16(y0);
			pbuf[nbuf++]=pack16(min(xs, L), min(ys, L));
			pbuf[nbuf++]=transp;

			int x, y;
			for (y=0; y<L; y++) for (x=0; x<L; x++)
			{
				int color=x<xs&&y<ys?pi[x+y*pt]:-1;
				pbuf[nbuf++]=color;
			}
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void tile(int layer, int x0, int y0, int* pi, int xs, int ys, int pt=0, int transp=-1, int phase=255)
	{
#ifdef GDEBUG
		tile(layer, x0<<16, y0<<16, pi, xs, ys, pt, transp, phase);
#endif
	}

	void text(int layer, double x, double y, int color, const char* s, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+9<=BUFSIZE)
		{
			pbuf[nbuf++]=M_TEXT|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x);
			pbuf[nbuf++]=__d2f16(y);
			pbuf[nbuf++]=color;

			char* ps=(char*)(pbuf+nbuf);
			for (int ks=0, ns=(int)strlen(s); ks<TEXTMAXLEN; ks++) ps[ks]=ks<ns?s[ks]:0;

			nbuf+=5;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void text(int layer, double x, double y, int color, std::string s, int phase=255)
	{
#ifdef GDEBUG
		if (!hmutex || locked) return;
		if (layer<0||layer>=NLAYER) return;
		if (phase&0xFFFFFF00) return;

		EnterCriticalSection(&cs);
		if (nbuf+9<=BUFSIZE)
		{
			pbuf[nbuf++]=M_TEXT|layer<<12|phase<<16;
			pbuf[nbuf++]=__d2f16(x);
			pbuf[nbuf++]=__d2f16(y);
			pbuf[nbuf++]=color;

			char* ps=(char*)(pbuf+nbuf);
			for (int ks=0, ns=s.length(); ks<TEXTMAXLEN; ks++) ps[ks]=ks<ns?s[ks]:0;

			nbuf+=5;
		}
		LeaveCriticalSection(&cs);
#endif
	}

	void text(int layer, int x, int y, int color, const char* s, int phase=255)
	{
#ifdef GDEBUG
		text(layer, double(x), double(y), color, s, phase);
#endif
	}

	void textnum(int layer, double x, double y, int color, int n, int phase=255)
	{
#ifdef GDEBUG
		char s[0x100];
		sprintf(s, "%d", n);
		if (strlen(s)+1>=TEXTMAXLEN) return;
		text(layer, x, y, color, s, phase);
#endif
	}

	void textval(int layer, double x, double y, int color, double v, int phase=255)
	{
#ifdef GDEBUG
		char s[0x100];
		if (fabs(v)<1e10) sprintf(s, "%f", v); else sprintf(s, "bignum");
		if (strlen(s)+1>=TEXTMAXLEN) return;
		text(layer, x, y, color, s, phase);
#endif
	}

};

#ifdef GDEBUG
__inline DWORD WINAPI gdebug_thproc(LPVOID pprm)
{
	gdebug_client *pmc=(gdebug_client*)pprm;

	while (1)
	{
		EnterCriticalSection(&pmc->cs);
		if (pmc->nbuf) if (WaitForSingleObject(pmc->hmutex, 2000)==WAIT_OBJECT_0)
		{
			pmc->__flush();
			ReleaseMutex(pmc->hmutex);
//			Beep(1000, 200);
		}
		LeaveCriticalSection(&pmc->cs);

		Sleep(200);
	}

	return 0;
}
#endif
