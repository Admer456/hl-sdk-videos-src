
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"

LINK_WEAPON_TO_CLASS(weapon_mk23, CMK23);

void CMK23::Spawn()
{
	Precache();
	m_iId = WEAPON_MK23;
	SET_MODEL(ENT(pev), "models/w_mk23.mdl");

	m_iDefaultAmmo = MK23_DEFAULT_GIVE;

	FallInit();
}

void CMK23::Precache()
{
	PRECACHE_MODEL("models/v_mk23.mdl");
	PRECACHE_MODEL("models/w_mk23.mdl");
	PRECACHE_MODEL("models/p_mk23.mdl");

	m_usFireMk23 = PRECACHE_EVENT(1, "events/mk23.sc");
}

bool CMK23::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MK23_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MK23;
	p->iWeight = MK23_WEIGHT;

	return true;
}

bool CMK23::AddDuplicate(CBasePlayerItem* pItem)
{
	// Remember: this is done exclusively on the serverside
	#ifndef CLIENT_DLL
	// The player doesn't have the akimbo variant but they are picking
	// up another pistol. So get the akimbo variant
	if (!pItem->m_pPlayer->HasNamedPlayerItem("weapon_mk23_akimbo"))
	{
		pItem->m_pPlayer->GiveNamedItem("weapon_mk23_akimbo");
		return true;
	}
	#endif

	return CBasePlayerWeapon::AddDuplicate(pItem);
}

void CMK23::PrimaryAttack()
{
	// This setup allows you to easily cycle between multiple shot animations
	// first firing anim + m_iClip % (number of unique firing anims)
	// Alternatively, you can use RANDOM_LONG(MK23_SHOOT_1, MK23_SHOOT_XYZ)
	const int regularShootAnim = MK23_SHOOT_1 + m_iClip % 2;
	// Here we also apply a "last shot" anim
	const int shootAnim = m_iClip == 0 ? MK23_SHOOT_LAST : regularShootAnim;
	// For aesthetics, let's have slightly different recoil each time
	const float yawPunch = m_iClip % 2 ? -0.7f : -0.5f;

	ShootSingle(shootAnim, yawPunch);
}

void CMK23::SecondaryAttack()
{
	// Homework: gangsta style (use _TILTED animations)
}

// Observe: a relatively CLEAN shooting implementation
// Can get even cleaner, but this is good enough for now
void CMK23::ShootSingle(int shootAnim, float yawPunch, bool leftSide)
{
	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		return;
	}

	// Semi-auto firing
	if (m_pPlayer->m_afButtonLast & IN_ATTACK)
	{
		return;
	}

	m_iClip--;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// This is required for a correct gpGlobals->v_forward
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// The actual shooting
	const Vector vecSrc = m_pPlayer->GetGunPosition();
	const Vector vecAim = gpGlobals->v_forward;
	const Vector vecShotDir = m_pPlayer->FireBulletsPlayer(
		1,
		vecSrc, vecAim,
		VECTOR_CONE_1DEGREES,
		8192.0f, BULLET_PLAYER_9MM,
		0, 0,
		m_pPlayer->pev,
		m_pPlayer->random_seed);

	// Event playback
	PLAYBACK_EVENT_FULL(
	// This FEV_NOTHOST stuff is honestly ugly as hell,
	// but one way to make this prettier is to write a
	// UTIL_DefaultPlaybackFlags function that does this
#ifdef CLIENT_WEAPONS
		FEV_NOTHOST,
#else
		0,
#endif
		// Or, honestly, write an entire "PlaybackEvent" method in weapons,
		// so that it fills in all this boilerplate for us!
		m_pPlayer->edict(),
		m_usFireMk23,
		0.0f,
		g_vecZero,
		g_vecZero,
		vecShotDir.x, vecShotDir.y,
		// We're gonna use iparam1 for the animation!
		// This way we don't have to do repetitive work in the appropriate EV_ function
		// We will also encode a horizontal recoil here for good aesthetics
		shootAnim, yawPunch * 100.0f,
		// bparam1 will determine on what side the shell gets ejected
		leftSide ? 1 : 0, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.01f);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5f;
}

bool CMK23::Deploy()
{
	return DefaultDeploy("models/v_mk23.mdl", "models/p_mk23.mdl", MK23_DRAW, "onehanded");
}

void CMK23::Reload()
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	DefaultReload(MK23_MAX_CLIP, MK23_RELOAD, 1.4f);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4f;
}

void CMK23::WeaponIdle()
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim(MK23_IDLE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 31.0f / 30.0f;
	}
}
