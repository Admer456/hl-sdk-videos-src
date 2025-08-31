#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "customentity.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL

class CHook : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	void Touch( CBaseEntity* pOther ) override;
	void Disengage();

	static CHook* Create( CBaseEntity* owner, CGrapple* weapon, Vector origin, Vector direction );

	bool IsLanded() const { return landed; }

private:
	void MaintainRopeBeam();

private:
	EHANDLE player = {};
	bool landed = false;
	int ropeSprite = 0;
	CGrapple* owningWeapon = nullptr;
	CBeam* beam = nullptr;
};

LINK_ENTITY_TO_CLASS( ghook, CHook );

void CHook::Spawn()
{
	Precache();

	SET_MODEL( edict(), "models/riblet1.mdl" );
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	UTIL_SetOrigin( pev, pev->origin );

	player = Instance( pev->owner );
	pev->nextthink = gpGlobals->time + 0.5f;

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.5f;
	pev->friction = 0.5f;

	MaintainRopeBeam();
}

void CHook::Precache()
{
	PRECACHE_MODEL( "models/riblet1.mdl" );
	ropeSprite = PRECACHE_MODEL( "sprites/tongue.spr" );
}

template<typename T>
static T UTIL_Lerp( T a, T b, float alpha )
{
	return a + alpha * ( b - a );
}

template<typename T>
static T UTIL_Damp( T source, T target, float alpha, float deltaTime )
{
	return UTIL_Lerp( target, source, std::pow( alpha, deltaTime ) );
}

// Assumes the normal points towards the sphere's centre
static Vector UTIL_ProjectWithinSphere( Vector direction, Vector normal )
{
	const float dot = DotProduct( direction, normal );

	// Discard if the vector goes *into* the sphere
	// We only want to wrap around the sphere if it's going outside of it
	if ( dot > 0.0f )
	{
		return direction;
	}
	
	return direction - DotProduct( direction, normal ) * normal;
}

void CHook::Think()
{
	pev->nextthink = gpGlobals->time + 0.001f;

	if ( !landed )
	{
		return;
	}

	const Vector delta = pev->origin - player->pev->origin;
	const float distance = owningWeapon->GetDistance();
	const float distanceToPlayer = delta.Length();
	const float distanceFromBorder = distanceToPlayer - distance;

	// I've tried experimenting with:
	// 1. A quadratic formula - gives a super stronger pull at great distances
	//const float modifier = (distanceFromBorder * distanceFromBorder) / 20.0f;
	// 2. A linear formula - feels the most natural?
	const float modifier = 10.0f * distanceFromBorder;
	// 3. A square root formula - feels off NGL
	//const float modifier = 200.0f * sqrt( distanceFromBorder * 2.0f );

	// If the player is outside the sphere, align the velocity with the sphere's surface
	const Vector directionAlongSphere = UTIL_ProjectWithinSphere( player->pev->velocity.Normalize(), delta.Normalize() );
	const Vector velocityAlongSphere = directionAlongSphere * player->pev->velocity.Length();
	// But, if the player is inside the sphere, "blur" said alignment in a 20-unit zone
	const float factorAlongSphere = std::fmax( 0.0f, std::fmin( 1.0f, distanceFromBorder / 20.0f ) );
	const Vector newVelocityBase = UTIL_Lerp( player->pev->velocity, velocityAlongSphere, factorAlongSphere );
	
	// This is a bit like applying an impulse in other engines. It results in what's
	// essentially acceleration. Important note: delta is normalised here.
	player->pev->velocity = newVelocityBase + delta.Normalize() * modifier * gpGlobals->frametime;

	// Framerate-independent damping in my HL SDK??? No way!
	// This damping could be a little bit better if you only dampened the movement towards the
	// sphere's centre, and let movement *along* the sphere's surface fluctuate freely.
	player->pev->velocity = UTIL_Damp( player->pev->velocity, g_vecZero, 0.5f, gpGlobals->frametime );
}

void CHook::Touch( CBaseEntity* pOther )
{
	if ( !FClassnameIs( pOther->pev, "worldspawn" ) )
	{
		Disengage();
		return;
	}

	landed = true;
	pev->velocity = g_vecZero;

	// Include a bit of pull from the get go
	owningWeapon->SetDistance( (player->pev->origin - pev->origin).Length() - 32.0f );

	// No more touchy
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
}

void CHook::Disengage()
{
	UTIL_Remove( beam );
	beam = nullptr;

	// Let the gun fire again
	owningWeapon->Unlock();
	owningWeapon = nullptr;
	UTIL_Remove( this );
}

CHook* CHook::Create( CBaseEntity* owner, CGrapple* weapon, Vector origin, Vector direction )
{
	CHook* hook = GetClassPtr<CHook>();
	hook->pev->owner = owner->edict();
	hook->pev->origin = origin + direction * 40.0f;
	hook->pev->velocity = direction * 1500.0f;
	hook->owningWeapon = weapon;
	hook->Spawn();

	return hook;
}

void CHook::MaintainRopeBeam()
{
	if ( owningWeapon == nullptr )
	{
		return;
	}

	if ( beam == nullptr )
	{
		beam = CBeam::BeamCreate( "sprites/tongue.spr", 2.0f );
		beam->EntsInit( player->entindex(), entindex() );
		beam->SetFlags( BEAM_FSOLID );
		beam->SetBrightness( 255.0f );
		beam->SetEndAttachment( 0 );
		beam->pev->spawnflags |= SF_BEAM_TEMPORARY;
	}
}

#endif

LINK_WEAPON_TO_CLASS( weapon_grapple, CGrapple );

constexpr const char* WorldModel = "models/w_crossbow.mdl";
constexpr const char* ViewModel = "models/v_crossbow.mdl";
constexpr const char* PlayerModel = "models/p_crossbow.mdl";

void CGrapple::Spawn()
{
	Precache();
	m_iId = WEAPON_GRAPPLE;
	SET_MODEL( ENT( pev ), WorldModel );

	m_iDefaultAmmo = WEAPON_NOCLIP;
	m_iClip = WEAPON_NOCLIP;

	FallInit();
}

void CGrapple::Precache()
{
	UTIL_PrecacheOther( "ghook" );
	PRECACHE_MODEL( WorldModel );
	PRECACHE_MODEL( ViewModel );
	PRECACHE_MODEL( PlayerModel );
}

bool CGrapple::GetItemInfo( ItemInfo* p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 0;
	p->iId = m_iId = WEAPON_GRAPPLE;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

void CGrapple::PrimaryAttack()
{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.01f );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

#ifndef CLIENT_DLL
	if ( m_Hook != nullptr )
	{
		if ( static_cast<CHook*>(m_Hook)->IsLanded() )
		{
			m_Distance -= 300.0f * gpGlobals->frametime;
			if ( m_Distance < 30.0f )
			{
				m_Distance = 30.0f;
			}
		}

		return;
	}
#endif

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	m_Hook = CHook::Create( m_pPlayer, this, vecSrc, vecAiming );
#endif
}

void CGrapple::SecondaryAttack()
{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.01f );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

#ifndef CLIENT_DLL
	if ( m_Hook != nullptr )
	{
		if ( static_cast<CHook*>(m_Hook)->IsLanded() )
		{
			m_Distance += 300.0f * gpGlobals->frametime;
			if ( m_Distance > 700.0f )
			{
				m_Distance = 700.0f;
			}
		}
	}
#endif
}

void CGrapple::Reload()
{
#ifndef CLIENT_DLL
	if ( m_Hook == nullptr )
	{
		return;
	}

	static_cast<CHook*>(m_Hook)->Disengage();
#endif

	m_fInReload = false;
}

bool CGrapple::Deploy()
{
	m_fireState = FIRE_OFF;
	return DefaultDeploy( ViewModel, PlayerModel, PEARL_DRAW, "bow" );
}

void CGrapple::WeaponIdle()
{
	if ( (m_pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 && (m_pPlayer->pev->button & IN_ATTACK) != 0 )
	{
		return;
	}

	ResetEmptySound();

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;
}
