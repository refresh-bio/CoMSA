#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include "defs.h"
#include "sub_rc.h"
#include <cmath>

// *******************************************************************************************
//
// *******************************************************************************************
class CSimpleModel
{
	uint32_t n_symbols;
	uint32_t max_total;
	uint32_t *stats;
	uint32_t total;

	void rescale()
	{
		total = 0;
		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			stats[i] = (stats[i] + 1) / 2;
			total += stats[i];
		}
	}

public: 
	CSimpleModel() : n_symbols(0), stats(nullptr)
	{};

	~CSimpleModel()
	{
		if (stats)
			delete[] stats;
	};

	void Init(uint32_t _n_symbols, uint32_t _max_total)
	{
		if (stats)
			delete[] stats;
		n_symbols = _n_symbols;
		max_total = _max_total;

		stats = new uint32_t[n_symbols];
		fill_n(stats, n_symbols, 1);
		total = n_symbols;
	}

	void GetFreq(int symbol, int &sym_freq, int &left_freq, int &totf)
	{
		left_freq = 0;

		for (int i = 0; i < symbol; ++i)
			left_freq += stats[i];

		sym_freq = stats[symbol];
		totf = total;
	}

	void Update(int symbol)
	{
		stats[symbol]++;
		total++;

		if (total >= max_total)
			rescale();
	}

	int GetSym(int left_freq)
	{
		int t = 0;

		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			t += stats[i];
			if (t > left_freq)
				return i;
		}

		return -1;
	}

	uint32_t GetTotal()
	{
		return total;
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM> class CRangeCoderModel
{
	CRangeEncoder<CVectorIOStream> *rce;
	CRangeDecoder<CVectorIOStream> *rcd;

	CSimpleModel simple_model;

	int no_symbols;
	int lg_totf;
	int totf;
	int rescale;
	bool compress;

public:
	CRangeCoderModel(CBasicRangeCoder<T_IO_STREAM> *rcb, int _no_symbols, int _lg_totf, int _rescale, int* _init, bool _compress) :
		no_symbols(_no_symbols), lg_totf(_lg_totf), totf(1 << _lg_totf), rescale(_rescale), compress(_compress)
	{
		simple_model.Init(no_symbols, rescale);

		if (compress)
		{
			rce = (CRangeEncoder<T_IO_STREAM>*) (rcb);
			rcd = nullptr;
		}
		else
		{
			rce = nullptr;
			rcd = (CRangeDecoder<T_IO_STREAM>*) (rcb);
		}
	}

	~CRangeCoderModel()
	{
	}

	void Reset()
	{
		simple_model.Init(no_symbols, rescale);
	}

	void Encode(int x)
	{
		int syfreq, ltfreq;
		simple_model.GetFreq(x, syfreq, ltfreq, totf);
		rce->EncodeFrequency(syfreq, ltfreq, totf);
		simple_model.Update(x);
	}

	int Decode()
	{
		int syfreq, ltfreq;

		totf = simple_model.GetTotal();
		ltfreq = rcd->GetCumulativeFreq(totf);

		int x = simple_model.GetSym(ltfreq);

		simple_model.GetFreq(x, syfreq, ltfreq, totf);
		rcd->UpdateFrequency(syfreq, ltfreq, totf);
		simple_model.Update(x);

		return x;
	}
};

// EOF
