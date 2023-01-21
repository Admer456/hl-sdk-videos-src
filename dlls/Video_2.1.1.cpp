
#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CFuncRepairable : public CBaseEntity
{
public:
	void Spawn() override;

	int ObjectCaps() override
	{
		return FCAP_IMPULSE_USE;
	}

	void Think() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
};

LINK_ENTITY_TO_CLASS(func_repairable, CFuncRepairable);

void CFuncRepairable::Spawn()
{
	pev->takedamage = DAMAGE_YES;
	pev->health = 100;

	pev->solid = SOLID_BSP;

	// MOVETYPE_NONE doesn't work with SOLID_BSP // TODO: confirm???
	// MOVETYPE_PUSH calls Think every 0.1s
	// Thus, MOVETYPE_PUSHSTEP
	pev->movetype = MOVETYPE_PUSHSTEP;
	SET_MODEL(edict(), STRING(pev->model));

	// In order to call Think(), the entity needs pev->nextthink greater than 0
	pev->nextthink = gpGlobals->time + 1.5f;
}

void CFuncRepairable::Think()
{
	// Spark only if health is less than 90
	if (pev->health < 90)
	{
		UTIL_Sparks(pev->origin);
	}

	// Add 0.05 seconds to it, to avoid nextthink == gpGlobals->time
	pev->nextthink = gpGlobals->time + (pev->health / 100.0f) + 0.05f;
}

void CFuncRepairable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	pev->health += 5;

	if (pev->health > 100)
		pev->health = 100;

	ALERT(at_console, "health is now %i\n", (int)pev->health);
}

bool CFuncRepairable::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (bitsDamageType & DMG_CLUB)
	{
		pev->health += flDamage;
		if (pev->health > 100)
			pev->health = 100;

		return false;
	}

	pev->health -= flDamage;

	// Must prevent health from going less than 0, cuz'
	// it will affect our nextthink calculations
	if (pev->health < 0)
		pev->health = 0;

	return true;
}
