#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

// *******************************************************************************************
//
// *******************************************************************************************
class CSecondStage
{
public: 
	CSecondStage()
	{};

	virtual ~CSecondStage()
	{};

	virtual void operator()() = 0;
};

// EOF
