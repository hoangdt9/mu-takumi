///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "NewUISystem.h"
#if defined(__ANDROID__) || defined(MU_IOS)
#include "Platform/gl_compat.h"
#endif
#include <algorithm>
#include <cstdint>
#include <vector>

namespace
{
constexpr float kSpriteFrameMin = 0.2f;
constexpr float kSpriteFrameMax = 1.0f;
constexpr float kSpriteFrameStep = 0.1f;

float ComputeFrameDelta()
{
    return kSpriteFrameStep * FPS_ANIMATION_FACTOR;
}

float UpdateAnimationFrame(float currentFrame, bool isVisible)
{
    const float delta = ComputeFrameDelta();
    currentFrame += isVisible ? delta : -delta;
    return std::clamp(currentFrame, kSpriteFrameMin, kSpriteFrameMax);
}

#if defined(__ANDROID__) || defined(MU_IOS)
enum class SpriteBatchBlendMode : uint8_t
{
    AlphaBlend,
    AlphaBlendMinus,
    AlphaTest,
    AlphaBlend2,
};

struct SpriteQuadBatch
{
    std::vector<float> vertices;
    int texture = -1;
    SpriteBatchBlendMode blendMode = SpriteBatchBlendMode::AlphaBlend;
    int quadCount = 0;
};

SpriteBatchBlendMode DetermineSpriteBlendMode(const OBJECT* o)
{
    if (o->Type == BITMAP_FORMATION_MARK || o->SubType == 2)
    {
        return SpriteBatchBlendMode::AlphaTest;
    }

    if (o->SubType == 1)
    {
        return SpriteBatchBlendMode::AlphaBlendMinus;
    }

    if (o->SubType == 3)
    {
        return SpriteBatchBlendMode::AlphaBlend2;
    }

    return SpriteBatchBlendMode::AlphaBlend;
}

void ApplySpriteBlendMode(SpriteBatchBlendMode mode)
{
    switch (mode)
    {
    case SpriteBatchBlendMode::AlphaBlend:
        EnableAlphaBlend();
        break;
    case SpriteBatchBlendMode::AlphaBlendMinus:
        EnableAlphaBlendMinus();
        break;
    case SpriteBatchBlendMode::AlphaTest:
        EnableAlphaTest();
        break;
    case SpriteBatchBlendMode::AlphaBlend2:
        EnableAlphaBlend2();
        break;
    }
}

void FlushSpriteQuadBatch(SpriteQuadBatch& batch)
{
    if (batch.quadCount <= 0 || batch.vertices.empty() || batch.texture < 0)
    {
        batch.vertices.clear();
        batch.texture = -1;
        batch.quadCount = 0;
        return;
    }

    ApplySpriteBlendMode(batch.blendMode);
    BindTexture(batch.texture);
    GL_DrawQuadsBulk(batch.vertices.data(), batch.quadCount);

    batch.vertices.clear();
    batch.texture = -1;
    batch.quadCount = 0;
}

void AppendSpriteBatchVertex(std::vector<float>& vertices, const vec3_t position, const float uv[2], const float color[4])
{
    vertices.push_back(position[0]);
    vertices.push_back(position[1]);
    vertices.push_back(position[2]);
    vertices.push_back(color[0]);
    vertices.push_back(color[1]);
    vertices.push_back(color[2]);
    vertices.push_back(color[3]);
    vertices.push_back(uv[0]);
    vertices.push_back(uv[1]);
}

void QueueSpriteQuadBatch(SpriteQuadBatch& batch, OBJECT* o)
{
    o->AnimationFrame = UpdateAnimationFrame(o->AnimationFrame, o->Visible);
    float scale = o->AnimationFrame * o->Scale;

    BITMAP_t* bitmap = Bitmaps.GetTexture(o->Type);
    float width = bitmap->Width * scale;
    float height = bitmap->Height * scale;

    float u = 0.f;
    float v = 0.f;
    float uWidth = 1.f;
    float vHeight = 1.f;
    if (o->Type == BITMAP_FORMATION_MARK)
    {
        width = 64.f;
        height = 64.f;
        uWidth = 0.33f;
        vHeight = 0.33f;
        switch (o->SubType)
        {
        case 1:
            u = 0.33f;
            break;
        case 2:
            u = 0.66f;
            break;
        case 3:
            v = 0.33f;
            break;
        case 4:
            u = 0.33f;
            v = 0.33f;
            break;
        case 5:
            u = 0.66f;
            v = 0.33f;
            break;
        case 6:
            v = 0.66f;
            break;
        case 7:
            u = 0.33f;
            v = 0.66f;
            break;
        }
    }

    vec3_t transformedPosition;
    VectorTransform(o->Position, CameraMatrix, transformedPosition);
    float x = transformedPosition[0];
    float y = transformedPosition[1];
    float z = transformedPosition[2];
    width *= 0.5f;
    height *= 0.5f;

    vec3_t positions[4];
    if (o->Angle[2] == 0.f)
    {
        Vector(x - width, y - height, z, positions[0]);
        Vector(x + width, y - height, z, positions[1]);
        Vector(x + width, y + height, z, positions[2]);
        Vector(x - width, y + height, z, positions[3]);
    }
    else
    {
        vec3_t localPositions[4];
        Vector(-width, -height, z, localPositions[0]);
        Vector(width, -height, z, localPositions[1]);
        Vector(width, height, z, localPositions[2]);
        Vector(-width, height, z, localPositions[3]);

        vec3_t rotationAngle;
        Vector(0.f, 0.f, o->Angle[2], rotationAngle);
        float rotationMatrix[3][4];
        AngleMatrix(rotationAngle, rotationMatrix);
        for (int i = 0; i < 4; ++i)
        {
            VectorRotate(localPositions[i], rotationMatrix, positions[i]);
            positions[i][0] += x;
            positions[i][1] += y;
        }
    }

    float texCoords[4][2];
    TEXCOORD(texCoords[3], u, v);
    TEXCOORD(texCoords[2], u + uWidth, v);
    TEXCOORD(texCoords[1], u + uWidth, v + vHeight);
    TEXCOORD(texCoords[0], u, v + vHeight);

    float color[4] = { o->Light[0], o->Light[1], o->Light[2], 1.f };

    if (o->Type == BITMAP_BLOOD + 1 || o->Type == BITMAP_FONT_HIT)
    {
        color[3] = 1.f;
    }
    else if (o->SubType == 0)
    {
        color[3] = o->Light[0];
    }

    const SpriteBatchBlendMode blendMode = DetermineSpriteBlendMode(o);
    if (batch.quadCount > 0 && (batch.texture != o->Type || batch.blendMode != blendMode))
    {
        FlushSpriteQuadBatch(batch);
    }

    if (batch.vertices.empty())
    {
        batch.vertices.reserve(4096);
    }

    batch.texture = o->Type;
    batch.blendMode = blendMode;
    for (int i = 0; i < 4; ++i)
    {
        AppendSpriteBatchVertex(batch.vertices, positions[i], texCoords[i], color);
    }
    ++batch.quadCount;
}
#endif
}

OBJECT	Sprites   [MAX_SPRITES];
inline SpinLock* g_CreateSprite_lock = new SpinLock();
int CreateSprite(int Type,vec3_t Position,float Scale,vec3_t Light,OBJECT *Owner,float Rotation,int SubType)
{
	if (!g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[g_pNewUISystem->GetUI_NewOptionWindow()->eEffectStatic]) return false;

#if defined(__ANDROID__) || defined(MU_IOS)
	if (ShouldThrottleAdaptiveEffectSpawn(ADAPTIVE_EFFECT_SPRITE, Type, Position, SubType, Scale, Owner))
	{
		return false;
	}
#endif

	//g_CreateSprite_lock->lock();
	for(int i=0;i<MAX_SPRITES;i++)
	{
		OBJECT *o = &Sprites[i];
		//== Fix Effect
		if (i >= (MAX_SPRITES - 2) && o->Live)
		{
			o->Live = false;
			o->Visible = false;
		}
		if(!o->Live)
		{
			o->Live           = true;
			o->Type           = Type;
			o->SubType        = SubType;
			o->Owner          = Owner;
			o->AnimationFrame = 1.f;
    		o->Scale          = Scale;
			o->Angle[2]       = Rotation;
			VectorCopy(Position,o->Position);
			VectorCopy(Position,o->StartPosition);
			VectorCopy(Light,o->Light);
			//g_CreateSprite_lock->unlock();
			return i;
		}
	}
	//g_CreateSprite_lock->unlock();
	return false;
}

void RenderSprite(OBJECT *o,OBJECT *Owner)
{
	if (GetRenderEffect() == false) return;

	if (o->Visible)
	{
		o->AnimationFrame += 0.1f * static_cast<float>(FPS_ANIMATION_FACTOR);
		if (o->AnimationFrame > 1.f)
		{
			o->AnimationFrame = 1.f;
		}
	}
	else
	{
		o->AnimationFrame -= 0.1f * static_cast<float>(FPS_ANIMATION_FACTOR);
		if (o->AnimationFrame < 0.2f)
		{
			o->AnimationFrame = 0.2f;
		}
	}
	float Scale = o->AnimationFrame*o->Scale;
	
	BITMAP_t* pBitmap = Bitmaps.GetTexture(o->Type);
	float Width  = pBitmap->Width * Scale;
	float Height = pBitmap->Height * Scale;

    if ( o->Type==BITMAP_FORMATION_MARK )
    {
		float u = 0.0f, v = 0.0f, uw, vw;
		uw=0.33f; vw=0.33f;
        switch ( o->SubType )
        {
        case 0:
            u=0.f; v=0.f;
            break;

        case 1:
            u=0.33f; v=0.f;
            break;

        case 2:
            u=0.66f; v=0.f;
            break;

        case 3:
            u=0.f; v=0.33f;
            break;

        case 4:
            u=0.33f; v=0.33f;
            break;

        case 5:
            u=0.66f; v=0.33f;
            break;

        case 6:
            u=0.f; v=0.66f;
            break;

        case 7:
            u=0.33f; v=0.66f;
            break;
        }

        RenderSprite( o->Type, o->Position, 64, 64, o->Light, o->Angle[2], u, v, uw, vw );
    }
    else
    {
        RenderSprite(o->Type,o->Position,Width,Height,o->Light,o->Angle[2]);
    }
}

void RenderSprites ( BYTE byRenderOneMore )
{
#if defined(__ANDROID__) || defined(MU_IOS)
    SpriteQuadBatch spriteBatch;
    spriteBatch.vertices.reserve(4096);
    const bool restoreDepthTest = DepthTestEnable;
#endif
	for(int i=0;i<MAX_SPRITES;i++)
	{
		OBJECT *o = &Sprites[i];
        if( byRenderOneMore == 1 )
        {
            if ( o->Position[2] > 150.f ) 
			{
				continue;
			}
        }
        else if( byRenderOneMore == 2 )
        {
            if( o->Position[2] <= 100.f )
            {
                o->Live = false;
                continue;
            }
        }

		if(o->Live)
		{
#if defined(__ANDROID__) || defined(MU_IOS)
            QueueSpriteQuadBatch(spriteBatch, o);
#else
            if( o->Type == BITMAP_FORMATION_MARK )
            {
                EnableAlphaTest ();
            }
            else if(o->SubType == 0)
			{
          	    EnableAlphaBlend();
			}
			else if( o->SubType==1 )
			{
               	EnableAlphaBlendMinus();
			}
            else if( o->SubType==2 )
			{
                EnableAlphaTest();
			}
            else if( o->SubType==3 )
			{
                EnableAlphaBlend2();
			}
    		RenderSprite(o,o->Owner);
#endif

            if( byRenderOneMore == 0 || byRenderOneMore == 2 )
            {
                o->Live = false;
            }
		}
	}
#if defined(__ANDROID__) || defined(MU_IOS)
    FlushSpriteQuadBatch(spriteBatch);
    DisableAlphaBlend();
    if (restoreDepthTest)
    {
        EnableDepthTest();
    }
    else
    {
        DisableDepthTest();
    }
#endif
}

void CheckSprites()
{
	for(int i=0; i<MAX_SPRITES; i++)
	{
		OBJECT *o = &Sprites[i];
		if(o->Live)
		{
         	o->Visible = true;
		}
	}
}
