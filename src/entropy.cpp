// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <iostream>
#include <unordered_map>
#include "entropy.h"

// *******************************************************************************************
// Entropy coding of the columns
void CEntropy::forward()
{
	string src;
	uint64_t priority;

	init_rc();

	rce->Start();

	int ctx_prefix = no_prefix_ctx - 1;
	int ctx_sel;

	*pre_entropy_sequences_size = 0;

	while (!in_out->IsCompleted())
	{
		if (!in_out->Pop(priority, src))
			continue;

		ctx_sel = no_selector_ctx - 1;
		ctx_prefix = no_prefix_ctx - 1;

		for (auto x : src)
		{
			// Prefix selection: 0a(125) -> 0, 0b(126) -> 1, 1 -> 2, reszta -> 3
			int prefix = (x == 125) ? 0 : (x == 126) ? 1 : (x == 1) ? 2 : 3;
			//			int prefix = (x == 125) ? 0 : (x == 126) ? 1 : (x == 123) ? 2 : (x == 124) ? 3 : 4;

			rc_prefix[ctx_prefix]->Encode(prefix);
			ctx_prefix = ctx_update_prefix(ctx_prefix, prefix);

			if (prefix < 3)
				continue;

			int selector = ilog2(x);
			int suffix = x - (1 << (selector - 1));

			rc_selector[ctx_sel]->Encode(selector - 2);

			ctx_sel = ctx_update_selector(ctx_sel, selector - 2);

			rc_suffix[ctx_sel % no_suffix_ctx]->Encode(suffix);
		}

		*pre_entropy_sequences_size += src.size();
	}

	rce->End();

	delete_rc();
}

// *******************************************************************************************
// Entropy decoding
void CEntropy::reverse()
{
	string dest;
	uint64_t priority = 0;

	init_rc();

	rcd->Start();

	int n_vec = 0;
	int ctx_prefix = no_prefix_ctx - 1;
	int ctx_sel = no_selector_ctx - 1;

	size_t decoded_symbols = 0;
	size_t cur_column_decoded_symbols = 0;

	size_t zero_run_code = 0;		// It is necessary to decode 0-runs to find the column boundary
	size_t zero_run_code_no_bits = 0;

	while (decoded_symbols < *pre_entropy_sequences_size)
	{
		if (cur_column_decoded_symbols == n_sequences)
		{
			ctx_prefix = no_prefix_ctx - 1;
			ctx_sel = no_selector_ctx - 1;

			++n_vec;

			in_out->Push(priority++, dest);
			dest.clear();

			cur_column_decoded_symbols = 0;
		}
		else if (cur_column_decoded_symbols > n_sequences)
			assert("Decoded too many\n");

		int prefix = rc_prefix[ctx_prefix]->Decode();

		ctx_prefix = ctx_update_prefix(ctx_prefix, prefix);
		int x;

		if (prefix > 1 && zero_run_code_no_bits)		// In case of 0-run we need to add its length
		{
			size_t zero_run_len = zero_run_code + (1ull << zero_run_code_no_bits) - 1;
			cur_column_decoded_symbols += zero_run_len;

			zero_run_code = 0;
			zero_run_code_no_bits = 0;
		}

		if (prefix < 3)
		{
			if (prefix == 2)
			{
				x = 1;
				++cur_column_decoded_symbols;
			}
			else
			{
				if (prefix == 1)
					x = 126;
				else
					x = 125;

				if (!zero_run_code_no_bits)
					zero_run_code = 0;

				++zero_run_code_no_bits;
				zero_run_code += prefix << (zero_run_code_no_bits - 1);
				
				size_t zero_run_len = zero_run_code + (1ull << zero_run_code_no_bits) - 1;

				if (cur_column_decoded_symbols + zero_run_len == n_sequences)
				{
					cur_column_decoded_symbols += zero_run_len;
					zero_run_code = 0;
					zero_run_code_no_bits = 0;
				}
			}
		}
		else
		{
			int selector = rc_selector[ctx_sel]->Decode() + 2;
			ctx_sel = ctx_update_selector(ctx_sel, selector - 2);

			int suffix = rc_suffix[ctx_sel % no_suffix_ctx]->Decode();

			x = suffix + (1 << (selector - 1));
			
			++cur_column_decoded_symbols;
		}

		dest.push_back(x);
		++decoded_symbols;
	}

	in_out->Push(priority, dest);

	rcd->End();

	delete_rc();

	in_out->MarkCompleted();
}

// *******************************************************************************************
// Do processing
void CEntropy::operator()()
{
	if (forward_mode)
		forward();
	else
		reverse();
}

// *******************************************************************************************
// Initialize range coder classes
void CEntropy::init_rc()
{
	CBasicRangeCoder<CVectorIOStream> *rcb;

	if (forward_mode)
	{
		rce = new CRangeEncoder<CVectorIOStream>(*vios);
		rcd = nullptr;		
		rcb = (CBasicRangeCoder<CVectorIOStream> *) rce;
	}
	else
	{
		rce = nullptr;		
		rcd = new CRangeDecoder<CVectorIOStream>(*vios);
		rcb = (CBasicRangeCoder<CVectorIOStream> *) rcd;
	}
	
	// Initialize models
	for (int i = 0; i < no_prefix_ctx; ++i)
		rc_prefix[i] = new CRangeCoderModel<CVectorIOStream>(rcb, 4, 7, 1 << 8, nullptr, forward_mode);

	for (int i = 0; i < no_selector_ctx; ++i)
		rc_selector[i] = new CRangeCoderModel<CVectorIOStream>(rcb, 5, 7, 1 << 8, nullptr, forward_mode);

	for (int i = 0; i < no_suffix_ctx; ++i)
		rc_suffix[i] = new CRangeCoderModel<CVectorIOStream>(rcb, 1 << (i % 8 + 1), 10, 1 << 10, nullptr, forward_mode);
}

// *******************************************************************************************
// Delete range coder classes and models
void CEntropy::delete_rc()
{
	if (rce)
		delete rce;
	rce = nullptr;

	if (rcd)
		delete rcd;
	rcd = nullptr;
	
	for (int i = 0; i < no_prefix_ctx; ++i)
	{
		if (rc_prefix[i])
			delete rc_prefix[i];
		rc_prefix[i] = nullptr;
	}
	
	for (int i = 0; i < no_selector_ctx; ++i)
	{
		if (rc_selector[i])
			delete rc_selector[i];
		rc_selector[i] = nullptr;
	}
	
	for (int i = 0; i < no_suffix_ctx; ++i)
	{
		if(rc_suffix[i])
			delete rc_suffix[i];
		rc_suffix[i] = nullptr;
	}
}

// *******************************************************************************************
// Int log2
int CEntropy::ilog2(int x)
{
	int r;

	for (r = 0; x; ++r)
		x >>= 1;

	return r;
}

// *******************************************************************************************
// Calculate entropy of a string - currently unused
double CEntropy::calc_avg_entropy(string &s)
{
	double ent = 0.0;

	unordered_map<int, int> m_hist;

	for (auto c : s)
		m_hist[c] += 1;

	double tot = (double) s.size();

	for (auto x : m_hist)
		ent += -x.second * log2(x.second / tot);

	ent /= tot;

	return ent;
}

// *******************************************************************************************
// Update selector context
uint32_t CEntropy::ctx_update_selector(uint32_t old, uint32_t selector)
{
	return ((old << 3) + selector) % no_selector_ctx;
}

// *******************************************************************************************
// Update prefix context
uint32_t CEntropy::ctx_update_prefix(uint32_t old, uint32_t prefix)
{
	return (old * 5 + prefix) % no_prefix_ctx;
}

// EOF
