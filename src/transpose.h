#pragma once
// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <string>
#include <vector>
#include "queue.h"

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
class CTranspose
{
	CRegisteringPriorityQueue<vector<string>*> *matrix;
	CRegisteringPriorityQueue<string> *in_out;
	size_t n_sequences;
	size_t n_columns;
	bool forward_mode;

	void forward();
	void reverse();

public:
	CTranspose(CRegisteringPriorityQueue<vector<string>*> *_matrix, CRegisteringPriorityQueue<string> *_in_out, size_t _n_sequences, size_t _n_columns, bool _forward_mode) :
		matrix(_matrix), in_out(_in_out), n_sequences(_n_sequences), n_columns(_n_columns), forward_mode(_forward_mode)
	{
		if (!matrix || !in_out)
			throw "No I/O queues";
	}

	void operator()();
};

// EOF
