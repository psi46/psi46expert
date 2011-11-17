#ifndef __EVENT_DECODER_H__
#define __EVENT_DECODER_H__

#include "BasePixel/psi46_tb.h"
#include "BasePixel/RawPacketDecoder.h"
#include "pipe.h"

#include <TH2I.h>

typedef long long int uint64_t;

/* Pipe Objects -------------------------------------------------------- */

class PipeObjectShort : public PipeObject {
	public:
		short s;
};

#define MAXDATASIZE 10000

class CRawEvent : public PipeObject {
	public:
		unsigned int flags;
		uint64_t time;
		unsigned int length;
		short data[MAXDATASIZE];
		unsigned int dflag[MAXDATASIZE];

		bool IsHeaderOk()  { return (flags & 0xff00) == 0x8000; }
		bool IsData()      { return (flags & 0x01) != 0; }
		bool IsTrigger()   { return (flags & 0x04) != 0; }
		bool IsRocReset()  { return (flags & 0x08) != 0; }
		bool IsCalibrate() { return (flags & 0x10) != 0; }
		bool IsOverflow()  { return (flags & 0x80) != 0; }

		void PrintEventFlags();
		void PrintTime();
		void PrintDataFlags(unsigned int index);
		void PrintData();
		void Print();
};

class CEvent : public PipeObject {
	public:
		bool isData;
		bool isTrigger;
		bool isReset;
		bool isCalibrate;
		bool isOverflow;
		uint64_t timestamp;
		unsigned int nRocs;
		int nHits;
		DecodedReadoutModule hits;
		
	public:
		void print();
};

/* Pipes --------------------------------------------------------------- */

class RAMRawDataReader : public Pipe {
	CTestboard * board;
	unsigned int datastart;
	unsigned int dataend;
	unsigned int databuffersize;
	unsigned int dataptr;
	unsigned int buffersize;
	unsigned int bufferpos;
	
	unsigned short * buffer;
	PipeObjectShort s;

	public:
		PipeObjectShort * Write();
		RAMRawDataReader(CTestboard * b, unsigned int ramstart, unsigned int ramend, unsigned int datalength);
		~RAMRawDataReader();
};

class RawData2RawEvent : public Pipe {
	protected:
		CRawEvent rawevent;
		PipeObjectShort * Read();
		CRawEvent * Write();
};

class RawEventDecoder : public Pipe {
	public:
		CEvent decoded_event;
		CRawEvent * Read();
		CEvent * Write();
		unsigned int nROCs;
		
		RawEventDecoder(unsigned int nROCs);
};

class HitMapper : public Pipe {
	TH2I * hitmap;
	
	public:
	HitMapper();
	~HitMapper();
	CEvent * Read();
	CEvent * Write();
	TH2I * getHitMap(unsigned int iroc);
};

class EfficiencyMapper : public Pipe {
	TH2I * effmap;
	TH2I * bkgmap;
	TH1I * effdist;
	unsigned int TrigPerPixel;
	unsigned int EventCounter;
	
	public:
	EfficiencyMapper(unsigned int ntrig);
	~EfficiencyMapper();
	CEvent * Read();
	CEvent * Write();
	TH2I * getEfficiencyMap(unsigned int iroc);
	TH1I * getEfficiencyDist(unsigned int iroc);
	TH2I * getBackgroundMap(unsigned int iroc);
	void setTrigPerPixel(int ntrig);
};

class EventCounter : public Pipe {
	private:
		bool haveTrigger;
	public:
		int TriggerCounter;
		int RocSequenceErrorCounter;
	public:
		CEvent * Read();
		CEvent * Write();
		EventCounter();
};

class MultiplicityHistogrammer : public Pipe {
	protected:
		TH1I * ModuleMultiplicity;
		TH1I * RocMultiplicity [16];
		TH1I * DColMultiplicity [16][26];
		int nRocs;
	public:
		CEvent * Read();
		CEvent * Write();
		MultiplicityHistogrammer();
		~MultiplicityHistogrammer();
		TH1I * getModuleMultiplicity();
		TH1I * getRocMultiplicity(int roc);
		TH1I * getDColMultiplicity(int roc, int dcol);
};

class PulseHeightHistogrammer : public Pipe {
	protected:
		TH1I * pulseheight;
	public:
		CEvent * Read();
		CEvent * Write();
		PulseHeightHistogrammer();
		~PulseHeightHistogrammer();
		TH1I * getPulseHeightHistogram();
};

#endif
