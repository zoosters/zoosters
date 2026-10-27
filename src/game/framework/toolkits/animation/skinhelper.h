#pragma once
struct RpAtomic;

namespace RWS {
namespace SkinHelper {

// Calls m_callback(skin, m_pData) for each skinned atomic until it returns 0.
struct SkinCallbackData {
	int (*m_callback)(void *skin, void *data);
	void *m_pData;
};

int FindFirstSkinCallback(void *skin, void **out);
RpAtomic *ForAllSkinsCallback(RpAtomic *atomic, SkinCallbackData *cbData);

}   // namespace SkinHelper
}   // namespace RWS
