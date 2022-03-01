/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
#include "../RaidCore/Headers/RaidCore_FileFormats.h"
#include "../RaidCore/Headers/RaidCore_Asset.h"
#include "../RaidCore/Headers/RaidCore_Platform.h"
#include "../RaidCore/Headers/RaidCore_Task.h"
#include "../RaidCore/Headers/RaidCore_SoundSystem.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"

inline vec2 TopDownAlign(game_render_engine::GfxTexture *Bitmap, vec2 Align) {
    Align.y = (real32)(Bitmap->imageStats.height - 1) - Align.y;

    Align.x = game_math::safeRatio0(Align.x, (real32)Bitmap->imageStats.width);
    Align.y = game_math::safeRatio0(Align.y, (real32)Bitmap->imageStats.height);

    return(Align);
}

struct load_bitmap_work {
    platform_load_file *loadFile;
    GameAssets *assets;
    BitmapID ID;
    TaskWithMemory *Task;
    game_render_engine::GfxTexture *Bitmap;

    asset_state FinalState;
};

game_memory::MemoryBlock loadBitmapFile(platform_load_file* load, const char * path, const vec2& alignment, game_memory::arena_p pArena) {
    game_memory::MemoryBlock mem = load(path, pArena);
    if (mem.m_pMemory) {
        TemporaryMemory memory = BeginTemporaryMemory(pArena);
        //
        game_memory::MemoryBlock bmp = getBitmapRaw(mem.m_pMemory, pArena);
        game_render_engine::GfxTexture * bitmap = (game_render_engine::GfxTexture *)bmp.m_pMemory;

        //        bitmap->alignPercentage.x = game_math::safeRatio0(alignment.x, (real32)bitmap->imageStats.width);
        //        bitmap->alignPercentage.y = game_math::safeRatio0(((real32)(bitmap->imageStats.height - 1) - alignment.y), (real32)bitmap->imageStats.height);
        bitmap->alignPercentage = alignment;
        //
        EndTemporaryMemory(memory);
        return (bmp);
    }
    return mem;
}


internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapWork) {
    load_bitmap_work *Work = (load_bitmap_work *)Data;

    asset_type& type = Work->assets->assetTypes[Work->ID.Value];
    for (uint32 id = type.OnePastLastAssetIndex - 1; id >= type.FirstAssetIndex; --id) {
        if (AtomicCompareUInt32((uint32 *)&Work->assets->slots[id].State, AssetState_Queued) == AssetState_Queued) {
            asset_bitmap_info *Info = &Work->assets->assets[id].bitmapInfo;
            game_memory::MemoryBlock bitmapMemory = loadBitmapFile(Work->loadFile, Info->FileName, Info->AlignPercentage, &Work->Task->arena);

            if (NULL != bitmapMemory.m_pMemory) {
                CompletePreviousWritesBeforeFutureWrites;
                Work->Bitmap = (game_render_engine::GfxTexture*)PushSizeP(Work->assets->arena, bitmapMemory.m_iMemoryLength);
                copyMemory((uint8*)Work->Bitmap, bitmapMemory.m_pMemory, bitmapMemory.m_iMemoryLength);
                Work->Bitmap->data = ((uint8*)Work->Bitmap) + game_memory::GetAlignedSize(sizeof(game_render_engine::GfxTexture), 16);

                Work->assets->slots[id].Bitmap = Work->Bitmap;
                Work->assets->slots[id].State = Work->FinalState;
            } else {
                Work->assets->slots[id].State = AssetState_Unloaded;
            }
        }
    }

    EndTaskWithMemory(Work->Task);
}

void LoadGameBitmap(GameState* gameState, BitmapID ID) {
    GameAssets *assets = gameState->assets;
    asset_type& type = assets->assetTypes[ID.Value];
    if (ID.Value &&
        (AtomicCompareExchangeUInt32((uint32 *)&assets->slots[type.FirstAssetIndex].State, AssetState_Queued, AssetState_Unloaded) ==
        AssetState_Unloaded)) {
        for (uint32 slotId = type.FirstAssetIndex + 1; slotId < type.OnePastLastAssetIndex; ++slotId) {
            AtomicCompareExchangeUInt32((uint32 *)&assets->slots[slotId].State, AssetState_Queued, AssetState_Unloaded);
        }
        TaskWithMemory *Task = BeginTaskWithMemory(assets->tranState->Tasks, ArrayCount(assets->tranState->Tasks));
        if (Task) {
            load_bitmap_work *Work = PushStruct(&Task->arena, load_bitmap_work);
            Work->loadFile = gameState->loadFile;

            Work->assets = assets;
            Work->ID = ID;
            Work->Task = Task;
            Work->Bitmap = NULL;// PushStruct(&assets->tranState->TranArena, game_render_engine::GfxTexture);
            Work->FinalState = AssetState_Loaded;

            gameState->addEntry(assets->tranState->LowPriorityQueue, LoadBitmapWork, Work);
        } else {
            //AtomicExchangeU32((uint32 *)&assets->slots[type.FirstAssetIndex].State, AssetState_Unloaded);
            for (uint32 slotId = type.FirstAssetIndex; slotId < type.OnePastLastAssetIndex; ++slotId) {
                AtomicExchangeU32((uint32 *)&assets->slots[slotId].State, AssetState_Unloaded);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------
// Sound

internal loaded_sound
DEBUGLoadWAV(char *FileName, platform_api::PlatformApi api, game_memory::arena* arena) {
    loaded_sound Result = {};

    TemporaryMemory memory = BeginTemporaryMemory(arena);
    game_memory::MemoryBlock ReadResult = api.loadFile(FileName, arena);
    if (ReadResult.m_iMemoryLength != 0) {
        file_formats::WAVE_header *Header = (file_formats::WAVE_header *)ReadResult.m_pMemory;
        Assert(Header->RIFFID == file_formats::WAVE_ChunkID_RIFF);
        Assert(Header->WAVEID == file_formats::WAVE_ChunkID_WAVE);
    }
    EndTemporaryMemory(memory);

    return(Result);
}

struct load_sound_work {
    platform_api::PlatformApi api;
    GameAssets *assets;
    SoundID ID;
    TaskWithMemory *Task;
    loaded_sound *Sound;

    asset_state FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadSoundWork) {
    load_sound_work *Work = (load_sound_work *)Data;

    asset_sound_info *Info = &Work->assets->assets[Work->ID.Value].soundInfo;
    *Work->Sound = DEBUGLoadWAV(Info->FileName, Work->api, &Work->Task->arena);

    CompletePreviousWritesBeforeFutureWrites;

    Work->assets->slots[Work->ID.Value].Sound = Work->Sound;
    Work->assets->slots[Work->ID.Value].State = Work->FinalState;

    EndTaskWithMemory(Work->Task);
}

void LoadSound(GameState* gameState, SoundID ID) {
    GameAssets *assets = gameState->assets;
    asset_type& type = assets->assetTypes[ID.Value];
    if (ID.Value &&
        (AtomicCompareExchangeUInt32((uint32 *)&assets->slots[type.FirstAssetIndex].State, AssetState_Queued, AssetState_Unloaded) ==
        AssetState_Unloaded)) {
        TaskWithMemory *Task = BeginTaskWithMemory(assets->tranState->Tasks, ArrayCount(assets->tranState->Tasks));
        if (Task) {
            load_sound_work *Work = PushStruct(&Task->arena, load_sound_work);

            Work->assets = assets;
            Work->ID = ID;
            Work->Task = Task;
            Work->Sound = PushStruct(&assets->tranState->TranArena, loaded_sound);
            Work->FinalState = AssetState_Loaded;

            gameState->addEntry(assets->tranState->LowPriorityQueue, LoadSoundWork, Work);
        } else {
            AtomicExchangeU32((uint32 *)&assets->slots[type.FirstAssetIndex].State, AssetState_Unloaded);
        }
    }
}

//-----------------------------------------------------------------------------------------------------------------------------
#include "Headers\RaidCore_RandomMath.h"

internal BitmapID
BestMatchAsset(GameAssets *assets, AssetType TypeID,
asset_vector *MatchVector, asset_vector *WeightVector) {
    BitmapID Result = {};
    real32 BestDiff = FLT_MAX;
    asset_type *Type = assets->assetTypes + TypeID;
    for (uint32 AssetIndex = Type->FirstAssetIndex;
         AssetIndex < Type->OnePastLastAssetIndex;
         ++AssetIndex) {
        asset *Asset = assets->assets + AssetIndex;

        real32 TotalWeightedDiff = 0.0f;
        for (uint32 TagIndex = Asset->FirstTagIndex;
             TagIndex < Asset->OnePastLastTagIndex;
             ++TagIndex) {
            asset_tag *Tag = assets->tags + TagIndex;

            real32 A = MatchVector->E[Tag->ID];
            real32 B = Tag->Value;
            real32 D0 = game_math::absolute(A - B);
            real32 D1 = game_math::absolute((A - assets->tagRange[Tag->ID] * game_math::sign(A)) - B);
            real32 Difference = _Min(D0, D1);

            real32 Weighted = WeightVector->E[Tag->ID] * Difference;
            TotalWeightedDiff += Weighted;
        }
        if (BestDiff > TotalWeightedDiff) {
            BestDiff = TotalWeightedDiff;
            Result.Value = AssetIndex;
        }
    }
    return(Result);
}

internal BitmapID
RandomAssetFrom(GameAssets *assets, AssetType TypeID, random::random_series *Series) {
    BitmapID Result = {};
    asset_type *Type = assets->assetTypes + TypeID;
    if (Type->FirstAssetIndex != Type->OnePastLastAssetIndex) {
        uint32 Count = (Type->OnePastLastAssetIndex - Type->FirstAssetIndex);
        uint32 Choice = RandomChoice(Series, Count);
        Result.Value = Type->FirstAssetIndex + Choice;
    }
    return(Result);
}

internal BitmapID
GetFirstBitmapID(GameAssets *assets, AssetType TypeID) {
    BitmapID Result = {};
    asset_type *Type = assets->assetTypes + TypeID;
    if (Type->FirstAssetIndex != Type->OnePastLastAssetIndex) {
        Result.Value = Type->FirstAssetIndex;
    }
    return(Result);
}

internal BitmapID
GetLastBitmapID(GameAssets *assets, AssetType TypeID) {
    BitmapID Result = {};
    asset_type *Type = assets->assetTypes + TypeID;
    if (Type->FirstAssetIndex != Type->OnePastLastAssetIndex) {
        Result.Value = Type->OnePastLastAssetIndex - 1;
    }
    return(Result);
}

rc_macro_cpp game_render_engine::GfxTexture *GetBitmap(GameAssets *assets, BitmapID ID) {
    game_render_engine::GfxTexture *Result = assets->slots[ID.Value].Bitmap;
    return(Result);
}

internal asset_type*
BeginAssetType(GameAssets *assets, AssetType TypeID) {
    asset_type* result = assets->assetTypes + TypeID;
    result->FirstAssetIndex = assets->nextFreeAssetId;
    result->OnePastLastAssetIndex = result->FirstAssetIndex;
    return (result);
}

internal asset*
AddBitmapAsset(GameAssets *assets, asset_type* assetType, char *FileName, vec2 AlignPercentage = v2(0.5f, 0.5f)) {
    Assert(assetType);
    Assert(assetType->OnePastLastAssetIndex < assets->assetCount);

    asset *Asset = assets->assets + assetType->OnePastLastAssetIndex++;
    Asset->FirstTagIndex = assets->nextFreeTagId;
    Asset->OnePastLastTagIndex = Asset->FirstTagIndex;

    BitmapID ID = { assets->nextFreeSlotId++ };

    asset_bitmap_info *Info = &assets->assets[ID.Value].bitmapInfo;
    Info->FileName = FileName;
    Info->AlignPercentage = AlignPercentage;

    return (Asset);
}

internal void
AddTag(GameAssets *assets, asset* Asset, asset_tag_id ID, real32 Value) {
    Assert(Asset);

    ++Asset->OnePastLastTagIndex;
    asset_tag *Tag = assets->tags + assets->nextFreeTagId++;

    Tag->ID = ID;
    Tag->Value = Value;
}

internal void
EndAssetType(GameAssets *assets, asset_type* assetType) {
    Assert(assetType);
    assets->nextFreeAssetId = assetType->OnePastLastAssetIndex;
}

// NOTE(Roman): here we should allocate the static asset data structures, and initialize the GameAssets structure
//  loading the assets should happen from an asset pack.
GameAssets* AllocateGameAssets(game_memory::arena *arena, memory_int Size, TemporaryState *tranState) {
    GameAssets *assets = PushStruct(arena, GameAssets);
    setBytes32(assets, 0, sizeof(GameAssets));
    assets->arena = arena;
    assets->tranState = tranState;

    for (uint32 TagType = 0;
         TagType < Tag_Count;
         ++TagType) {
        assets->tagRange[TagType] = 1000000.0f;
    }
    assets->tagRange[Tag_FacingDirection] = Tau32;

    assets->assetCount = 2 * 256 * Asset_Count;
    assets->slots = PushArray(arena, assets->assetCount, asset_slot);
    assets->assets = PushArray(arena, assets->assetCount, asset);

    assets->tagCount = 1024 * Asset_Count;
    assets->tags = PushArray(arena, assets->tagCount, asset_tag);

    assets->nextFreeSlotId = 1;
    assets->nextFreeAssetId = 1;

    asset_type* assetType = NULL;
    asset* Asset = NULL;

	// NOTE(Roman) : alocate the static assets first
	//
	real32 AngleRight = 0.0f*Tau32;
	real32 AngleBack = 0.25f*Tau32;
	real32 AngleLeft = 0.5f*Tau32;
	real32 AngleFront = 0.75f*Tau32;

    vec2 HeroAlign = { 0.5f, 0.16129f };

	assetType = BeginAssetType(assets, AssetType_HeroHead);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_head.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_head.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_head.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_head.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
	EndAssetType(assets, assetType);

	assetType = BeginAssetType(assets, AssetType_HeroCape);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_cape.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_cape.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_cape.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_cape.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
	EndAssetType(assets, assetType);

	assetType = BeginAssetType(assets, AssetType_HeroTorso);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_torso.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_torso.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_torso.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
	Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_torso.bmp", HeroAlign);
	AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
	EndAssetType(assets, assetType);

	assetType = BeginAssetType(assets, AssetType_HeroShadow);
	AddBitmapAsset(assets, assetType, "data\\test\\test_hero_shadow.bmp", HeroAlign);
	EndAssetType(assets, assetType);

	assetType = BeginAssetType(assets, AssetType_HeroJump_S);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	assetType = BeginAssetType(assets, AssetType_HeroStrike_S);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	assetType = BeginAssetType(assets, AssetType_HeroWalk_S);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	assetType = BeginAssetType(assets, AssetType_HeroWeaponAction_S);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	assetType = BeginAssetType(assets, AssetType_HeroWeapon);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	assetType = BeginAssetType(assets, AssetType_HeroWeaponAction);
	//AddSoundAsset(assets, assetType, "");
	EndAssetType(assets, assetType);
	//----------------------------------------------------------//
	// NOTE(Roman) : alocate the rest
	//

    vec2 TreeAlign = v2(0.493827164f, 0.295652181f);
    assetType = BeginAssetType(assets, AssetType_Tree);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree00.bmp", TreeAlign);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree01.bmp", TreeAlign);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree02.bmp", TreeAlign);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, AssetType_Grass);
    AddBitmapAsset(assets, assetType, "data\\test2\\grass00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\grass01.bmp");
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, AssetType_Tuft);
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft01.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft02.bmp");
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, AssetType_Stone);
    AddBitmapAsset(assets, assetType, "data\\test2\\ground00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground01.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground02.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground03.bmp");
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, AssetType_Step);
    AddBitmapAsset(assets, assetType, "data\\test2\\step00.bmp");
    EndAssetType(assets, assetType);

    //////
    assetType = BeginAssetType(assets, dAssetType_HeroHead);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_head.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_Animation, 0.0f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_head.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_Animation, 0.0f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_head.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_Animation, 0.0f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_head.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_Animation, 0.0f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
    //--------------------------------------------------------------------------------------------
    vec2 HeroAlign2 = { 0.5f, 0.22129f };

    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_head.bmp", HeroAlign2);
    AddTag(assets, Asset, Tag_Animation, 0.5f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_head.bmp", HeroAlign2);
    AddTag(assets, Asset, Tag_Animation, 0.5f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_head.bmp", HeroAlign2);
    AddTag(assets, Asset, Tag_Animation, 0.5f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_head.bmp", HeroAlign2);
    AddTag(assets, Asset, Tag_Animation, 0.5f);
    AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_HeroCape);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_cape.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_cape.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_cape.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_cape.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_HeroTorso);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_right_torso.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleRight);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_back_torso.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleBack);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_left_torso.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleLeft);
    Asset = AddBitmapAsset(assets, assetType, "data\\test\\test_hero_front_torso.bmp", HeroAlign);
    AddTag(assets, Asset, Tag_FacingDirection, AngleFront);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_HeroShadow);
    AddBitmapAsset(assets, assetType, "data\\test\\test_hero_shadow.bmp", HeroAlign);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_Tree);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree00.bmp", TreeAlign);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree01.bmp", TreeAlign);
    AddBitmapAsset(assets, assetType, "data\\test2\\tree02.bmp", TreeAlign);
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_Grass);
    AddBitmapAsset(assets, assetType, "data\\test2\\grass00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\grass01.bmp");
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_Tuft);
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft01.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\tuft02.bmp");
    EndAssetType(assets, assetType);

    assetType = BeginAssetType(assets, dAssetType_Stone);
    AddBitmapAsset(assets, assetType, "data\\test2\\ground00.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground01.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground02.bmp");
    AddBitmapAsset(assets, assetType, "data\\test2\\ground03.bmp");
    EndAssetType(assets, assetType);

    vec2 FloorAlign = { 0.5f, 1.0f };
    assetType = BeginAssetType(assets, dAssetType_Step);
    AddBitmapAsset(assets, assetType, "data\\test2\\step00.bmp", FloorAlign);
    EndAssetType(assets, assetType);

    return(assets);
}
