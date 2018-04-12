// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.1
// Date   : 2018-04-12
// *******************************************************************************************

#include "transpose.h"
#include <iostream>
#include <algorithm>
#include <xmmintrin.h>

// *******************************************************************************************
// Perform transposition of a matrix
void CTranspose::forward()
{
	uint64_t priority = 0;
	vector<string>* v_sequences = nullptr;
	matrix->Pop(priority, v_sequences);

	size_t in_n_columns = v_sequences->front().size();
	size_t in_n_rows = v_sequences->size();

	// Check whether all sequences are of the same length
	for (auto &x : *v_sequences)
		if (x.size() != in_n_columns)
		{
			cerr << "Sequences are of different lengths\n";
			exit(1);
		}

	const int BLOCK_SIZE = 64;
	const int PREFETCH_STEP = 32;

	vector<string> v_str(BLOCK_SIZE);

	for (auto &x : v_str)
		x.resize(in_n_rows);

	for (int i = (int) in_n_columns - 1; i >= 0; i -= BLOCK_SIZE)
	{
		int i_end = max(i - BLOCK_SIZE, -1);

		for (size_t j = 0; j < in_n_rows; ++j)
		{
			if (j + PREFETCH_STEP < in_n_rows)
				_mm_prefetch((char*)(*v_sequences)[j + PREFETCH_STEP].data() + i, _MM_HINT_T0);

			for (int ii = i; ii > i_end; --ii)
				v_str[ii % BLOCK_SIZE][j] = (*v_sequences)[j][ii];
		}

		for (int ii = i; ii > i_end; --ii)
			in_out->Push(priority++, v_str[ii % BLOCK_SIZE]);
	}

	in_out->MarkCompleted();
}

// *******************************************************************************************
// Perform reverse transposition of a matrix
void CTranspose::reverse()
{
	string src;
	uint64_t priority;
	vector<string>* v_sequences = new vector<string>();

	v_sequences->resize(n_sequences);

	for (size_t i = 0; i < n_sequences; ++i)
		(*v_sequences)[i].resize(n_columns);

	const int BLOCK_SIZE = 64;
	const int PREFETCH_STEP = 32;

	int i_end = (int) n_columns;
	int i = i_end - 1;

	vector<string> v_src(BLOCK_SIZE);

	while (!in_out->IsCompleted())
	{
		if (!in_out->Pop(priority, src))
			continue;

		v_src[i % BLOCK_SIZE] = move(src);

		if (i % BLOCK_SIZE == 0)
		{
			for (size_t j = 0; j < n_sequences; ++j)
			{
				if (j + PREFETCH_STEP < n_sequences)
					_mm_prefetch((char*)(*v_sequences)[j + PREFETCH_STEP].data() + i, _MM_HINT_T0);

				for (int ii = i; ii < i_end; ++ii)
					(*v_sequences)[j][ii] = v_src[ii % BLOCK_SIZE][j];

			}
			i_end = i;
		}

		--i;
	}

	matrix->Push(0, v_sequences);
	matrix->MarkCompleted();
}

// *******************************************************************************************
// Do processing
void CTranspose::operator()()
{
	if (forward_mode)
		forward();
	else
		reverse();
}

// EOF