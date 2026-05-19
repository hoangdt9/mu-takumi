#pragma once

// -----------------------------------------------------------------------------
// Android UI FPS (inventory / skill picker) — Tuấn Thanh style optimizations:
// - Sparse inventory grid (fewer immediate-mode quads)
// - Skill picker iterates learned skills only
// - Inventory item icons: 3D only for hover / drag (cached UI textures elsewhere)
// Set to 0 locally to A/B test against legacy UI cost.
// -----------------------------------------------------------------------------
#if defined(__ANDROID__)

#ifndef TAKUMI_ANDROID_UI_PERF
#define TAKUMI_ANDROID_UI_PERF 1
#endif

#ifndef TAKUMI_ANDROID_UI_SPARSE_INVENTORY_GRID
#define TAKUMI_ANDROID_UI_SPARSE_INVENTORY_GRID 0
#endif

#ifndef TAKUMI_ANDROID_UI_LIGHT_INVENTORY_ITEM_3D
#define TAKUMI_ANDROID_UI_LIGHT_INVENTORY_ITEM_3D 0
#endif

#ifndef TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
#define TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE 1
#endif

#ifndef TAKUMI_ANDROID_UI_PNG_HUD_OVERLAY
#define TAKUMI_ANDROID_UI_PNG_HUD_OVERLAY 0
#endif

#ifndef TAKUMI_ANDROID_UI_ITEM_PLACEHOLDER_2D
#define TAKUMI_ANDROID_UI_ITEM_PLACEHOLDER_2D 0
#endif

#ifndef TAKUMI_ANDROID_UI_SKIP_HOTKEY_ITEM_3D
#define TAKUMI_ANDROID_UI_SKIP_HOTKEY_ITEM_3D 1
#endif

// Verbose UI_LOAD / UI_IFACE / UI_BCUSTOM trace to TakumiErrorReport (off in normal play).
#ifndef TAKUMI_ANDROID_UI_DEBUG_TRACE
#define TAKUMI_ANDROID_UI_DEBUG_TRACE 0
#endif

#else

#define TAKUMI_ANDROID_UI_PERF 0
#define TAKUMI_ANDROID_UI_SPARSE_INVENTORY_GRID 0
#define TAKUMI_ANDROID_UI_LIGHT_INVENTORY_ITEM_3D 0
#define TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE 0
#define TAKUMI_ANDROID_UI_PNG_HUD_OVERLAY 0
#define TAKUMI_ANDROID_UI_ITEM_PLACEHOLDER_2D 0
#define TAKUMI_ANDROID_UI_SKIP_HOTKEY_ITEM_3D 0
#define TAKUMI_ANDROID_UI_DEBUG_TRACE 0

#endif
