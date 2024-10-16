/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

#ifndef CLIENT_DLL
#define SAWBLADE_AIR_VELOCITY 1500
#define SAWBLADE_WATER_VELOCITY 800
#define SAWBLADE_NUM_BOUNCES 25

class CSawblade : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	void EXPORT BubbleThink();
	void EXPORT DiscTouch(CBaseEntity* pOther);

	void OnHitCharacter(CBaseEntity* pCharacter);
	void OnHitWorld(CBaseEntity* pOther, Vector surfaceNormal);
	void Stop();

	// TODO: this variable isn't save-restored, meaning
	// if you reload a save with it non-stop, you could
	// theoretically make it bounce forever, cuz' this will be 0
	int m_NumBounces = 0;
	int m_Trail = 0;

public:
	static CSawblade* DiscCreate(Vector start, Vector angles, CBaseEntity* owner);
};

LINK_ENTITY_TO_CLASS(sawblader_disc, CSawblade);

CSawblade* CSawblade::DiscCreate(Vector start, Vector angles, CBaseEntity* owner)
{
	CSawblade* pSawblade = GetClassPtr((CSawblade*)NULL);
	pSawblade->pev->classname = MAKE_STRING("sawblader_disc");
	pSawblade->pev->origin = start;
	pSawblade->pev->angles = angles;
	pSawblade->pev->owner = owner->edict();
	pSawblade->Spawn();

	return pSawblade;
}

void CSawblade::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_SLIDEBOX;

	pev->gravity = 0.001f;
	pev->friction = 0.0f;

	SET_MODEL(ENT(pev), "models/sawblade_disc.mdl");

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f));

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex()); // entity
	WRITE_SHORT(m_Trail);	 // model
	WRITE_BYTE(10);			 // life
	WRITE_BYTE(3);			 // width
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(255);		 // r, g, b
	WRITE_BYTE(255);		 // brightness

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	SetTouch(&CSawblade::DiscTouch);
	SetThink(&CSawblade::BubbleThink);
	pev->nextthink = gpGlobals->time + 0.2;
}

void CSawblade::Precache()
{
	PRECACHE_MODEL("models/sawblade_disc.mdl");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	PRECACHE_SOUND("weapons/xbow_fly1.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("fvox/beep.wav");
	m_Trail = PRECACHE_MODEL("sprites/smoke.spr");
}

int CSawblade::Classify()
{
	return CLASS_NONE;
}

void CSawblade::DiscTouch(CBaseEntity* pOther)
{
	// Entities that take damage are typically NPCs
	// and players, so we'll treat them as characters.
	// This will also be called for func_breakable, which
	// is no character, but oh well!
	if (pOther->pev->takedamage != DAMAGE_NO)
	{
		OnHitCharacter(pOther);
	}
	else // func_wall, worldspawn and others will just make it bounce
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		OnHitWorld(pOther, tr.vecPlaneNormal);
	}
}

void CSawblade::BubbleThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->waterlevel == 0)
		return;

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CSawblade::OnHitCharacter(CBaseEntity* pCharacter)
{
	TraceResult tr = UTIL_GetGlobalTrace();
	entvars_t* pevOwner = VARS(pev->owner);

	// UNDONE: this needs to call TraceAttack instead
	ClearMultiDamage();

	if (pCharacter->IsPlayer())
	{
		pCharacter->TraceAttack(pevOwner, gSkillData.plrDmgSawbladeClient, pev->velocity.Normalize(), &tr, DMG_NEVERGIB);
	}
	else
	{
		pCharacter->TraceAttack(pevOwner, gSkillData.plrDmgSawbladeMonster, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
	}

	ApplyMultiDamage(pev, pevOwner);

	// play body "thwack" sound
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM);
		break;
	}

	// The disc magically disappears!
	pev->velocity = Vector(0, 0, 0);
	pev->nextthink = gpGlobals->time + 0.01f;
	SetThink(&CSawblade::SUB_Remove);
	SetTouch(nullptr);
}

void CSawblade::OnHitWorld(CBaseEntity* pOther, Vector surfaceNormal)
{
	// Vector reflection formula. TODO: modify the
	// Vector class itself and add this as a method!
	const Vector direction = pev->velocity.Normalize();
	const Vector newDirection = direction - 2.0f * surfaceNormal * DotProduct(surfaceNormal, direction);

	// Calculate new angles. MOVETYPE_BOUNCE will automatically take care of bouncing
	//pev->velocity = newDirection * pev->speed;
	// TODO: play some spinning animation or rotate the entity
	pev->angles = UTIL_VecToAngles(newDirection);

	m_NumBounces++;

	if (m_NumBounces >= SAWBLADE_NUM_BOUNCES)
	{
		// This will get changed below if the disc is allowed to stick in what it hit.
		pev->nextthink = gpGlobals->time;

		SetThink(&CSawblade::SUB_Remove);
		SetTouch(nullptr);

		// If the disc has lost its energy and is hitting a worldspawn,
		// it will slash into the wall. You may also check for func_wall and others.
		if (FClassnameIs(pOther->pev, "worldspawn"))
		{
			Stop();
		}
	}

	// IDEA: sample the material which we've hit, play different sounds depending on impact angle etc.
	const int bouncePitch = RANDOM_LONG(98, 105);
	EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", 1.0f, ATTN_NORM, 0, bouncePitch);

	// Emit sparks each time we hit a surface
	if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
	{
		UTIL_Sparks(pev->origin);
	}
}

void CSawblade::Stop()
{
	Vector vecDir = pev->velocity.Normalize();
	UTIL_SetOrigin(pev, pev->origin - vecDir * 12);
	pev->angles = UTIL_VecToAngles(vecDir);
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_FLY;
	pev->velocity = Vector(0, 0, 0);

	// Disappear in about 10 seconds.
	pev->nextthink = gpGlobals->time + 10.0;
}
#endif

LINK_WEAPON_TO_CLASS(weapon_sawblader, CSawblader);

void CSawblader::Spawn()
{
	Precache();
	m_iId = WEAPON_SAWBLADER;
	SET_MODEL(ENT(pev), "models/w_crossbow.mdl");

	m_iDefaultAmmo = SAWBLADER_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}

void CSawblader::Precache()
{
	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	UTIL_PrecacheOther("sawblader_disc");

	m_usSawblader = PRECACHE_EVENT(1, "events/sawblader.sc");
}

bool CSawblader::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "bolts";
	p->iMaxAmmo1 = BOLT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SAWBLADER_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iId = WEAPON_SAWBLADER;
	p->iFlags = 0;
	p->iWeight = SAWBLADER_WEIGHT;
	return true;
}

bool CSawblader::Deploy()
{
	return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", m_iClip == 0 ? SAWBLADER_DRAW2 : SAWBLADER_DRAW1, "bow");
}

void CSawblader::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(m_iClip == 0 ? SAWBLADER_HOLSTER2 : SAWBLADER_HOLSTER1);
}

void CSawblader::PrimaryAttack()
{
	FireSawblade();
}

void CSawblader::FireSawblade()
{
	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_iClip--;

	int flags = 0;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSawblader, 0.0, g_vecZero, g_vecZero, 0, 0, m_iClip, 0, 0, 0);

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim);

	anglesAim.x = -anglesAim.x;
#ifndef CLIENT_DLL
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	CSawblade* pSawblade = CSawblade::DiscCreate(vecSrc, anglesAim, m_pPlayer);

	if (m_pPlayer->pev->waterlevel == 3)
	{
		pSawblade->pev->velocity = vecDir * SAWBLADE_WATER_VELOCITY;
		pSawblade->pev->speed = SAWBLADE_WATER_VELOCITY;
	}
	else
	{
		pSawblade->pev->velocity = vecDir * SAWBLADE_AIR_VELOCITY;
		pSawblade->pev->speed = SAWBLADE_AIR_VELOCITY;
	}
	pSawblade->pev->avelocity.z = 10;
#endif

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
}

void CSawblader::Reload()
{
	if (m_pPlayer->ammo_bolts <= 0)
		return;

	if (m_pPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	if (DefaultReload(5, SAWBLADER_RELOAD, 4.5))
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
	}
}

void CSawblader::WeaponIdle()
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES); // get the autoaim vector but ignore it;  used for autoaim crosshair in DM

	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			if (0 != m_iClip)
			{
				SendWeaponAnim(SAWBLADER_IDLE1);
			}
			else
			{
				SendWeaponAnim(SAWBLADER_IDLE2);
			}
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		else
		{
			if (0 != m_iClip)
			{
				SendWeaponAnim(SAWBLADER_FIDGET1);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0;
			}
			else
			{
				SendWeaponAnim(SAWBLADER_FIDGET2);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 30.0;
			}
		}
	}
}

class CSawbladerAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_sawblader_clip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/w_sawblader_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_SAWBLADERCLIP_GIVE, "bolts", BOLT_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_sawblader, CSawbladerAmmo);
