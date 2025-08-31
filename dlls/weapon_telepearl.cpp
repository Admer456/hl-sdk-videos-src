#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL

class CTelePearlEntity : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	void Touch( CBaseEntity* pOther ) override;

	static CTelePearlEntity* Create( CBaseEntity* owner, Vector origin, Vector direction, bool itemHunt = false );

	void SetItemHunt( bool itemHuntMode )
	{
		itemHunt = itemHuntMode;
	}

	void SetOwningWeapon( CTelePearl* telePearl )
	{
		owningWeapon = telePearl;
	}

private:
	EHANDLE itemToHunt = {};
	CTelePearl* owningWeapon = nullptr;
	bool itemHunt = false;
};

LINK_ENTITY_TO_CLASS( tpearl, CTelePearlEntity );

void CTelePearlEntity::Spawn()
{
	Precache();

	SET_MODEL( edict(), "models/w_squeak.mdl" );
	UTIL_SetSize( pev, Vector( -4, -4, 0 ), Vector( 4, 4, 8 ) );
	UTIL_SetOrigin( pev, pev->origin );

	pev->gravity = 0.5;
	pev->friction = 0.5;

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	EMIT_SOUND( edict(), CHAN_STATIC, "squeek/sqk_deploy1.wav", 1.0f, ATTN_NORM );

	pev->nextthink = gpGlobals->time + 0.5f;

	itemToHunt = nullptr;

	if ( itemHunt )
	{
		pev->dmgtime = 4.0f;

		CBaseEntity* entity = nullptr;
		while ( entity = UTIL_FindEntityInSphere( entity, pev->origin, 512.0f ) )
		{
			// Skip the player
			if ( entity->edict() == pev->owner )
			{
				continue;
			}

			// Let's filter out only ammo_*, weapon_*, item_* and monster_scientist
			bool isAmmo = dynamic_cast<CBasePlayerAmmo*>(entity) != nullptr;
			bool isScientist = FStrEq( STRING( entity->pev->classname ), "monster_scientist" );
			if ( isAmmo || isScientist )
			{
				// Just select the first one that qualifies
				itemToHunt = entity;
				return;
			}
		}
	}
}

void CTelePearlEntity::Precache()
{
	PRECACHE_MODEL( "models/w_squeak.mdl" );
	PRECACHE_SOUND( "squeek/sqk_deploy1.wav" );
}

float UTIL_Clamp( float x, float minValue, float maxValue )
{
	if ( x < minValue )
	{
		return minValue;
	}

	if ( x > maxValue )
	{
		return maxValue;
	}

	return x;
}

void CTelePearlEntity::Think()
{
	if ( itemHunt )
	{
		// Traceline to the ground and maintain roughly 80 units away from it
		TraceResult tr;
		Vector traceEnd = pev->origin - Vector( 0.0f, 0.0f, 128.0f );
		UTIL_TraceLine( pev->origin, traceEnd, ignore_monsters, edict(), &tr );

		// Gently descend if too high in the air
		if ( tr.flFraction == 1.0f )
		{
			pev->velocity.z -= 10.0f;
		}
		else
		{
			pev->velocity.z = (tr.vecEndPos.z + 80.0f) - pev->origin.z;
		}

		pev->dmgtime -= 0.1f;

		if ( pev->dmgtime < 0.0f )
		{
			ExplosionCreate( pev->origin, g_vecZero, edict(), 32.0f, false );
			UTIL_Remove( this );
		}

		// Often times, there may be no objects nearby, in which case just stand still
		if ( itemToHunt != nullptr )
		{
			pev->velocity = pev->velocity + (itemToHunt->pev->origin - pev->origin) * 0.25f;
		}

		pev->velocity.x = UTIL_Clamp( pev->velocity.x, -50.0f, 50.0f );
		pev->velocity.y = UTIL_Clamp( pev->velocity.y, -50.0f, 50.0f );
	}

	pev->nextthink = gpGlobals->time + 0.1f;
}

void CTelePearlEntity::Touch( CBaseEntity* pOther )
{
	if ( itemHunt )
	{
		UTIL_Remove( this );
		return;
	}

	// Apply a rough offset above the floor. It's not 36 units exactly
	// because sometimes there may be a ramp there, which would get
	// the player stuck. This could be improved with some good raycasting.
	Vector destination = pev->origin + Vector( 0.0f, 0.0f, 50.0f );

	// Instagibbing monsters, breakables etc.
	bool gib = pOther->MyMonsterPointer() != nullptr || FClassnameIs( pOther->pev, "func_breakable" );
	int savedSolid = pOther->pev->solid;
	if ( gib )
	{
		// If an entity has a tiny bbox, e.g. a headcrab may be 32 units tall (16 in the centre),
		// we will apply just about enough offset to get the player's centre above that so the player is
		// on the floor, not IN the floor.
		// This formula seems to work: 1 + min(0, 36 - 16) = 1 + 20 = 21 (in the headcrab's case)
		// That would then put the player 16+21=37 units above the floor, or rather, their feet would be
		// 1 unit above. Revise Chapter 3 for some basic vector maths and tracelines!
		destination = pOther->Center();
		destination.z += 1.0f + V_min( 0.0f, (72.0f / 2.0f) - (pOther->pev->size.z / 2.0f) );

		// This is a little hack, we'll restore the solidity type later.
		// When tracing, we don't want to trace against the entity we're teleporting to, basically.
		pOther->pev->solid = SOLID_NOT;
	}

	// TODO: "wiggle out" logic may be preferable before the teleport is cancelled.

	// We're gonna trace a box roughly around the player, 1 unit upward,
	// just to see if the player's gonna get stuck inside. We'll check if
	// the trace starts in a solid volume, that's really the only thing we're looking for.
	const Vector traceStart = destination;
	const Vector traceEnd = destination + Vector( 0.0f, 0.0f, 1.0f );

	TraceResult tr;
	CBaseEntity* owner = Instance( pev->owner );

	// A tracehull is like a raycast but for boxes, specifically AABBs. A shapecast if you will.
	// Hull 0 is the point hull, 1 is player standing/humanoid, 2 is big monsters
	// and 3 is player crouching/small monsters.
	UTIL_TraceHull( traceStart, traceEnd, ignore_monsters, 1, owner->edict(), &tr );
	if ( !tr.fStartSolid )
	{
		// TODO: Check if the player is crouching.
		// TODO: Play some kind of cancel sound
		UTIL_SetOrigin( owner->pev, destination );

		// We can only instagib now that we've made sure
		// the player can actually teleport here.
		if ( gib )
		{
			// Restore the solidity type here.
			pOther->pev->solid = savedSolid;
			pOther->pev->health = 0;
			pOther->Killed( owner->pev, GIB_ALWAYS );
			// Sometimes there's a bug where there won't be gibs, so work around that
			pOther->TakeDamage( pev, owner->pev, 9999.0f, DMG_CRUSH | DMG_ALWAYSGIB );
		}
	}

	// TODO: Half-Life teleport particle FX, beams and sprites..
	UTIL_Remove( this );
}

CTelePearlEntity* CTelePearlEntity::Create( CBaseEntity* owner, Vector origin, Vector direction, bool itemHunt )
{
	// NOTE: This is a modified GetClassPtr with the parametre a = nullptr by default
	CTelePearlEntity* pPearl = GetClassPtr<CTelePearlEntity>();
	pPearl->pev->classname = MAKE_STRING( "ft_fire" );
	pPearl->pev->origin = origin + direction * 60.0f;
	pPearl->pev->angles = g_vecZero;
	pPearl->pev->velocity = direction.Normalize() * 200.0f;
	pPearl->pev->owner = owner->edict();
	pPearl->SetItemHunt( itemHunt );
	pPearl->Spawn();
	return pPearl;
}

#endif

LINK_WEAPON_TO_CLASS( weapon_pearl, CTelePearl );

constexpr const char* WorldModel = "models/w_squeak.mdl";
constexpr const char* ViewModel = "models/v_squeak.mdl";
constexpr const char* PlayerModel = "models/p_squeak.mdl";

void CTelePearl::Spawn()
{
	Precache();
	m_iId = WEAPON_TELEPEARL;
	SET_MODEL( ENT( pev ), WorldModel );

	m_iDefaultAmmo = WEAPON_NOCLIP;
	m_iClip = WEAPON_NOCLIP;

	FallInit();
}

void CTelePearl::Precache()
{
	UTIL_PrecacheOther( "tpearl" );
	PRECACHE_MODEL( WorldModel );
	PRECACHE_MODEL( ViewModel );
	PRECACHE_MODEL( PlayerModel );
}

bool CTelePearl::GetItemInfo( ItemInfo* p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iId = m_iId = WEAPON_TELEPEARL;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

void CTelePearl::PrimaryAttack()
{
	ThrowPearl( false );
}

void CTelePearl::SecondaryAttack()
{
	ThrowPearl( true );
}

bool CTelePearl::Deploy()
{
	m_fireState = FIRE_OFF;
	return DefaultDeploy( ViewModel, PlayerModel, PEARL_DRAW, "grenade" );
}

void CTelePearl::WeaponIdle()
{
	if ( (m_pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 && (m_pPlayer->pev->button & IN_ATTACK) != 0 )
	{
		return;
	}

	ResetEmptySound();

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;
}

void CTelePearl::ThrowPearl( bool itemHunt )
{
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

	SendWeaponAnim( PEARL_THROW1 );

#ifndef CLIENT_DLL
	CTelePearlEntity::Create( m_pPlayer, vecSrc, vecAiming, itemHunt );
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.5f );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}
