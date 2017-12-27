#pragma once
// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
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
