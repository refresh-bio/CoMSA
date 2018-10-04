#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <string>
#include <vector>
#include "defs.h"
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
	stage_mode_t stage_mode;

	void forward();
	void reverse();
	void copy_forward();
	void copy_reverse();

public:
	CTranspose(CRegisteringPriorityQueue<vector<string>*> *_matrix, CRegisteringPriorityQueue<string> *_in_out, size_t _n_sequences, size_t _n_columns, stage_mode_t _stage_mode) :
		matrix(_matrix), in_out(_in_out), n_sequences(_n_sequences), n_columns(_n_columns), stage_mode(_stage_mode)
	{
		if (!matrix || !in_out)
			throw "No I/O queues";
	}

	void operator()();
};

// EOF
