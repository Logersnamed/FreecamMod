#pragma once
#include "core/game_data/common.h"

namespace GameData {
    enum class ChrDbgFlags : std::uint8_t {
        playerNoDead                 = 0x00,
        playerHorseNoDead            = 0x01,
        playerExterminate            = 0x02,
        allNoGoodsConsume            = 0x03,
        allNoStaminaConsume          = 0x04,
        allNoFPConsume               = 0x05,
        allNoArrowConsume            = 0x06,
        allNoMagicQtyConsume         = 0x07,
        playerHide                   = 0x08,
        playerSilence                = 0x09,
        allNoDead                    = 0x0A,
        allNoDamage                  = 0x0B,
        allNoHit                     = 0x0C,
        allNoAttack                  = 0x0D,
        allNoMove                    = 0x0E,
        allNoUpdateAI                = 0x0F,
        allNoWepProtDurabilityDamage = 0x10,
        allNoAshOfWarFPConsume       = 0x11,
        allNoGoodsConsume2           = 0x12,
        forceParryMode               = 0x13,
        allOmissionMode              = 0x28,
        omissionLvl                  = 0x29
    };
}