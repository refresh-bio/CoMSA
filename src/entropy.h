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
#include "defs.h"
#include "queue.h"
#include "rc.h"

// *******************************************************************************************
constexpr int c_pow(int base, int exponent)
{
	int r = 1;
	for (int i = 0; i < exponent; ++i)
		r *= base;

	return r;
}

const int MAX_NO_PREFIX_CTX = c_pow(5, 5);
const int MAX_NO_SELECTOR_CTX = c_pow(8, 3);
const int MAX_NO_SUFFIX_CTX = c_pow(8, 2);

enum class ctx_length_t {tiny, small, medium, large, huge};

const int CONTEXTS[5][3] = { 
	{c_pow(5, 2), c_pow(8, 1), c_pow(8, 1)},
	{c_pow(5, 3), c_pow(8, 2), c_pow(8, 1)},
	{c_pow(5, 4), c_pow(8, 2), c_pow(8, 2)},
	{c_pow(5, 5), c_pow(8, 2), c_pow(8, 2)},
	{c_pow(5, 5), c_pow(8, 3), c_pow(8, 2)}
};

// *******************************************************************************************
//
// *******************************************************************************************
class CEntropy
{
	CRegisteringPriorityQueue<string> *in_out;
	CVectorIOStream *vios;
	bool forward_mode;
	size_t *pre_entropy_sequences_size;
	size_t n_sequences;
	ctx_length_t ctx_length;

	CRangeEncoder<CVectorIOStream> *rce;
	CRangeDecoder<CVectorIOStream> *rcd;
	CRangeCoderModel<CVectorIOStream>
		*rc_prefix[MAX_NO_PREFIX_CTX],
		*rc_selector[MAX_NO_SELECTOR_CTX],
		*rc_suffix[MAX_NO_SUFFIX_CTX];

	int no_prefix_ctx;
	int no_selector_ctx;
	int no_suffix_ctx;

	void forward();
	void reverse();

	void init_rc();
	void delete_rc();

	int ilog2(int x);
	double calc_avg_entropy(string &s);

	uint32_t ctx_update_selector(uint32_t old, uint32_t selector);
	uint32_t ctx_update_prefix(uint32_t old, uint32_t prefix);

public:
	CEntropy(CRegisteringPriorityQueue<string> *_in_out, CVectorIOStream *_vios, size_t &pre_entropy_sequences_size, size_t _n_sequences, bool _forward_mode, ctx_length_t _ctx_length) :
		in_out(_in_out), vios(_vios), forward_mode(_forward_mode), pre_entropy_sequences_size(&pre_entropy_sequences_size), n_sequences(_n_sequences), ctx_length(_ctx_length), rce(nullptr), rcd(nullptr)
	{
		if (!in_out || !vios)
			throw "No I/O queues";

		no_prefix_ctx = CONTEXTS[(uint8_t) ctx_length][0];
		no_selector_ctx = CONTEXTS[(uint8_t)ctx_length][1];
		no_suffix_ctx = CONTEXTS[(uint8_t)ctx_length][2];
	};

	~CEntropy()
	{
		delete_rc();
	}

	void operator()();
};

// EOF
