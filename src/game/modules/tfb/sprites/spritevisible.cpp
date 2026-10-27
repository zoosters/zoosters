#include "modules/tfb/sprites/gamesprite.h"

// 0x461b40
bool Sprite_IsVisible(RWS::TFB::GameSprite *sprite)
{
	if (!sprite->m_visible)
		return false;
	RWS::TFB::GameSprite *parent = sprite->m_parent;
	if (parent && (sprite->m_inheritFlags & 8))   // 8: the visibility follows the parent (as ExposeVisible::GetInt)
		return parent->m_visible ? true : false;
	return true;
}
