#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

// static values
static constexpr extended_symbol VALUE_SYM = { symbol {"USDT", 4}, "tethertether"_n };
static const set<name> STATUS_TYPES = set<name>{
    "pending"_n,        // pending admin approval and funding
    "open"_n,           // open for applications and funding
    "started"_n,        // hunter approved by author and work in progress
    "submitted"_n,      // hunter submitted the work, waiting for for approval from author
    "released"_n,       // author approved work, ready for claim
    "done"_n,           // bounty claimed, bounty successfully completed
    "closed"_n          // bounty closed by author/admin
};
static const set<name> STATUS_DEPOSIT_TYPES = set<name>{"pending"_n, "open"_n, "started"_n};
static const set<name> BOUNTY_TYPES = set<name>{"traditional"_n};
static constexpr uint32_t DAY = 86400;

static const string ERROR_INVALID_MEMO = "invalid transfer memo (ex: \"mybounty[,author.eosn]\")";

class [[eosio::contract("work.pomelo")]] pomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `configs`
     *
     * ### params
     *
     * - `{name} status` - contract status ("ok", "testing", "maintenance")
     * - `{uint64_t} fee` - platform fee (bips - 1/100 1%)
     * - `{name} login_contract` - EOSN Login contract
     * - `{name} fee_account` - fee
     * - `{set<name>} metadata_keys` - list of keys allowed to include in bounty Metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "fee": 1000,
     *     "login_contract": "login.eosn",
     *     "fee_account": "fee.pomelo",
     *     "metadata_keys": ["url"]
     * }
     * ```
     */
    struct [[eosio::table("configs")]] configs_row {
        name            status = "testing"_n;
        uint64_t        fee = 1000;
        name            login_contract = "login.eosn"_n;
        name            fee_account = "fee.pomelo"_n;
        set<name>       metadata_keys = {"url"_n};
    };
    typedef eosio::singleton< "configs"_n, configs_row > configs_table;

    /**
     * ## TABLE `tokens`
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} min_amount` - min amount required per bounty
     * - `{uint64_t} max_amount` - max amount required per bounty
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,USDT",
     *     "contract": "tethertether",
     *     "min_amount": 50000,
     *     "max_amount": 5000000
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol              sym;
        name                contract;
        uint64_t            min_amount;
        uint64_t            max_amount;

        uint64_t primary_key() const { return sym.code().raw(); }
    };
    typedef eosio::multi_index< "tokens"_n, tokens_row> tokens_table;

    /**
     * ## TABLE `bounties`
     *
     * *scope*: `{name} get_self()`
     *
     * ## params
     *
     * - `{name} bount_id` - (primary key) bounty ID (ex: "bounty1")
     * - `{name} author_user_id` - author (EOSN Login ID)
     * - `{map<name, asset>} funders` - funders contributions map (EOSN Login id => amount)
     * - `{extended_asset} amount` - amount of tokens to be released once bounty is completed
     * - `{set<name>} applicant_user_ids` - list of applicants that have applied to bounty
     * - `{set<name>} approved_user_id` - approved account for bounty
     * - `{name} status="pending"` - status (`pending/open/started/submitted/done`)
     * - `{name} type="traditional"` - bounty type (`traditional` = "1 worker at a time, 1 is paid out")
     * - `{name} permissions="approval"` - bounty permissions (`approval` = "Funder must approve hunter to start work")
     * - `{map<name, string>} metadata={}` - bounty metadata
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} submitted_at` - submitted at time
     * - `{time_point_sec} completed_at` - completed at time
     *
     * ### example
     *
     * ```json
     * {
     *     "bount_id": "bounty1",
     *     "author_user_id": "author.eosn",
     *     "funders": {"funder1.eosn": "5.0000 USDT", "funder2.eosn": "5.0000 USDT"}
     *     "amount": {"quantity": "10.0000 USDT", "contract": "tethertether"},
     *     "claimed": "10.0000 USDT",
     *     "applicant_user_ids": ["hunter.eosn"],
     *     "approved_user_id": "hunter.eosn",
     *     "status": "pending",
     *     "type": "traditional",
     *     "permissions": "approval",
     *     "metadata": [{"key": "url", "value": "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"}],
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     *     "submitted_at": "1970-01-01T00:00:00",
     *     "completed_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table]] bounties_row {
        name                    bounty_id;
        name                    author_user_id;
        map<name, asset>        funders;
        extended_asset          amount;
        extended_asset          fee;
        asset                   claimed;
        set<name>               applicant_user_ids;
        name                    approved_user_id;
        name                    status = "pending"_n;
        name                    type = "traditional"_n;
        name                    permissions = "approval"_n;
        map<name, string>       metadata;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          submitted_at;
        time_point_sec          completed_at;

        uint64_t primary_key() const { return bounty_id.value; }
    };
    typedef eosio::multi_index< "bounties"_n, bounties_row> bounties_table;

    /**
     * ## ACTION `setconfig`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} [status]` - contract status: ok/testing/disabled
     * - `{uint64_t} [fee]` - platform fee (bips - 1/100 1%)
     * - `{name} [login_contract]` - EOSN Login contract
     * - `{name} [fee_account]` - fee account
     * - `{set<name>} metadata_keys` - allowed metadata keys
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo setconfig '[ok, 1000, "login.eosn", "fee.pomelo", ["url"]]' -p work.pomelo
     * $ cleos push action work.pomelo setconfig '[disabled, null, null, null, []]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const optional<name> status, const optional<uint64_t> fee, const optional<name> login_contract, const optional<name> fee_account, const set<name> metadata_keys );

    /**
     * ## ACTION `token`
     *
     * - **authority**: `get_self()`
     *
     * Set token as supported asset
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} min_amount` - min amount required per bounty
     * - `{uint64_t} max_amount` - max amount required per bounty
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo token '["4,USDT", "tethertether", 50000, 5000000]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t max_amount );

    /**
     * ## ACTION `deltoken`
     *
     * - **authority**: `get_self()`
     *
     * Delete token from supported assets
     *
     * ### params
     *
     * - `{symbol_code} symcode` - symbol code
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo deltoken '["EOS"]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void deltoken( const symbol_code symcode );

    /**
     * ## ACTION `create`
     *
     * - **authority**: `author_user_id`
     *
     * Create bounty
     *
     * ### params
     *
     * - `{name} author_user_id` - author (EOSN Login ID)
     * - `{name} bount_id` - bounty ID
     * - `{symbol_code} accepted_token` - accepted deposit token (ex: `"USDT"`)
     * - `{optional<name>} bounty_type` - bounty type (default = traditional)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action work.pomelo create '[author.eosn, bounty1, "USDT", null]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void create( const name author_user_id, const name bounty_id, const symbol_code accepted_token, const optional<name> bounty_type );

    /**
     * ## ACTION `setstate`
     *
     * - **authority**: `get_self()`
     *
     * Set bounty state
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} status` - status `pending/published/banned/retired/denied'
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo setstate '[bounty1, published]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void setstate( const name bounty_id, const name state );

    /**
     * ## ACTION `syncbounty`
     *
     * - **authority**: `get_self()`
     *
     * Sync bounty details from backend. Called by `/admin/sync` on backend if there are inconsistencies between backend and blockchain.
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} status` - status `pending/published/banned/retired/denied'
     * - `{vector<name>} applicant_user_ids` - list of applicants
     * - `{optional<name>} approved_user_id` - approved applicant
     * - `{time_point_sec} updated_at` - last update time
     * - `{optional<time_point_sec>} submitted_at` - submitted time
     * - `{optional<time_point_sec>} completed_at` - completed time
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo syncbounty '[bounty1, started, [applicant1.eosn, applicant2.eosn], applicant2.eosn, 2021-01-01T00:00:00, null, null]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void syncbounty( const name bounty_id, const name state, const vector<name> applicant_user_ids, const optional<name> approved_user_id, const time_point_sec updated_at, const optional<time_point_sec> submitted_at, const optional<time_point_sec> completed_at );

    /**
     * ## ACTION `setmetadata`
     *
     * - **authority**: `author_user_id`
     *
     * Add/remove metadata. If metadata_value == "" - remove it.
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} metadata_key` - one of the allowed metadata keys
     * - `{string} metadata_value` - metadata value
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo setmetadata '[bounty1, url, "https://github.com/pomelo-io/pomelo-rest-api/issues/735"]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void setmetadata( const name bounty_id, const name metadata_key, const string metadata_value );

    /**
     * ## ACTION `approve`
     *
     * - **authority**: bounty `author_user_id`
     *
     * Author select hunter for bounty
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} applicant_user_id - approved applicant (EOSN Login ID)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo approve '[bounty1, hunter.eosn]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void approve( const name bounty_id, const name applicant_user_id );

    /**
     * ## ACTION `forfeit`
     *
     * - **authority**: `hunter_user_id`
     *
     * Hunter forfeits the bounty and puts the bounty into `open` state
     *
     * > Bounty state must be "started"
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo forfeit '[bounty1]' -p hunter.eosn
     * ```
     */
    [[eosio::action]]
    void forfeit( const name bounty_id );

    /**
     * ## ACTION `close`
     *
     * - **authority**: `author_user_id`
     *
     * Author closes the bounty and puts it into `closed` state
     *
     * > Bounty state must be "open" or "pending"
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo close '[bounty1]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void close( const name bounty_id );

    /**
     * ## ACTION `publish`
     *
     * - **authority**: `admin`
     *
     * Admin publishes the bounty making it open for applications
     *
     * > Bounty state must be "pending"
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo publish '[bounty1]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void publish( const name bounty_id );

    /**
     * ## ACTION `release`
     *
     * - **authority**: `author_user_id`
     *
     * Author releases funds to hunter when bounty is completed
     *
     * > Bounty state must be "completed"
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo release '[bounty1]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void release( const name bounty_id );

    /**
     * ## ACTION `deny`
     *
     * - **authority**: `author_user_id`
     *
     * Author denies completed bounty
     *
     * > Bounty state is reverted back to "progress"
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo deny '[bounty1]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void deny( const name bounty_id );

    /**
     * ## ACTION `withdraw`
     *
     * - **authority**: `author_user_id` (EOSN Login)
     *
     * Author withdraws funds from bounty in "pending" state
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} chain` - chain name
     * - `{name} receiver` - receiver (Antelope account or EVM address)
     * - `{string} [memo=""]` - (optional) memo (only available when using Antelope receiver)
     *
     * ### example
     *
     * ```bash
     * // withdraw to EOS Native
     * $ cleos push action work.pomelo withdraw '[bounty1, eos, "myaccount", null]' -p author.eosn
     *
     * // withdraw to Exchange
     * $ cleos push action work.pomelo withdraw '[bounty1, eos, "eosbndeposit", "100245696"]' -p author.eosn
     *
     * // withdraw to EOS EVM
     * $ cleos push action work.pomelo withdraw '[bounty1, eos.evm, "0xaa2F34E41B397aD905e2f48059338522D05CA534", null]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void withdraw( const name bounty_id, const name chain, const string receiver, const optional<string> memo );

    /**
     * ## ACTION `apply`
     *
     * - **authority**: `user_id`
     *
     * Hunter apply to bounty
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} user_id` - user ID (EOS account linked to EOSN Login)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo apply '[bounty1, hunter.eosn]' -p hunter.eosn
     * ```
     */
    [[eosio::action]]
    void apply( const name bounty_id, const name user_id );

    /**
     * ## ACTION `complete`
     *
     * - **authority**: `user_id`
     *
     * hunter signals work is completed
     *
     * > Funds are auto-released after 72 hours if no explicit approval from author
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo complete '[bounty1]' -p hunter.eosn
     * ```
     */
    [[eosio::action]]
    void complete( const name bounty_id );

    /**
     * ## ACTION `claim`
     *
     * - **authority**: `approved_user_id` (EOSN Login)
     *
     * Hunter claims bounty funds
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} chain` - chain name
     * - `{name} receiver` - receiver (Antelope account or EVM address)
     * - `{string} [memo=""]` - (optional) memo (only available when using Antelope receiver)
     *
     * ### example
     *
     * ```bash
     * // claim to EOS Native
     * $ cleos push action work.pomelo claim '[bounty1, eos, "myaccount", null]' -p claimer.eosn
     *
     * // claim to Exchange
     * $ cleos push action work.pomelo claim '[bounty1, eos, "eosbndeposit", "100245696"]' -p claimer.eosn
     *
     * // claim to EOS EVM
     * $ cleos push action work.pomelo claim '[bounty1, eos.evm, "0xaa2F34E41B397aD905e2f48059338522D05CA534", null]' -p claimer.eosn
     * ```
     */
    [[eosio::action]]
    void claim( const name bounty_id, const name chain, const string receiver, const optional<string> memo );

    /**
     * ## TRANSFER NOTIFY HANDLER `on_transfer`
     *
     * Process incoming transfer
     *
     * ### params
     *
     * - `{name} from` - from EOS account (donation sender)
     * - `{name} to` - to EOS account (process only incoming)
     * - `{asset} quantity` - quantity received
     * - `{string} memo` - transfer memo, i.e. "bounty1,author.eosn"
     *
     * ### example
     *
     * ```bash
     * $ cleos transfer author work.pomelo "5.0000 USDT" "bounty1,author.eosn" --contract tethertether -p author.eosn
     * ```
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    [[eosio::action]]
    void depositlog( const name bounty_id, const name funder_user_id, const name from, const extended_asset ext_quantity, const asset fee, const double value, const string& memo );

    [[eosio::action]]
    void createlog( const name bounty_id, const name author_user_id, extended_symbol ext_sym, const name type, const name permissions );

    [[eosio::action]]
    void statelog( const name bounty_id, const name status, const name action );

    [[eosio::action]]
    void claimlog( const name bounty_id, const name chain, const string receiver, const extended_asset ext_quantity, const asset fee, const name status, const name worker_user_id, const uint32_t days_since_created );

    [[eosio::action]]
    void withdrawlog( const name bounty_id, const name chain, const string receiver, const extended_asset refund, const name status, const name author_user_id );

    // DEBUG (used to help testing)
    #ifdef DEBUG
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );
    #endif

    // action wrappers
    using setconfig_action = eosio::action_wrapper<"setconfig"_n, &pomelo::setconfig>;
    using token_action = eosio::action_wrapper<"token"_n, &pomelo::token>;
    using deltoken_action = eosio::action_wrapper<"deltoken"_n, &pomelo::deltoken>;
    using create_action = eosio::action_wrapper<"create"_n, &pomelo::create>;
    using setstate_action = eosio::action_wrapper<"setstate"_n, &pomelo::setstate>;
    using setmetadata_action = eosio::action_wrapper<"setmetadata"_n, &pomelo::setmetadata>;
    using approve_action = eosio::action_wrapper<"approve"_n, &pomelo::approve>;
    using forfeit_action = eosio::action_wrapper<"forfeit"_n, &pomelo::forfeit>;
    using close_action = eosio::action_wrapper<"close"_n, &pomelo::close>;
    using publish_action = eosio::action_wrapper<"publish"_n, &pomelo::publish>;
    using release_action = eosio::action_wrapper<"release"_n, &pomelo::release>;
    using deny_action = eosio::action_wrapper<"deny"_n, &pomelo::deny>;
    using withdraw_action = eosio::action_wrapper<"withdraw"_n, &pomelo::withdraw>;
    using apply_action = eosio::action_wrapper<"apply"_n, &pomelo::apply>;
    using complete_action = eosio::action_wrapper<"complete"_n, &pomelo::complete>;
    using claim_action = eosio::action_wrapper<"claim"_n, &pomelo::claim>;

    using depositlog_action = eosio::action_wrapper<"depositlog"_n, &pomelo::depositlog>;
    using createlog_action = eosio::action_wrapper<"createlog"_n, &pomelo::createlog>;
    using statelog_action = eosio::action_wrapper<"statelog"_n, &pomelo::statelog>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &pomelo::claimlog>;
    using withdrawlog_action = eosio::action_wrapper<"withdrawlog"_n, &pomelo::withdrawlog>;

private:
    // getters
    extended_asset calculate_fee( const extended_asset ext_quantity );
    pomelo::configs_row get_configs();
    pomelo::tokens_row get_token( const extended_symbol ext_sym );
    pomelo::tokens_row get_token( const extended_asset ext_quantity );
    pomelo::tokens_row get_token( const symbol_code symcode );
    bool is_token_enabled( const symbol_code symcode );
    name get_user_id( const name account );
    bool is_user( const name user_id );
    checksum256 get_trx_id();
    pair<name, name> parse_memo(const string& memo);
    void validate_bounty(const bounties_row& bounty);

    // notifiers
    void deposit_bounty( const name bounty_id, const name user_id, const name from, const extended_asset ext_quantity, const string memo );

    // utils
    void transfer( const name from, const name to, const extended_asset value, const string memo );

    // bridge
    void handle_bridge_transfer( const name chain, const string receiver, const extended_asset value, const string memo, const string default_memo );

    // DEBUG (used to help testing)
    #ifdef DEBUG
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
    #endif
};
