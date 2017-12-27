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
#include "queue.h"

// *******************************************************************************************
//
// *******************************************************************************************
class CRLE
{
	CRegisteringPriorityQueue<string> *in;
	CRegisteringPriorityQueue<string> *out;
	bool forward_mode;

	void forward();
	void reverse();

	void emit_code(string &dest, int cnt, int offset);

public:
	CRLE(CRegisteringPriorityQueue<string> *_in, CRegisteringPriorityQueue<string> *_out, bool _forward_mode) : 
		in(_in), out(_out), forward_mode(_forward_mode)
	{
		if (!in || !out)
			throw "No I/O queues";
	};

	~CRLE()
	{
	}

	void operator()();
};

// EOF
