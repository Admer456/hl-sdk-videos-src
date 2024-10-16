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

LINK_WEAPON_TO_CLASS(weapon_gl, CGrenadeLauncher);

void CGrenadeLauncher::Reload()
{
	if (m_pPlayer->ammo_rockets <= 0)
		return;

	// Homework: You can implement shotgun-style reloading here
	DefaultReload(GRENADE_LAUNCHER_MAX_CLIP, RPG_RELOAD, 1.5f);

	// Needed so that the charged attack can work again
	// DefaultReload sets it to +3
	m_flTimeWeaponIdle = GetNextAttackDelay(2.0f);
}

void CGrenadeLauncher::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_rpg.mdl");
	m_iId = WEAPON_GRENADE_LAUNCHER;

	m_iDefaultAmmo = GRENADE_LAUNCHER_MAX_CLIP;
	FallInit(); // get ready to fall down.
}

void CGrenadeLauncher::Precache()
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usGrenLauncher = PRECACHE_EVENT(1, "events/grenlauncher.sc");
}

bool CGrenadeLauncher::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = M203_GRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GRENADE_LAUNCHER_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_GRENADE_LAUNCHER;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return true;
}

bool CGrenadeLauncher::Deploy()
{
	return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", m_iClip != 0 ? RPG_DRAW1 : RPG_DRAW_UL, "rpg");
}

void CGrenadeLauncher::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(RPG_HOLSTER1);
}

void CGrenadeLauncher::PrimaryAttack()
{
	LaunchGrenade(1000.0f);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
	m_ChargeTimer = 0.0f;
}

void CGrenadeLauncher::SecondaryAttack()
{
	if ( m_ChargeTimer >= 3.0f )
	{
		m_ChargeTimer = 3.0f;
		return;
	}

	// Important to set m_flTimeWeaponIdle here
	m_ChargeTimer += 0.1f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(0.1f);
}

void CGrenadeLauncher::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0)
	{
		if (m_ChargeTimer > 0.0f)
		{
			// Cancel the shot if it's not held for long enough
			if (m_ChargeTimer < 0.3f)
			{
				m_ChargeTimer = 0.0f;
				PlayEmptySound();
				return;
			}

			LaunchGrenade(m_ChargeTimer * 500.0f);
			return;
		}

		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			if (m_iClip == 0)
				iAnim = RPG_IDLE_UL;
			else
				iAnim = RPG_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if (m_iClip == 0)
				iAnim = RPG_FIDGET_UL;
			else
				iAnim = RPG_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.1;
		}

		ResetEmptySound();
		SendWeaponAnim(iAnim);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}

void CGrenadeLauncher::LaunchGrenade( float intensity )
{
	// Very important: reset the timer here!
	m_ChargeTimer = 0.0f;

	if (0 != m_iClip)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		// Make the grenade come out of a bit more physically accurate spot
		Vector vecSrc = m_pPlayer->GetGunPosition()
			+ gpGlobals->v_forward * 16
			+ gpGlobals->v_right * 8
			+ gpGlobals->v_up * -8;
		// Launch forward!
		Vector vecVelocity = gpGlobals->v_forward * intensity;
		CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecVelocity, 5.0f);
#endif

		int flags;
#if defined(CLIENT_WEAPONS)
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usGrenLauncher);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		PlayEmptySound();
	}
}
