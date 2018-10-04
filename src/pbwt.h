#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <vector>
#include <string>
#include <cstdio>

#include "queue.h"
#include "defs.h"

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
class CPBWT 
{
	CRegisteringPriorityQueue<string> *in;
	CRegisteringPriorityQueue<string> *out;
	stage_mode_t stage_mode;

	vector<int> prev_ordering;
	vector<int> curr_ordering;

	void forward();
	void reverse();
	void direct_copy();

public:
	CPBWT(CRegisteringPriorityQueue<string> *_in, CRegisteringPriorityQueue<string> *_out, stage_mode_t _stage_mode) : 
		in(_in), out(_out), stage_mode(_stage_mode)
	{
		if (!in || !out)
			throw "No I/O queues";
	};

	void operator()();
};

// EOF
