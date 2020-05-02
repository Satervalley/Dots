#pragma once

#include <set>
#include "CEffect.h"


typedef std::set<CEffect*> CEffect_Set;


class CEffectManager
{
public:

	~CEffectManager()
	{
		Release();
	}

	void Add(CEffect* pe)
	{
		if (pe)
			esEffects.insert(pe);
	}

	void Remove(CEffect* pe, bool bDelete = true)
	{
		if (pe)
		{
			esEffects.erase(pe);
			if (bDelete)
				delete pe;
		}
	}

	void Release(void)
	{
		for (auto it : esEffects)
		{
			delete it;
		}
		esEffects.clear();
	}

	size_t Size(void) const
	{
		return esEffects.size();
	}
protected:
	CEffect_Set esEffects;
};

