/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_ASSET_H_
#include "RaidCore_Common.h"
#include "RaidCore_Math.h"
#include "RaidCore_RenderHandler.h"
#include "RaidCore_MemManager.h"

struct loaded_sound {
    int32 SampleCount;
    void *Memory;
};

enum asset_state {
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
    AssetState_Locked,
};
struct asset_slot {
    asset_state State;
    union {
        game_render_engine::GfxTexture *Bitmap;
        loaded_sound *Sound;
    };
};

enum asset_tag_id {
    Tag_Smoothness,
    Tag_Flatness,
    Tag_FacingDirection, // NOTE(roman): Angle in radians off of due right
    Tag_Animation,

    Tag_Count,
};

enum AssetType {
    AssetType_None = 0,
    AssetType_Shadow,
    AssetType_Tree,
    AssetType_Rock,
    AssetType_Stone,
    AssetType_Step,
    AssetType_Grass,
    AssetType_Tuft,

	// NOTE(Roman): Statically allocated assets,
	//  these have their own data storage that is never removed for the entire game
	// ** internally the assets may change! such as different weapon is used, or hero changes cloths, etc.
	AssetType_HeroHead,
	AssetType_HeroCape,
	AssetType_HeroTorso,
	AssetType_HeroWeapon,
	AssetType_HeroWeaponAction,
	AssetType_HeroShadow,
	AssetType_HeroWalk_S,
	AssetType_HeroStrike_S,
	AssetType_HeroJump_S,
	AssetType_HeroWeaponAction_S,

    // debug stuff
    dAssetType_debugMark,
    dAssetType_Shadow,
    dAssetType_Tree,
    dAssetType_Rock,
    dAssetType_Stone,
    dAssetType_Step,
    dAssetType_Grass,
    dAssetType_Tuft,
    dAssetType_HeroHead,
    dAssetType_HeroCape,
    dAssetType_HeroTorso,
    dAssetType_HeroWeapon,
	dAssetType_HeroWeaponAction,
	dAssetType_HeroShadow,
    //
    Asset_Count
};
//
// 
struct asset_tag {
    uint32 ID;
    real32 Value;
};
struct asset_type {
    uint32 FirstAssetIndex;
    uint32 OnePastLastAssetIndex;
};
struct asset_bitmap_info {
    char *FileName;
    vec2 AlignPercentage;
};
struct asset_sound_info {
    char *FileName;
};
struct asset {
    uint32 FirstTagIndex;
    uint32 OnePastLastTagIndex;

    union {
        asset_bitmap_info bitmapInfo;
        asset_sound_info soundInfo;
    };
};
struct asset_vector {
    real32 E[Tag_Count];
};

struct GameAssets {
    struct TemporaryState *tranState;
    game_memory::arena* arena;

    real32 tagRange[Tag_Count];

    uint32 tagCount;
    asset_tag *tags;

    uint32 assetCount;
    asset *assets;
    asset_slot *slots;

    asset_type assetTypes[Asset_Count];

    // NOTE(roman): These should go away once we actually load a asset pack file,
    // but could stay, for flexibility
    uint32 nextFreeSlotId;
    uint32 nextFreeAssetId;
    uint32 nextFreeTagId;
};

void LoadGameBitmap(GameState* gameState, BitmapID ID);
void LoadSound(GameState* gameState, SoundID ID);

#define __RC_ASSET_H_
#endif//__RC_ASSET_H_
