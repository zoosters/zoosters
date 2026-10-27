#ifndef ZOOSTERS_TFB_SPRITES_SPRITEMANAGER_API_H
#define ZOOSTERS_TFB_SPRITES_SPRITEMANAGER_API_H

namespace RWS { namespace TFB { class GameSprite; } }

RWS::TFB::GameSprite *FindSpriteByName(const char *name);   // first sprite of the active set with this name (case-insensitive)
void RemoveSpriteByName(const char *name);
void ShowCaption(const char *name);
void HideCaption(const char *name);
RWS::TFB::GameSprite *SpriteMgr_CreateTextSprite(const char *name, const char *text, float x, float y, float width, float height);
void SpriteMgr_SetTextSprite(const char *name, const char *text, float x, float y, float height, float width);
// InvalidateSprite: re-resolve the sprite and its children; null = rebuild the render list.
namespace Script { void InvalidateSprite(RWS::TFB::GameSprite *sprite); }

#endif
