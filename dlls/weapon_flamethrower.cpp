#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL

// Entity that "sticks" to an NPC, gib etc. and progressively burns them
class CFireSticker : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT FireStickerThink();

	void FinishCharring();

private:
	float m_flNextDamageTime = -1.0f;
	float m_flExpireTime = -1.0f;
	EHANDLE m_pVictim{};
	
public:
	static CFireSticker* CreateFireSticker( CBaseEntity* victim, CBaseEntity* owner );
};

LINK_ENTITY_TO_CLASS( ft_sticker, CFireSticker );

void CFireSticker::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	// Roughly scale with the width of the thing
	pev->scale = m_pVictim->pev->size.y / 10.0f;
	pev->renderamt = 255.0f;
	pev->rendermode = kRenderTransAdd;

	SET_MODEL( edict(), "sprites/xffloor.spr" );
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CFireSticker::FireStickerThink );

	pev->nextthink = gpGlobals->time + 0.01f;
	m_flExpireTime = gpGlobals->time + 10.0f;
	m_flNextDamageTime = gpGlobals->time + 0.5f;
}

void CFireSticker::Precache()
{
	PRECACHE_MODEL( "sprites/xffloor.spr" );
	PRECACHE_MODEL( "models/bonegibs.mdl" );
}

void CFireSticker::FireStickerThink()
{
	// Time to go? Bye bye then!
	if ( m_flExpireTime < gpGlobals->time || m_pVictim.Get() == nullptr )
	{
		UTIL_Remove( this );
		pev->nextthink = gpGlobals->time + 0.1f;
		return;
	}

	// What's a flame that doesn't burn?
	if ( m_flNextDamageTime < gpGlobals->time )
	{
		CBaseEntity* owner = Instance( pev->owner );
		// Take damage now, so we can check the
		// remaining health after the fact.
		m_pVictim->TakeDamage( pev, owner->pev, 10.0f, DMG_BURN );
		m_flNextDamageTime = gpGlobals->time + 0.5f;

		// Charred bones!! Yay!
		// "MyMonsterPointer" is a way to check if an entity is a CBaseMonster.
		// This way, if attached to brushes or gibs, it won't mess things up.
		if ( m_pVictim->MyMonsterPointer() != nullptr
			&& m_pVictim->pev->health <= 0.0f )
		{
			FinishCharring();
		}
	}

	// Make sure the flame stays somewhere at the centre of the victim.
	// It would be much better to have a few randomised offsets! E.g.
	// one flame on the arms, a few on the legs etc.
	pev->origin = pev->origin * 0.7f + m_pVictim->Center() * 0.3f;

	// The effect is animated at 20fps. One way of doing this, instead of
	// incrementing the frame counter, is to directly set it from a *quantised* time.
	const uint64_t milliseconds = gpGlobals->time * 1000.0f;
	// This is the absolute frame. It'll be modulo'd by the number of frames in the sprite.
	// You may see this pattern in some games! Explore codebases other than HL SDK sometimes.
	// This formula here basically divides 1000ms onto a grid of 20 frames.
	const uint64_t frameAbsolute = milliseconds / (1000 / 20);

	pev->frame = frameAbsolute % MODEL_FRAMES( pev->modelindex );
	pev->nextthink = gpGlobals->time + 0.001f;
}

void CFireSticker::FinishCharring()
{
	for ( int i = 0; i < 4; i++ )
	{
		// NOTE: This is a modified GetClassPtr with the parametre a = nullptr by default
		CGib* gib = GetClassPtr<CGib>();
		gib->Spawn( "models/bonegibs.mdl" );
		gib->pev->body = i;

		const Vector victimSize = m_pVictim->pev->size;
		const Vector randomPointInSize = Vector(
			RANDOM_FLOAT( -victimSize.x, victimSize.x ),
			RANDOM_FLOAT( -victimSize.y, victimSize.y ),
			RANDOM_FLOAT( -victimSize.z, victimSize.z )
		);

		// This is basically a much more elegant version of the logic in
		// CGib::SpawnRandomGibs. You're welcome, Valve :)
		gib->pev->origin = pev->origin + randomPointInSize;
		gib->pev->velocity = randomPointInSize * 5.0f;
		gib->pev->avelocity = randomPointInSize * 10.0f;
		// Oh yeah, charred dry bones shouldn't really bleed I think?
		gib->m_bloodColor = DONT_BLEED;

		// Attach another flame here, for extra effect!
		CreateFireSticker( gib, Instance( pev->owner ) );
	}

	// Make sure to make all parties involved non-solid, so
	// the gibs don't accidentally bump into them
	m_pVictim->pev->solid = SOLID_NOT;
	pev->solid = SOLID_NOT;
	
	UTIL_Remove( m_pVictim );
	UTIL_Remove( this );
}

CFireSticker* CFireSticker::CreateFireSticker( CBaseEntity* victim, CBaseEntity* owner )
{
	if ( victim == nullptr || owner == nullptr )
	{
		return nullptr;
	}
	
	// NOTE: This is a modified GetClassPtr with the parametre a = nullptr by default
	CFireSticker* pFire = GetClassPtr<CFireSticker>();
	pFire->pev->classname = MAKE_STRING( "ft_sticker" );
	pFire->pev->origin = victim->pev->origin;
	pFire->pev->angles = g_vecZero;
	pFire->pev->owner = owner->edict();
	pFire->m_pVictim = victim;
	pFire->Spawn();

	return pFire;
}

// Fire projectile. Flies and burns anything it touches. Spawns fire stickers upon contact.
class CFireProjectile : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;
	int Classify() override;
	void EXPORT FireThink();
	void EXPORT FireTouch( CBaseEntity* pOther );

	void OnHitCharacter( CBaseEntity* pCharacter );
	void OnHitWorld( CBaseEntity* pOther, Vector origin, Vector surfaceNormal );

public:
	static inline int FireSprite = 0;
	static CFireProjectile* CreateFire( Vector start, Vector aim, CBaseEntity* owner );
};

LINK_ENTITY_TO_CLASS( ft_fire, CFireProjectile );

void CFireProjectile::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	pev->renderamt = 255.0f;
	pev->rendermode = kRenderTransAlpha;

	const Vector one = Vector( 1, 1, 1 );
	SET_MODEL( edict(), "sprites/fthrow.spr" );
	UTIL_SetSize( pev, -one * 2.0f, one * 2.0f );
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CFireProjectile::FireThink );
	SetTouch( &CFireProjectile::FireTouch );

	pev->nextthink = gpGlobals->time + 0.1f;
}

void CFireProjectile::Precache()
{
	PRECACHE_MODEL( "sprites/fthrow.spr" );
	FireSprite = PRECACHE_MODEL( "sprites/fire.spr" );
}

int CFireProjectile::Classify()
{
	return CLASS_NONE;
}

void CFireProjectile::FireThink()
{
	pev->nextthink = gpGlobals->time + 0.1f;
	pev->scale *= 1.25f;
	pev->frame += 1.0f;

	if ( pev->frame > 15.0f )
	{
		pev->renderamt = 0.0f;
		UTIL_Remove( this );
	}
}

void CFireProjectile::FireTouch( CBaseEntity* pOther )
{
	// Don't hurt the original player who launched this
	// But also ignore any accidental collisions with other projectiles
	if ( pOther->edict() == pev->owner || dynamic_cast<CFireProjectile*>(pOther) != nullptr )
	{
		return;
	}

	// Entities that take damage are typically NPCs and players, so we'll
	// treat them as characters. This will also be called for func_breakable,
	// which is no character, but oh well!
	if ( pOther->pev->takedamage != DAMAGE_NO )
	{
		OnHitCharacter( pOther );
	}
	else // func_wall, worldspawn and others will just make it disappear
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		OnHitWorld( pOther, tr.vecEndPos, tr.vecPlaneNormal );
	}

	UTIL_Remove( this );
}

void CFireProjectile::OnHitCharacter( CBaseEntity* pCharacter )
{
	pCharacter->TakeDamage( pev, &pev->owner->v, 1.0f, DMG_BURN );

	// 50% chance of the victim catching fire
	if ( RANDOM_LONG( 0, 1 ) )
	{
		CFireSticker::CreateFireSticker( pCharacter, Instance( pev->owner ) );
	}
}

void CFireProjectile::OnHitWorld( CBaseEntity* pOther, Vector origin, Vector surfaceNormal )
{
	float dot = DotProduct( surfaceNormal, Vector( 0.0f, 0.0f, 1.0f ) );
	if ( dot < 0.34f ) // roughly 70 degrees
	{
		return;
	}

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );

	WRITE_BYTE( TE_FIREFIELD );
	WRITE_COORD( origin.x );
	WRITE_COORD( origin.y );
	// offset the Z a little bit so the fire mostly isn't inside the ground
	WRITE_COORD( origin.z + 30.0f );
	WRITE_SHORT( 32 ); // radius
	WRITE_SHORT( FireSprite );
	WRITE_BYTE( 5 ); // count
	WRITE_BYTE( TEFIRE_FLAG_LOOP // loop at 15fps
		| TEFIRE_FLAG_PLANAR // spawn on a flat plane
		| TEFIRE_FLAG_ADDITIVE ); // additive render mode
	WRITE_BYTE( 150 ); // 15 secs

	MESSAGE_END();
}

CFireProjectile* CFireProjectile::CreateFire( Vector start, Vector aim, CBaseEntity* owner )
{
	Vector randomVector = Vector(
		RANDOM_FLOAT( -1.0f, 1.0f ),
		RANDOM_FLOAT( -1.0f, 1.0f ),
		RANDOM_FLOAT( -1.0f, 1.0f )
	);

	// NOTE: This is a modified GetClassPtr with the parametre a = nullptr by default
	CFireProjectile* pFire = GetClassPtr<CFireProjectile>();
	pFire->pev->classname = MAKE_STRING( "ft_fire" );
	pFire->pev->origin = start +
		gpGlobals->v_right * 20.0f +
		gpGlobals->v_up * 10.0f +
		aim * 60.0f +
		// Randomise the starting position a bit so they don't end up in each other
		randomVector * 4.0f;
	pFire->pev->angles = g_vecZero;
	pFire->pev->velocity = aim.Normalize() * 400.0f + randomVector * 25.0f;
	pFire->pev->owner = owner->edict();
	pFire->Spawn();

	return pFire;
}

#endif

LINK_WEAPON_TO_CLASS( weapon_flamethrower, CFlamethrower );

constexpr const char* WorldModel = "models/w_egon.mdl";
constexpr const char* ViewModel = "models/v_egon.mdl";
constexpr const char* PlayerModel = "models/p_egon.mdl";

void CFlamethrower::Spawn()
{
	Precache();
	m_iId = WEAPON_FLAMETHROWER;
	SET_MODEL( ENT( pev ), WorldModel );

	m_iDefaultAmmo = FLAMETHROWER_DEFAULT_GIVE;

	FallInit();
}

void CFlamethrower::Precache()
{
	UTIL_PrecacheOther( "ft_fire" );
	UTIL_PrecacheOther( "ft_sticker" );

	PRECACHE_MODEL( WorldModel );
	PRECACHE_MODEL( ViewModel );
	PRECACHE_MODEL( PlayerModel );
}

bool CFlamethrower::GetItemInfo( ItemInfo* p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iId = m_iId = WEAPON_FLAMETHROWER;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

void CFlamethrower::PrimaryAttack()
{
	// don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 )
	{
		PlayEmptySound();
		return;
	}

	//if (m_iClip <= 0)
	//{
	//	PlayEmptySound();
	//	return;
	//}

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	CFireProjectile::CreateFire( vecSrc, vecAiming, m_pPlayer );
#endif

	// I can't bother writing these events. That's homework for you if you wanna
	// do extra stuff like playing sounds etc.
	SendWeaponAnim( FLAMETHROWER_FIRE1 + RANDOM_LONG( 0, 3 ) );

	m_iClip--;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.1f );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

bool CFlamethrower::Deploy()
{
	m_fireState = FIRE_OFF;
	return DefaultDeploy( "models/v_egon.mdl", "models/p_egon.mdl", FLAMETHROWER_DRAW, "egon" );
}

void CFlamethrower::WeaponIdle()
{
	if ( (m_pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 && (m_pPlayer->pev->button & IN_ATTACK) != 0 )
	{
		return;
	}

	ResetEmptySound();

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;

	float flRand = RANDOM_FLOAT( 0, 1 );

	if ( flRand <= 0.5 )
	{
		iAnim = FLAMETHROWER_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
	else
	{
		iAnim = FLAMETHROWER_FIDGET1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	}

	SendWeaponAnim( iAnim );
}
