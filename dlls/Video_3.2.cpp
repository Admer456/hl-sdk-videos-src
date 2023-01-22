
#include "extdll.h"
#include "util.h"
#include "cbase.h"

// ==============================================
// animation_test1
// Feel free to uncomment some of these and play around
// ==============================================
class CAnimationTest1 : public CBaseAnimating
{
public:
	void Spawn() override;
	void Precache() override;

	void Think() override;
};

LINK_ENTITY_TO_CLASS(animation_test1, CAnimationTest1);

void CAnimationTest1::Spawn()
{
	Precache();

	SET_MODEL(edict(), "models/scientist.mdl");
	UTIL_SetOrigin(pev, pev->origin);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	//InitBoneControllers();
	//ResetSequenceInfo();

	pev->sequence = LookupSequence("run");
	pev->nextthink = gpGlobals->time + 1.0f;
}

void CAnimationTest1::Precache()
{
	PRECACHE_MODEL("models/scientist.mdl");
}

void CAnimationTest1::Think()
{
	pev->frame += 1.0f;
	if (pev->frame >= 255.0f)
	{
		pev->frame = 0.0f;
	}

	SetBodygroup(1, 2);

	pev->nextthink = gpGlobals->time + 0.01f;
	//pev->framerate = 3.0f + std::sin( gpGlobals->time * 0.333f ) * 3.0f;

	// time & 3 is the same as time % 4
	// For values like 4, 5, 6 and 7, it'll just
	// roll back to 0, 1, 2 and 3
	//SetBodygroup( 1, int(gpGlobals->time) & 3 );

	//StudioFrameAdvance();
}

// ==============================================
// animation_test2 - a can that changes its skin
// ==============================================
class CAnimationTest2 : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void Think() override;
};

LINK_ENTITY_TO_CLASS(animation_test2, CAnimationTest2);

void CAnimationTest2::Spawn()
{
	Precache();

	SET_MODEL(edict(), "models/can.mdl");
	UTIL_SetOrigin(pev, pev->origin);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_PUSH;

	pev->nextthink = gpGlobals->time + 1.0f;
}

void CAnimationTest2::Precache()
{
	PRECACHE_MODEL("models/can.mdl");
}

void CAnimationTest2::Think()
{
	if (++pev->skin >= 6)
	{
		pev->skin = 0;
	}

	pev->avelocity.x = 15.0f;
	pev->avelocity.y = 20.0f;

	pev->nextthink = pev->ltime + 0.1f;
}

// ==============================================
// animation_test3 - bouncy* explosion sprite
// 
// * if you uncomment it
// ==============================================
class CAnimationTest3 : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void Touch(CBaseEntity* pOther) override;
	void Think() override;

private:
	int numFrames = 0;
	int iterator = 1;
};

LINK_ENTITY_TO_CLASS(animation_test3, CAnimationTest3);

void CAnimationTest3::Spawn()
{
	Precache();

	SET_MODEL(edict(), "sprites/fexplo.spr");

	//pev->movetype = MOVETYPE_BOUNCE;
	//pev->solid = SOLID_SLIDEBOX;

	pev->renderamt = 255;
	pev->rendermode = kRenderTransAdd;

	//UTIL_SetSize( pev, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );
	//pev->friction = 0.0f;
	//pev->gravity = 0.1f;

	numFrames = MODEL_FRAMES(pev->modelindex);

	pev->nextthink = gpGlobals->time + 1.0f;
}

void CAnimationTest3::Precache()
{
	PRECACHE_MODEL("sprites/fexplo.spr");
}

void CAnimationTest3::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	pev->velocity = pOther->pev->origin - pev->origin;
	pev->velocity = pev->velocity * 10.0f;
}

void CAnimationTest3::Think()
{
	//if ( pev->frame >= numFrames - 1 )
	//{
	//	iterator = -1;
	//}
	//if ( pev->frame < 1.0f )
	//{
	//	iterator = 1;
	//}

	pev->frame += iterator;
	if (pev->frame >= numFrames)
		pev->frame = 0.0f;

	pev->nextthink = gpGlobals->time + 0.1f;
}
