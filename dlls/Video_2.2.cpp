
#include "extdll.h"
#include "util.h"
#include "cbase.h"

// ==============================================
// env_model
// ==============================================
class EnvModel : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_model, EnvModel);

void EnvModel::Spawn()
{
	if (!pev->model)
	{
		ALERT(at_console, "env_model doesn't have a model keyvalue\n");
		UTIL_Remove(this);
		return;
	}

	PRECACHE_MODEL(STRING(pev->model));
	SET_MODEL(edict(), STRING(pev->model));

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT; // or SOLID_BBOX if you'd like
}

void EnvModel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// ^ and = together -> toggle
	pev->effects ^= EF_NODRAW;
}

// ==============================================
// env_follower
// ==============================================
class EnvFollower : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;
};

LINK_ENTITY_TO_CLASS(env_follower, EnvFollower);

void EnvFollower::Spawn()
{
	PRECACHE_MODEL("sprites/glow01.spr");
	SET_MODEL(edict(), "sprites/glow01.spr");

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	pev->effects |= EF_DIMLIGHT;

	pev->nextthink = 1.5f;
}

void EnvFollower::Think()
{
	// Alternatively: UTIL_PlayerByIndex(1);
	CBaseEntity* player = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

	// You could add randomness here and update the entities less frequently
	// e.g. every 0.5 or 1 seconds
	pev->velocity = player->pev->origin - pev->origin;

	pev->nextthink = gpGlobals->time + 0.02f;
}
