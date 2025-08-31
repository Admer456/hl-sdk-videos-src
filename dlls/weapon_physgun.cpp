
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

LINK_WEAPON_TO_CLASS( weapon_physgun, CPhysgun );

constexpr const char* WorldModel = "models/w_crossbow.mdl";
constexpr const char* ViewModel = "models/v_crossbow.mdl";
constexpr const char* PlayerModel = "models/p_crossbow.mdl";

constexpr float AttractDistance = 400.0f;
constexpr float PickupDistance = 96.0f;

void CPhysgun::Spawn()
{
	Precache();
	m_iId = WEAPON_PHYSGUN;
	SET_MODEL( ENT( pev ), WorldModel );

	m_iDefaultAmmo = WEAPON_NOCLIP;
	m_iClip = WEAPON_NOCLIP;

	FallInit();
}

void CPhysgun::Precache()
{
	PRECACHE_MODEL( WorldModel );
	PRECACHE_MODEL( ViewModel );
	PRECACHE_MODEL( PlayerModel );
}

bool CPhysgun::GetItemInfo( ItemInfo* p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 0; // WEAPON_NOCLIP does not call Reload()
	p->iId = m_iId = WEAPON_PHYSGUN;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

static bool IsPhysicalEntity( edict_t* edict )
{
	if ( edict == nullptr )
	{
		return false;
	}

	CBaseEntity* entity = CBaseEntity::Instance( edict );

	// Monsters are a yes
	if ( dynamic_cast<CBaseMonster*>( entity ) )
	{
		return true;
	}

	// Pushables are a yes
	if ( FClassnameIs( edict, "func_pushable" ) )
	{
		return true;
	}

	// Our sawblades, turrets, rats, leech are all slidebox
	// Airtanks, gibs etc. are bbox, so catch them too
	if ( entity->pev->solid != SOLID_SLIDEBOX
		&& entity->pev->solid != SOLID_BBOX )
	{
		return false;
	}

	// Everything else may depend on a heuristic or so
	switch ( entity->pev->movetype )
	{
		case MOVETYPE_TOSS: return true;
		case MOVETYPE_FLY: return true;
		case MOVETYPE_PUSHSTEP: return true;
		case MOVETYPE_BOUNCE: return true;
		case MOVETYPE_WALK: return true;
		default: break;
	}
	return false;
}

void CPhysgun::PrimaryAttack()
{
	// There's a sneaky little issue with doing it in PrimaryAttack etc.
	// Half-Life has a client-server model and both of these parties see the "game's world"
	// a bit differently. The server "sees" a lot more entities than the client does, meaning
	// a traceline simply hits differently! So what will happen is, the gravity gun will have
	// one set of timer values on the client, and a whole other one on the server. Massive desync.
	// It won't affect us in this particular case, because there's no particular clientside logic
	// going on here like playing animations. But it's something to be aware of.
#ifndef CLIENT_DLL
	Vector vecSrc = GetGunPosition();

	if ( m_PhysMode )
	{
		// If we already have an entity, there's no need to re-trace
		// This avoids a new entity being picked up if it happens to be in between
		// the currently held entity and the player
		if ( m_Entity == nullptr )
		{
			TraceResult tr;
			UTIL_TraceLine( vecSrc, vecSrc + GetPlayerAim() * AttractDistance, dont_ignore_monsters, m_pPlayer->edict(), &tr );
			if ( IsPhysicalEntity( tr.pHit ) )
			{
				m_Entity = Instance( tr.pHit );
				m_Distance = (m_Entity->pev->origin - vecSrc).Length();
			}
		}

		if ( m_Entity != nullptr )
		{
			ForceMove( m_Entity );
		}

		m_flNextPrimaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay( 0.001f );
		return;
	}

	// Gravity gun mode, no entity currently held, so do a force push
	if ( m_Entity == nullptr )
	{
		TraceResult tr;
		UTIL_TraceLine( vecSrc, vecSrc + GetPlayerAim() * AttractDistance,
			dont_ignore_monsters, m_pPlayer->edict(), &tr );

		if ( IsPhysicalEntity( tr.pHit ) )
		{
			ForcePush( Instance( tr.pHit ) );
		}
	}
	else
	{
		// We're holding an entity, so go and push it
		ForcePush( m_Entity );
		m_Entity = nullptr;
	}
#endif

	m_flNextPrimaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay( 1.0f );
}

void CPhysgun::SecondaryAttack()
{
	// Read the top comment in PrimaryAttack for more context on this.
#ifdef CLIENT_DLL
	m_flNextSecondaryAttack = GetNextAttackDelay( 0.5f );
	return;
#else
	// Typically you'd expect SecondaryAttack to freeze the entity in the physgun mode,
	// BUT, I was too lazy. One way to do this would be to set the pev->movetype of an entity
	// somewhere, somehow in a way that is bookkept so you can unfreeze it later and return it
	// to its original movetype. You could store this whole thing in CBaseEntity but that'd be
	// a bit of a waste. An external system/manager of some sorts would do this much better
	if ( m_PhysMode )
	{
		return;
	}
	
	// Drop the entity
	if ( m_Entity.Get() )
	{
		m_Entity = nullptr;
		m_flNextSecondaryAttack = GetNextAttackDelay( 1.0f );
		return;
	}
	
	// Try attracting an entity
	Vector vecAiming = GetPlayerAim();
	Vector vecSrc = GetGunPosition();

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * AttractDistance, dont_ignore_monsters, m_pPlayer->edict(), &tr );
	if ( !IsPhysicalEntity( tr.pHit ) )
	{
		return;
	}

	CBaseEntity* entity = Instance( tr.pHit );

	const Vector delta = entity->pev->origin - vecSrc;
	const float distance = delta.Length();

	if ( distance < PickupDistance )
	{
		// TODO: Play back an event that triggers a pickup animation on the client
		m_Entity = entity;
		m_Distance = PickupDistance;
		m_flNextSecondaryAttack = GetNextAttackDelay( 1.0f );
	}
	else
	{
		entity->pev->velocity = entity->pev->velocity - delta.Normalize() * 300.0f * gpGlobals->frametime;
		// Clear onground so the entity can take off
		entity->pev->flags &= ~FL_ONGROUND;

		// Run this every frame until the entity gets close enough
		m_flNextSecondaryAttack = GetNextAttackDelay( 0.001f );
	}
#endif
}

void CPhysgun::Reload()
{
#ifndef CLIENT_DLL
	// This avoids the player picking up an object then switching to another mode and
	// potential bugs that could come with such an ability
	if ( m_Entity != nullptr )
	{
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "CANNOT CHANGE MODES: DROP OBJECT FIRST" );
		return;
	}

	m_PhysMode = !m_PhysMode;

	ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, m_PhysMode
		? "MODE: PHYSGUN"
		: "MODE: GRAVITY GUN" );

	m_flNextPrimaryAttack = GetNextAttackDelay( 1.0f );
	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
	m_flTimeWeaponIdle = m_flNextPrimaryAttack;
	// This is what DefaultReload does basically
	m_pPlayer->m_flNextAttack = m_flTimeWeaponIdle;
#endif
}

bool CPhysgun::Deploy()
{
	m_fireState = FIRE_OFF;
	return DefaultDeploy( ViewModel, PlayerModel, PHYSGUN_DRAW1, "bow" );
}

void CPhysgun::WeaponIdle()
{
	ResetEmptySound();

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
	{
		return;
	}

#ifndef CLIENT_DLL
	if ( m_PhysMode )
	{
		// WeaponIdle is called when the player is pressing neither LMB nor RMB
		// So, it makes sense to release the entity here
		m_Entity = nullptr;
	}
	else
	{
		// This is basically a Think function at this point, so here we can update
		// the currently held entity and keep it in front of the player
		if ( m_Entity.Get() )
		{
			ForceMove( m_Entity );
		}
	}
#endif

	m_flTimeWeaponIdle = GetNextAttackDelay( 0.001f );
}

Vector CPhysgun::GetGunPosition() const
{
	if ( m_pPlayer == nullptr )
	{
		return pev->origin;
	}

	return m_pPlayer->GetGunPosition();
}

Vector CPhysgun::GetPlayerAim() const
{
	if ( m_pPlayer == nullptr )
	{
		UTIL_MakeVectors( pev->angles );
	}
	else
	{
		UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	}

	return gpGlobals->v_forward;
}

void CPhysgun::ForcePush( CBaseEntity* entity )
{
	const Vector vecAiming = GetPlayerAim();

	entity->pev->velocity = entity->pev->velocity + vecAiming * 650.0f;

	// If we're aiming at the ground, try bouncing it upwards
	if ( entity->pev->flags & FL_ONGROUND && vecAiming.z < -0.1f )
	{
		entity->pev->velocity.z *= -1.0f;
		entity->pev->flags &= ~FL_ONGROUND;
	}
}

void CPhysgun::ForceMove( CBaseEntity* entity )
{
	const Vector vecSrc = GetGunPosition();
	const Vector vecAiming = GetPlayerAim();
	const Vector delta = (vecSrc + vecAiming * m_Distance) - entity->pev->origin;

	// Do NOT multiply by frametime here, you are setting
	// a value that is *already* in the "per-second" domain
	entity->pev->velocity = delta * 16.0f;
}
