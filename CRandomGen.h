#pragma once

#include <random>


class CRandomGen
{
public:
	CRandomGen()
	{
		std::random_device rd;
		rGen.seed(rd());
		//rGen.seed(1u);
	}


	bool GenBool(void)
	{
		return rGen() % 2u;
	}


	int GenInt(int nt = 100, int nf = 0)
	{
		ASSERT(nt > nf);
		return nf + (rGen() % (nt - nf));
	}


	bool Probility(unsigned int nProb = 1u, unsigned int nBase = 1000u)
	{
		ASSERT(nProb > 0u && nProb < nBase);
		return (rGen() % nBase) < nProb;
	}

protected:
	std::mt19937 rGen;
};

