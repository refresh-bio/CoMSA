// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <iostream>
#include "rle.h"

// *******************************************************************************************
// Perform RLE-0 coding
void CRLE::forward()
{
	string src, dest;
	uint64_t priority;

	int n_vec = 0;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		dest.clear();

		src.push_back((char) 127);		// sentinel
		int zero_len = 0;
//		int one_len = 0;
		int p_x = -1;

		int enc_len = 0;

		for (auto x : src)
		{
			if (x != p_x)
			{
				if (zero_len)
				{
					enc_len += zero_len;
					emit_code(dest, zero_len, 125);
					zero_len = 0;
				}
/*				else if (one_len)
				{
					emit_code(dest, one_len, 123);
					one_len = 0;
				}*/
			}

			if (x == 0)
				++zero_len;
//			else if (x == 1)
//				++one_len;
			else
			{
				dest.push_back(x);
				++enc_len;
			}

			p_x = x;
		}

		n_vec++;

		dest.pop_back();		// remove sentinel
		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Perform RLE-0 forward copy - just for debug purposes
void CRLE::copy_forward()
{
	string src, dest;
	uint64_t priority;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		dest.clear();

		for (auto x : src)
			if (x == 0)
				dest.push_back(1);
			else
				dest.push_back(x+1);

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Perform RLE-0 decoding
void CRLE::reverse()
{
	string src, dest;
	uint64_t priority;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		dest.clear();
		src.push_back((char) 127);			// sentinel

		int zero_len = 0;
		int zero_code = 0;
		int zero_code_no_bits = 0;
//		int one_len = 0;
//		int p_x = -1;

		for (auto x : src)
		{
			if (x == 125 || x == 126)
			{
				if (zero_code_no_bits == 0)
					zero_code = 0;

				if (x == 126)
					zero_code += 1 << zero_code_no_bits;
				++zero_code_no_bits;
			}
			else
			{
				if (zero_code_no_bits)
				{
					zero_len = zero_code + (1 << zero_code_no_bits) - 1;
					for (int i = 0; i < zero_len; ++i)
						dest.push_back(0);
					zero_code_no_bits = 0;
				}

				dest.push_back(x);
			}
		}

		dest.pop_back();		// remove sentinel
		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Perform RLE-0 reverse copy - just for debug purposes
void CRLE::copy_reverse()
{
	string src, dest;
	uint64_t priority;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		dest.clear();

		for (auto x : src)
			if (x == 1)
				dest.push_back(0);
			else
				dest.push_back(x-1);

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Emit code for 0-run
void inline CRLE::emit_code(string &dest, int cnt, int offset)
{
	for (++cnt; cnt != 1; cnt >>= 1)
		dest.push_back(offset + (cnt & 1));
}

// *******************************************************************************************
// Do processing
void CRLE::operator()()
{
	if (stage_mode == stage_mode_t::forward)
		forward();
	else if (stage_mode == stage_mode_t::reverse)
		reverse();
	else if (stage_mode == stage_mode_t::copy_forward)
		copy_forward();
	else if (stage_mode == stage_mode_t::copy_reverse)
		copy_reverse();
}

// EOF
