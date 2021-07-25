#pragma once
#include <cstdint>

constexpr uint64_t COIN                       = (uint64_t)100000000; // 1 WORKTIPS = pow(10, 8)
constexpr uint64_t MONEY_SUPPLY               = (UINT64_C(184467440737095516)); // MONEY_SUPPLY - total number coins to be generated
constexpr uint64_t EMISSION_SPEED_FACTOR_PER_MINUTE = (20);
constexpr uint64_t FINAL_SUBSIDY_PER_MINUTE = ((uint64_t)300000000000); // 3 * pow(10, 11)
constexpr uint64_t EMISSION_LINEAR_BASE       = ((uint64_t)(1) << 58);
 constexpr uint64_t YEARLY_INFLATION_INVERSE   = 200; // 0.5% yearly inflation, inverted for integer division
constexpr uint64_t EMISSION_SUPPLY_MULTIPLIER = 19;
constexpr uint64_t EMISSION_SUPPLY_DIVISOR    = 10;
constexpr uint64_t EMISSION_DIVISOR           = 2000000;

// PREMINE
constexpr uint64_t PREMINE = ((uint64_t)(81600000000000000));

// HF15 money supply parameters:
constexpr uint64_t BLOCK_REWARD_HF15      = 25 * COIN;
constexpr uint64_t MINER_REWARD_HF15      = BLOCK_REWARD_HF15 * 24 / 100; // Only until HF16
constexpr uint64_t SN_REWARD_HF15         = BLOCK_REWARD_HF15 * 66 / 100;
constexpr uint64_t FOUNDATION_REWARD_HF15 = BLOCK_REWARD_HF15 * 10 / 100;

// HF16+ money supply parameters: same as HF15 except the miner fee goes away and is redirected to
// LF to be used exclusively for Worktips Chainflip liquidity seeding and incentives.  See
// https://github.com/worktips-project/worktips-improvement-proposals/issues/24 for more details.  This ends
// after 6 months.
constexpr uint64_t BLOCK_REWARD_HF16        = BLOCK_REWARD_HF15;
constexpr uint64_t CHAINFLIP_LIQUIDITY_HF16 = BLOCK_REWARD_HF15 * 24 / 100;

// HF17: at most 6 months after HF16.  This is tentative and will likely be replaced before the
// actual HF with a new reward schedule including Chainflip rewards, but as per the LRC linked
// above, the liquidity funds end after 6 months.  That means that until HF17 is finalized, this is
// the fallback if we hit the 6-months-after-HF16 point:
constexpr uint64_t BLOCK_REWARD_HF17      = 18'333'333'333;
constexpr uint64_t FOUNDATION_REWARD_HF17 =  1'833'333'333;

static_assert(MINER_REWARD_HF15        + SN_REWARD_HF15 + FOUNDATION_REWARD_HF15 == BLOCK_REWARD_HF15);
static_assert(CHAINFLIP_LIQUIDITY_HF16 + SN_REWARD_HF15 + FOUNDATION_REWARD_HF15 == BLOCK_REWARD_HF16);
static_assert(                           SN_REWARD_HF15 + FOUNDATION_REWARD_HF17 == BLOCK_REWARD_HF17);

// -------------------------------------------------------------------------------------------------
//
// Blink
//
// -------------------------------------------------------------------------------------------------
// Blink fees: in total the sender must pay (MINER_TX_FEE_PERCENT + BURN_TX_FEE_PERCENT) * [minimum tx fee] + BLINK_BURN_FIXED,
// and the miner including the tx includes MINER_TX_FEE_PERCENT * [minimum tx fee]; the rest must be left unclaimed.
constexpr uint64_t BLINK_MINER_TX_FEE_PERCENT = 100; // The blink miner tx fee (as a percentage of the minimum tx fee)
constexpr uint64_t BLINK_BURN_FIXED           = 0;  // A fixed amount (in atomic currency units) that the sender must burn
constexpr uint64_t BLINK_BURN_TX_FEE_PERCENT_V15  = 150; // A percentage of the minimum miner tx fee that the sender must burn.  (Adds to BLINK_BURN_FIXED)
constexpr uint64_t BLINK_BURN_TX_FEE_PERCENT_V18  = 200; // A percentage of the minimum miner tx fee that the sender must burn.  (Adds to BLINK_BURN_FIXED)

static_assert(BLINK_MINER_TX_FEE_PERCENT >= 100, "blink miner fee cannot be smaller than the base tx fee");
static_assert(BLINK_BURN_FIXED >= 0, "fixed blink burn amount cannot be negative");
static_assert(BLINK_BURN_TX_FEE_PERCENT_V18 >= 0, "blink burn tx percent cannot be negative");

// -------------------------------------------------------------------------------------------------
//
// ONS
//
// -------------------------------------------------------------------------------------------------
namespace ons
{
enum struct mapping_type : uint16_t
{
  session = 0,
  wallet = 1,
  worktipsnet = 2, // the type value stored in the database; counts as 1-year when used in a buy tx.
  worktipsnet_2years,
  worktipsnet_5years,
  worktipsnet_10years,
  _count,
  update_record_internal,
};

constexpr bool is_worktipsnet_type(mapping_type t) { return t >= mapping_type::worktipsnet && t <= mapping_type::worktipsnet_10years; }

// How many days we add per "year" of ONS worktipsnet registration.  We slightly extend this to the 368
// days per registration "year" to allow for some blockchain time drift + leap years.
constexpr uint64_t REGISTRATION_YEAR_DAYS = 368;

constexpr uint64_t burn_needed(uint8_t hf_version, mapping_type type)
{
  uint64_t result = 0;

  // The base amount for session/wallet/worktipsnet-1year:
  const uint64_t basic_fee = (
      hf_version >= 18 ? 7*COIN :  // cryptonote::network_version_18
      hf_version >= 16 ? 15*COIN :  // cryptonote::network_version_16_pulse -- but don't want to add cryptonote_config.h include
      20*COIN                       // cryptonote::network_version_15_ons
  );
  switch (type)
  {
    case mapping_type::update_record_internal:
      result = 0;
      break;

    case mapping_type::worktipsnet: /* FALLTHRU */
    case mapping_type::session: /* FALLTHRU */
    case mapping_type::wallet: /* FALLTHRU */
    default:
      result = basic_fee;
      break;

    case mapping_type::worktipsnet_2years: result = 2 * basic_fee; break;
    case mapping_type::worktipsnet_5years: result = 4 * basic_fee; break;
    case mapping_type::worktipsnet_10years: result = 6 * basic_fee; break;
  }
  return result;
}
}; // namespace ons

