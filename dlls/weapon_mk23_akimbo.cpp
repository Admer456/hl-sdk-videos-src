
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"

LINK_WEAPON_TO_CLASS(weapon_mk23_akimbo, CMK23Akimbo);

void CMK23Akimbo::Spawn()
{
	Precache();
	m_iId = WEAPON_MK23_AKIMBO;

	// There is no w_mk23_akimbo.mdl, so we
	// gotta make do with what we got
	SET_MODEL(ENT(pev), "models/w_mk23.mdl");

	m_iDefaultAmmo = MK23_AKIMBO_DEFAULT_GIVE;

	FallInit();
}

void CMK23Akimbo::Precache()
{
	PRECACHE_MODEL("models/v_mk23_akimbo.mdl");
	PRECACHE_MODEL("models/p_mk23_akimbo.mdl");

	// This uses the same firing event as the MK23,
	// so no need to precache it ourselves.
	// 
	// Still, call the base class' Precache so that
	// the instance of this weapon has a valid event ID
	CMK23::Precache();
}

bool CMK23Akimbo::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MK23_AKIMBO_MAX_CLIP;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MK23_AKIMBO;
	p->iWeight = MK23_AKIMBO_WEIGHT;

	return true;
}

void CMK23Akimbo::PrimaryAttack()
{
	// This pattern of doing stuff is occasionally called "tables"
	// We have a table of animations which represents an animation cycle,
	// so it shoots left, right, left, right etc. with their variants
	static const int animationCycle[] =
	{
		MK23_AKIMBO_SHOOT_LEFT_1,
		MK23_AKIMBO_SHOOT_RIGHT_1,
		MK23_AKIMBO_SHOOT_LEFT_2,
		MK23_AKIMBO_SHOOT_RIGHT_2
	};
	// Similarly we have a little table of recoil values
	// You may use this pattern to implement Counter-Strike-style
	// predictable recoil patterns!
	static const float yawPunches[] =
	{
		0.7f,
		-0.4f,
		0.65f,
		-0.5f
	};
	const int currentAnimationId = m_iClip % ARRAYSIZE(animationCycle);

	ShootSingle(
		animationCycle[currentAnimationId], // choose an animation from the table
		yawPunches[currentAnimationId], // choose a yaw punch from the table
		currentAnimationId % 2 == 0 ); // every other shot is basically on the left side
}

void CMK23Akimbo::SecondaryAttack()
{
	// Homework: left hand vs. right hand firing
	// You can look at MK23::PrimaryAttack for this one,
	// and basically halve the tables in CMK23Akimbo::PrimaryAttack
}

bool CMK23Akimbo::Deploy()
{
	return DefaultDeploy("models/v_mk23_akimbo.mdl", "models/p_mk23_akimbo.mdl", MK23_AKIMBO_DRAW, "onehanded");
}

void CMK23Akimbo::Reload()
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	DefaultReload(MK23_AKIMBO_MAX_CLIP, MK23_AKIMBO_RELOAD, 2.0f);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15f;
}

void CMK23Akimbo::WeaponIdle()
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim(MK23_AKIMBO_IDLE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 31.0f / 30.0f;
	}
}
