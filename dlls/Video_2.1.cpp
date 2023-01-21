
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"

// ==============================================
// trigger_sethealth
// ==============================================
class CTriggerSetHealth : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(trigger_sethealth, CTriggerSetHealth);

void CTriggerSetHealth::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Is the activator a player?
	if (pActivator->IsPlayer())
	{ // If so, give em some health
		pActivator->pev->health = pev->health;
	}
}

// ==============================================
// env_spark_custom
// ==============================================
class CEnvSparkCustom : public CBaseEntity
{
public:
	void Think() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

private:
	bool canSpark = false;
};

LINK_ENTITY_TO_CLASS(env_spark_custom, CEnvSparkCustom);

void CEnvSparkCustom::Think()
{
	if (canSpark)
	{
		UTIL_Sparks(pev->origin); // Sparks will be emitted at the entity's position
	}

	// Wait 0.2 seconds before calling Think again
	// "speed" will be our delay keyvalue
	pev->nextthink = gpGlobals->time + pev->speed;
}

void CEnvSparkCustom::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	canSpark = !canSpark;
	// Think NOW when it's triggered
	pev->nextthink = gpGlobals->time;
}

// ==============================================
// func_trampoline
// ==============================================
class CFuncTrampoline : public CBaseEntity
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
};

LINK_ENTITY_TO_CLASS(func_trampoline, CFuncTrampoline);

void CFuncTrampoline::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER; // this entity won't be solid, but it'll be able to push nicely
	SET_MODEL(edict(), STRING(pev->model));
}

void CFuncTrampoline::Touch(CBaseEntity* pOther)
{
	// ANY entity that touches this will go up
	pOther->pev->velocity.z += 400.0f;

	// This calculates the entity's geometric centre
	// Alternative: pOther->Center()
	Vector position = pOther->pev->origin + (pOther->pev->maxs + pOther->pev->mins) * 0.5f;
	ExplosionCreate(position, pev->angles, edict(), 50, true);
}
