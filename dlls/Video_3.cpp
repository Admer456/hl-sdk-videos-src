
#include "extdll.h"
#include "util.h"
#include "cbase.h"

class FuncSecurityCamera : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;

private:
	CBaseEntity* FindIntruder();
	bool IsVisible(CBaseEntity* potentialIntruder);
	void Rotate();

	CBaseEntity* intruder = nullptr;
	bool detected = false;
};

LINK_ENTITY_TO_CLASS(func_security_cam, FuncSecurityCamera);

void FuncSecurityCamera::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	SET_MODEL(edict(), STRING(pev->model));
	UTIL_SetOrigin(pev, pev->origin);

	// Wait a couple of seconds for the client to join
	pev->nextthink = gpGlobals->time + 1.5f;
}

void FuncSecurityCamera::Think()
{
	// Locate an intruder, if any
	intruder = FindIntruder();

	// If we have an intruder, we trigger our targets once
	if (intruder && !detected)
	{
		detected = true;
		SUB_UseTargets(intruder, USE_SET, 1.0f);
	}

	// Rotate the camera
	Rotate();

	// Think pretty much every server tick
	pev->nextthink = gpGlobals->time + 0.016f;
}

CBaseEntity* FuncSecurityCamera::FindIntruder()
{
	// Step 1: search by radius
	CBaseEntity* potentialIntruder = nullptr;
	while (potentialIntruder = UTIL_FindEntityInSphere(potentialIntruder, pev->origin, 512.0f))
	{
		// Reject non-players
		if (!potentialIntruder->IsPlayer())
		{
			continue;
		}

		// Step 2: determine visibility by FOV and traceline
		if (IsVisible(potentialIntruder))
		{
			return potentialIntruder;
		}
	}

	// If we can't find any, return nullptr
	return nullptr;
}

bool FuncSecurityCamera::IsVisible(CBaseEntity* potentialIntruder)
{
	// Step 1: FOV check
	// Note: direction is normalised
	Vector direction = (potentialIntruder->pev->origin - pev->origin).Normalize();

	// UTIL_MakeVectors typically stores these in gpGlobals->v_xyz
	// UTIL_MakeVectorsPrivate will store them in vectors that we pass
	Vector forward;
	UTIL_MakeVectorsPrivate(pev->angles, forward, nullptr, nullptr);

	// If the angle between these vectors is greater than 60 degrees, bail out
	if (DotProduct(forward, direction) < 0.5f)
	{
		return false;
	}

	// Step 2: Traceline check
	TraceResult tr;
	UTIL_TraceLine(pev->origin, potentialIntruder->pev->origin, dont_ignore_monsters, edict(), &tr);

	return tr.pHit == potentialIntruder->edict();
}

void FuncSecurityCamera::Rotate()
{
	// In case there's an intruder, don't rotate
	if (intruder)
	{
		return;
	}

	// 15 degrees per second
	pev->angles.y += 15.0f * gpGlobals->frametime;
}
