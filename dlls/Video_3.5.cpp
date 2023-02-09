#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h" // Make sure to include this! CSprite and CBeam are here

class SpriteExample : public CBaseEntity
{
public:
	void Spawn() override
	{
		// Here we abuse 'netname' to store the model path,
		// couldn't bother writing keyvalues
		Precache();
		SET_MODEL(edict(), STRING(pev->netname));
	}

	void Precache() override
	{
		PRECACHE_MODEL(STRING(pev->netname));
	}

	// When used, this entity will spawn a sprite at a random location relative to this entity
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override
	{
		const Vector randomPosition = pev->origin + Vector{
			RANDOM_FLOAT(-64.0f, 64.0f),
			RANDOM_FLOAT(-64.0f, 64.0f),
			RANDOM_FLOAT(-64.0f, 64.0f)};

		CSprite* sprite = CSprite::SpriteCreate(STRING(pev->netname), randomPosition, true);
		sprite->SetScale(RANDOM_FLOAT(0.25f, 1.5f));
		sprite->SetTransparency(pev->rendermode, 255, 255, 255, 255, pev->renderfx);
		sprite->AnimateAndDie(RANDOM_FLOAT(10.0f, 20.0f)); // different framerates = more visual variation
	}
};

LINK_ENTITY_TO_CLASS(sprite_example, SpriteExample);

class SpriteExample2 : public CBaseEntity
{
public:
	void Spawn() override
	{
		Precache();

		for (auto& sprite : sprites) // https://www.learncpp.com/cpp-tutorial/for-each-loops/
		{
			sprite = CSprite::SpriteCreate("sprites/glow01.spr", pev->origin, false);
			sprite->SetTransparency(kRenderTransAdd, 64, 255, 64, 255, kRenderFxNone);
		}

		// Homework: "start inverted" spawnflag
		sprites[1]->TurnOff(); // The 2nd sprite must start off
		sprites[1]->SetColor(255, 64, 64);
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/glow01.spr");
	}

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override
	{
		sprites[counter % 2]->TurnOff(); // Turn off is 1st, because the 1st sprite starts on
		counter++;						 // ++counter vs. counter++ is just stupid, don't bother; do it outside like this instead
		sprites[counter % 2]->TurnOn();
	}

private:
	CSprite* sprites[2];
	int counter = 0;
};

LINK_ENTITY_TO_CLASS(sprite_example2, SpriteExample2);

class SpriteExample3 : public CBaseEntity
{
public:
	static constexpr int MaxSprites = 32;
	static constexpr float SpriteLife = 0.5f;

	void Spawn() override
	{
		Precache();

		for (int i = 0; i < MaxSprites; i++)
		{
			sprites[i] = CSprite::SpriteCreate("sprites/glow01.spr", pev->origin, false);
			sprites[i]->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone);
			sprites[i]->pev->movetype = MOVETYPE_NOCLIP;
			ResetSprite(i);
		}

		pev->nextthink = gpGlobals->time + 0.1f;
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/glow01.spr");
	}

	void Think() override
	{
		for (int i = 0; i < MaxSprites; i++)
		{
			if (spriteLives[i] <= 0.0f)
			{
				ResetSprite(i);
			}

			sprites[i]->SetBrightness(spriteLives[i] * 255.0f);
			sprites[i]->pev->velocity.y += RANDOM_FLOAT(-20.0f, 20.0f);
			sprites[i]->pev->velocity.z += RANDOM_FLOAT(-20.0f, 20.0f);
			spriteLives[i] -= 0.05f;
		}

		// If we update this too frequently, it might use up too much bandwidth
		// 20 Hz seems smooth enough
		pev->nextthink = gpGlobals->time + 0.05f;
	}

private:
	void ResetSprite(int index)
	{
		sprites[index]->SetBrightness(255);
		sprites[index]->pev->velocity = Vector{RANDOM_FLOAT(200.0f, 400.0f), 0.0f, 0.0f};
		// Respawn the sprite somewhere on a plane behind the parent entity
		sprites[index]->pev->origin = pev->origin + Vector{
			-64.0f, RANDOM_FLOAT(-64.0f, 64.0f), RANDOM_FLOAT(-64.0f, 64.0f)};
		
		spriteLives[index] = SpriteLife - RANDOM_FLOAT(0.0f, 0.2f);
	}

private:
	CSprite* sprites[MaxSprites];
	float spriteLives[MaxSprites];
};

LINK_ENTITY_TO_CLASS(sprite_example3, SpriteExample3);

class BeamExample : public CBaseEntity
{
public:
	static constexpr int MaxBeams = 8;

public:
	void Spawn() override
	{
		Precache();
		for (auto& beam : beams)
		{
			beam = CBeam::BeamCreate("sprites/lgtning.spr", 8);
			beam->SetNoise(5);
			beam->Spawn();
		}

		UTIL_MakeVectorsPrivate(pev->angles, pev->movedir, nullptr, nullptr);

		pev->nextthink = gpGlobals->time + 1.5f;
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/lgtning.spr");
	}

	void Think() override
	{
		pev->angles.x += 0.0083f;
		pev->angles.y += 0.013f;
		UTIL_MakeVectorsPrivate(pev->angles, pev->movedir, nullptr, nullptr);

		Vector vecStart = pev->origin;
		Vector vecDirection = pev->movedir;
		Vector vecEnd = vecStart + vecDirection * 512.0f;

		for (int i = 0; i < MaxBeams; i++)
		{
			// Draw a beam from where the trace started, to where it ends
			TraceResult tr{};
			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);
			beams[i]->SetStartPos(vecStart);
			beams[i]->SetEndPos(tr.vecEndPos);
			beams[i]->SetBrightness((MaxBeams - i) * (255.0f / MaxBeams));

			// Update the next trace with this same stuff
			vecStart = tr.vecEndPos;
			vecDirection = Reflect(vecDirection, tr.vecPlaneNormal);
			vecEnd = vecStart + vecDirection * 512.0f;
		}

		// If updated too frequently, might cause lots of bandwidth usage
		pev->nextthink = gpGlobals->time + 0.02f;
	}

	static inline Vector Reflect(Vector direction, Vector normal)
	{
		const float doubleDot = 2.0f * DotProduct(direction, normal);
		return direction - (doubleDot * normal);
	}

private:
	CBeam* beams[MaxBeams];
};

LINK_ENTITY_TO_CLASS(beam_example, BeamExample);
