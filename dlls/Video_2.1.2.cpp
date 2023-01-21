#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CFuncFader : public CBaseEntity
{
public:
	void Spawn() override;

	void FadeOutThink();
	void FadeInThink();
};

LINK_ENTITY_TO_CLASS(func_fader, CFuncFader);

void CFuncFader::Spawn()
{
	SET_MODEL(edict(), STRING(pev->model));
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSHSTEP;

	// The mapper sets the render mode
	pev->renderamt = 128; // 50% transparency

	SetThink(&CFuncFader::FadeInThink);
	pev->nextthink = 1.5f;
}

void CFuncFader::FadeOutThink()
{
	pev->renderamt -= 1.0f;
	if (pev->renderamt <= 0)
	{
		SetThink(&CFuncFader::FadeInThink);
	}

	pev->nextthink = gpGlobals->time + 0.02f;
}

void CFuncFader::FadeInThink()
{
	pev->renderamt += 1.0f;
	if (pev->renderamt >= 255)
	{
		SetThink(&CFuncFader::FadeOutThink);
	}

	pev->nextthink = gpGlobals->time + 0.02f;
}
