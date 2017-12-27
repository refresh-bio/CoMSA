// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <iostream>
#include <numeric>
#include "pbwt.h"

// *******************************************************************************************
// Perform gPBWT 
void CPBWT::forward()
{
	string src, dest;
	uint64_t priority;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;
		if (dest.empty())
			dest.resize(src.size());

		// Set initial ordering for the last column
		if (prev_ordering.empty())
		{
			prev_ordering.resize(src.size());
			iota(prev_ordering.begin(), prev_ordering.end(), 0);
			curr_ordering.resize(src.size());
		}

		// Build histogram
		vector<int> n_occ(128, 0);

		for (auto c : src)
			++n_occ[c];

		vector<int> n_sum_occ(128, 0);
		for (int i = 1; i < 128; ++i)
			n_sum_occ[i] = n_sum_occ[i - 1] + n_occ[i - 1];

		// Determine new ordering
		for (size_t i = 0; i < prev_ordering.size(); ++i)
		{
			int c_symbol = src[prev_ordering[i]];
			int c_pos = n_sum_occ[c_symbol]++;

			curr_ordering[c_pos] = prev_ordering[i];
			dest[i] = c_symbol;
		}

		prev_ordering.swap(curr_ordering);

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Do reverse gPBWT
void CPBWT::reverse()
{
	string src, dest;
	uint64_t priority;

	int n_vec = 0;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;
		if (dest.empty())
			dest.resize(src.size());

		++n_vec;

		// Set initail ordering
		if (prev_ordering.empty())
		{
			prev_ordering.resize(src.size());
			iota(prev_ordering.begin(), prev_ordering.end(), 0);
			curr_ordering.resize(src.size());
		}

		// Build histogram
		vector<int> n_occ(128, 0);

		// Permute
		for (size_t i = 0; i < prev_ordering.size(); ++i)
			dest[prev_ordering[i]] = src[i];

		for (auto c : dest)
			++n_occ[c];

		vector<int> n_sum_occ(128, 0);
		for (int i = 1; i < 128; ++i)
			n_sum_occ[i] = n_sum_occ[i - 1] + n_occ[i - 1];

		// Determine new ordering
		for (size_t i = 0; i < prev_ordering.size(); ++i)
		{
			int c_symbol = dest[prev_ordering[i]];
			int c_pos = n_sum_occ[c_symbol]++;

			curr_ordering[c_pos] = prev_ordering[i];
		}

		prev_ordering.swap(curr_ordering);

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Do processing
void CPBWT::operator()()
{
	if (forward_mode)
		forward();
	else
		reverse();
}

// EOF
