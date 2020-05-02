#pragma once

#include "framework.h"
#include <memory>
#include <vector>

#include "SRawData.h"

enum class EDrawType
{
	dtNone,		// do not draw effect
	dtBefore,	// draw effect before body
	dtAfter,	// draw effect after body
	dtReplace	// draw effect, do not draw body
};


enum class EPassType
{
	ptNone,
	ptBeforeVelocity,	// process before delta velocity
	ptAfterVelocity,	// process after delta velocity
	ptPosition	// process after delta process
};


class CEffect
{
public:
	// return true: this effect is overtime, can be deleted
	virtual bool Pass(SRawData& rd, float fDeltaTime) = 0;
	virtual void Draw(CRenderTarget* prt)
	{

	}

	EDrawType DrawType(void) const
	{
		return dtDrawType;
	}

	EPassType PassType(void) const
	{
		return ptPassType;
	}
protected:
	EDrawType dtDrawType{ EDrawType::dtNone };
	EPassType ptPassType{ EPassType::ptNone };
};


typedef std::shared_ptr<CEffect> CEffect_Pointer;
typedef std::vector<CEffect_Pointer> CEffect_Pointer_Vector;


class CSingleEffect : public CEffect
{
protected:
	int nIndex;
//	int nSteps;
};


