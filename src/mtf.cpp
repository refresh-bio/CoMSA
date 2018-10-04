// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <iostream>
#include "mtf.h"
#include <numeric>
#include <cmath>

// *******************************************************************************************
// Constructor
CMTFCore::CMTFCore()
{
	mtf1_variant = true;

	v_sym_pos.resize(256, -1);
}

// *******************************************************************************************
// Destructor
CMTFCore::~CMTFCore()
{
}

// *******************************************************************************************
// Return current value for a symbol
int inline CMTFCore::find_pos(int x)
{
	return v_sym_pos[x];
}

// *******************************************************************************************
//
void CMTFCore::Reset()
{
}

// *******************************************************************************************
// Reset to initial ordering of symbols
void CMTFCore::ResetCounts(uint32_t vec_size)
{
	v = v_init;
	v_sym_pos = v_sym_pos_init;
}

// *******************************************************************************************
// Insert single symbol to the valid symbols
void CMTFCore::InitSymbol(int x)
{
	v.push_back(x);
	v_sym_pos[x] = (int) v.size() - 1;
}

// *******************************************************************************************
// Insert vector of legal symbols
void CMTFCore::InitSymbols(const vector<int> &v_legal_symbols)
{
	v.clear();
	for (auto c : v_legal_symbols)
		InitSymbol(c);

	v_init = v;
	v_sym_pos_init = v_sym_pos;
}

// *******************************************************************************************
// Update MTF list
void CMTFCore::Insert(int x)
{
	int p = find_pos(x);

	if (p != 0)
		move_up(p);
}

// *******************************************************************************************
// Move symbol to the begining of the list
inline void CMTFCore::move_up(int x)
{
	int sym = v[x];

	if (mtf1_variant)			// MTF-1 variant (only symbols from pos. 1 are moved to pos. 0; the rest are moved to pos. 1)
	{
		if (x == 1)
		{
			v[1] = v[0];
			v[0] = sym;
			v_sym_pos[v[0]] = 0;
			v_sym_pos[v[1]] = 1;
		}
		else
		{
			for (int i = x; i > 1; --i)
			{
				v[i] = v[i - 1];
				v_sym_pos[v[i]] = i;
			}

			v[1] = sym;
			v_sym_pos[sym] = 1;
		}
	}
	else						// Classic MTF
	{
		for (int i = x; i > 0; --i)
		{
			v[i] = v[i - 1];
			v_sym_pos[v[i]] = i;
		}

		v[0] = sym;
		v_sym_pos[sym] = 0;
	}
}

// *******************************************************************************************
// Return position of symbol in the list
inline int CMTFCore::GetValue(int x)
{
	return find_pos(x);
}

// *******************************************************************************************
// Gest symbol from the given position
inline int CMTFCore::GetSymbol(int x)
{
	return v[x];
}

// *******************************************************************************************
// Check whether a symbol is valid
inline bool CMTFCore::IsPresent(int x)
{
	return find_pos(x) >= 0;

	return false;
}

// *******************************************************************************************
// Return size of the MTF list
inline int CMTFCore::Size()
{
	return (int) v.size();
}


// *******************************************************************************************
// 
// *******************************************************************************************

// *******************************************************************************************
// Do MTF coding
void CMTF::forward()
{
	string src, dest;
	uint64_t priority;

	mtf_core->InitSymbols(v_legal_symbols);

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		mtf_core->ResetCounts((uint32_t) src.size());

		dest.clear();
		dest.resize(src.size());
		uint32_t pos = 0;

		for (auto c : src)
		{
			int x = mtf_core->GetValue(c);
			mtf_core->Insert(c);

			dest[pos++] = x;
		}

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Direct copy - just for debugging
void CMTF::direct_copy()
{
	string src;
	uint64_t priority;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		out->Push(priority, src);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Do MTF decoding
void CMTF::reverse()
{
	string src, dest;
	uint64_t priority;

	mtf_core->InitSymbols(v_legal_symbols);

	int n_vec = 0;

	while (!in->IsCompleted())
	{
		if (!in->Pop(priority, src))
			continue;

		++n_vec;

		mtf_core->ResetCounts((uint32_t) src.size());

		dest.clear();
		dest.resize(src.size());
		uint32_t pos = 0;

		for (auto x : src)
		{
			int c = mtf_core->GetSymbol(x);
			mtf_core->Insert(c);

			//			dest.push_back(c);
			dest[pos++] = c;
		}

		out->Push(priority, dest);
	}

	out->MarkCompleted();
}

// *******************************************************************************************
// Run processing
void CMTF::operator()()
{
	v_legal_symbols.clear();

	v_legal_symbols.push_back('-');
	v_legal_symbols.push_back('.');
	for (int c = 'A'; c <= 'Z'; ++c)
		v_legal_symbols.push_back(c);
	for (int c = 'a'; c <= 'z'; ++c)
		v_legal_symbols.push_back(c);
	v_legal_symbols.push_back('*');

	for (int i = 0; i < 128; ++i)
		if (count(v_legal_symbols.begin(), v_legal_symbols.end(), i) == 0)
			v_legal_symbols.push_back(i);

	if (stage_mode == stage_mode_t::forward)
		forward();
	else if(stage_mode == stage_mode_t::reverse)
		reverse();
	else if (stage_mode == stage_mode_t::copy_forward && stage_mode == stage_mode_t::copy_reverse)
		direct_copy();
}

// EOF
