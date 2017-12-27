#pragma once
// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <vector>
#include <algorithm>
#include "ss.h"

using namespace std;

#pragma once

#include <vector>
#include <algorithm>
#include <array>
#include "queue.h"

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
class CMTFCore
{
	vector<int> v, v_init;
	vector<int> v_sym_pos, v_sym_pos_init;
	int mtf1_variant;

	int find_pos(int x);

	inline void move_up(int x);

public:
	CMTFCore();
	~CMTFCore();

	void Reset();
	void ResetCounts(uint32_t vec_size);
	void InitSymbol(int x);
	void InitSymbols(vector<int> &v_legal_symbols);
	void Insert(int x);
	int GetValue(int x);
	int GetSymbol(int x);
	bool IsPresent(int x);
	int Size();
};

// *******************************************************************************************
//
// *******************************************************************************************
class CMTF : public CSecondStage
{
	CRegisteringPriorityQueue<string> *in;
	CRegisteringPriorityQueue<string> *out;
	bool forward_mode;

	CMTFCore *mtf_core;
	vector<int> v_legal_symbols;

	int func_id;

	void forward();
	void reverse();

public:
	CMTF(CRegisteringPriorityQueue<string> *_in, CRegisteringPriorityQueue<string> *_out, bool _forward_mode) :
		in(_in), out(_out), forward_mode(_forward_mode)
	{
		if (!in || !out)
			throw "No I/O queues";

		mtf_core = new CMTFCore();
	};

	virtual ~CMTF()
	{
		if (mtf_core)
			delete mtf_core;
	}

	virtual void operator()();
};

// EOF
