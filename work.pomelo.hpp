#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

// static values
static constexpr extended_symbol VALUE_SYM = { symbol {"USDT", 4}, "tethertether"_n };
// static set<name> STATUS_TYPES = set<name>{"published"_n, "pending"_n, "retired"_n, "banned"_n, "denied"_n};
static set<name> STATUS_TYPES = set<name>{"pending"_n, "open"_n, "started"_n, "submitted"_n, "done"_n};
static set<name> STATUS_DEPOSIT_TYPES = set<name>{"pending"_n, "open"_n, "started"_n};
static constexpr uint32_t DAY = 86400;

static string ERROR_INVALID_MEMO = "invalid transfer memo (ex: \"eosio.grants:123\")";

class [[eosio::contract("work.pomelo")]] pomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `status`
     *
     * ### params
     *
     * - `{vector<uint32_t>} counters` - counters
     *   - `{uint32_t} counters[0]` - total created
     *   - `{uint32_t} counters[1]` - total approved
     *   - `{uint32_t} counters[2]` - total denied
     *   - `{uint32_t} counters[3]` - total completed
     *   - `{uint32_t} counters[3]` - total claimed
     * - `{time_point_sec} last_updated`
     *
     * ### example
     *
     * ```json
     * {
     *     "counters": [1234, 12],
     *     "last_updated": "2021-04-12T12:23:42"
     * }
     * ```
     */
    struct [[eosio::table("status")]] status_row {
        vector<uint32_t>        counters;
        time_point_sec          last_updated;
    };
    typedef eosio::singleton< "status"_n, status_row > status_table;

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
     *     "fee": 500,
     *     "login_contract": "login.eosn",
     *     "fee_account": "fee.pomelo",
     *     "metadata_keys": ["url"]
     * }
     * ```
     */
    struct [[eosio::table("configs")]] configs_row {
        name            status = "testing"_n;
        uint64_t        fee = 500;
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
     * - `{uint64_t} min_amount` - min amount required when donating
     * - `{uint64_t} oracle_id` - Defibox Oracle ID
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,EOS",
     *     "contract": "eosio.token",
     *     "min_amount": 10000,
     *     "oracle_id": 1
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol              sym;
        name                contract;
        uint64_t            min_amount;
        uint64_t            oracle_id;

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
     * - `{name} funder_user_id` - funder (EOSN Login ID)
     * - `{extended_asset} amount` - amount of tokens to be released once bounty is completed
     * - `{set<name>} applicant_user_ids` - list of applicants that have applied to bounty
     * - `{set<name>} approved_user_id` - approved account for bounty
     * - `{name} status="pending"` - status (`pending/open/started/submitted/done`)
     * - `{name} type="traditional"` - bounty type (`traditional` = "1 worker at a time, 1 is paid out")
     * - `{name} permissions="approval"` - bounty permissions (`approval` = "Funder must approve hunter to start work")
     * - `{Metadata} metadata={}` - bounty metadata
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
     *     "funder_user_id": "funder.eosn",
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
        name                    funder_user_id;
        extended_asset          amount;
        asset                   claimed;
        set<name>               applicant_user_ids;
        name                    approved_user_id;
        name                    status = "pending"_n;
        name                    type = "traditional"_n;
        name                    permissions = "approval"_n;
        map<string, string>     metadata;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          submitted_at;
        time_point_sec          completed_at;

        uint64_t primary_key() const { return bounty_id.value; }
    };
    typedef eosio::multi_index< "bounties"_n, bounties_row> bounties_table;

    /**
     * ## TABLE `transfer`
     *
     * - **scope**: `{name} get_self()`
     *
     * ### params
     *
     * - `{uint64_t} transfer_id` - (primary key) token transfer ID
     * - `{name} bounty_id` - bounty ID
     * - `{name} from` - EOS account sender
     * - `{name} to` - EOS account receiver
     * - `{extended_asset} ext_quantity` - amount of tokens transfered
     * - `{asset} fee` - fee charged and sent to `global.fee_account`
     * - `{string} memo` - transfer memo
     * - `{double} value` - valuation at time of received
     * - `{checksum256} trx_id` - transaction ID
     * - `{time_point_sec} created_at` - created at time
     *
     * ### example
     *
     * ```json
     * {
     *     "transfer_id": 10001,
     *     "bounty_id": 123,
     *     "from": "myaccount",
     *     "to": "work.pomelo",
     *     "ext_quantity": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
     *     "fee": "1.0000 EOS",
     *     "memo": "grant:grant1",
     *     "value": 100.0,
     *     "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
     *     "created_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table]] transfers_row {
        uint64_t                transfer_id;
        name                    bounty_id;
        name                    from;
        name                    to;
        extended_asset          ext_quantity;
        asset                   fee;
        string                  memo;
        double                  value;
        checksum256             trx_id;
        time_point_sec          created_at;

        uint64_t primary_key() const { return transfer_id; };
    };
    typedef eosio::multi_index< "transfers"_n, transfers_row> transfers_table;

    /**
     * ## ACTION `setconfig`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint64_t} fee` - platform fee (bips - 1/100 1%)
     * - `{name} login_contract` - EOSN Login contract
     * - `{name} fee_account` - fee account
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo setconfig '[1000, "login.eosn", "fee.pomelo"]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const optional<uint64_t> fee, const optional<name> login_contract, const optional<name> fee_account );

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
     * - `{uint64_t} min_amount` - min amount required when donating
     * - `{uint64_t} oracle_id` - Defibox oracle ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t oracle_id );

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
     * ## ACTION `createbounty`
     *
     * - **authority**: `funder_user_id`
     *
     * Create bounty
     *
     * ### params
     *
     * - `{name} funder_user_id` - funder (EOSN Login ID)
     * - `{name} bount_id` - bounty ID
     * - `{symbol_code} accepted_token` - accepted deposit token (ex: `"EOS"`)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action work.pomelo createbounty '[funder.eosn, bounty1, "USDT"]' -p funder.eosn
     * ```
     */
    [[eosio::action]]
    void createbounty( const name funder_user_id, const name bounty_id, const symbol_code accepted_token );

    /**
     * ## ACTION `setstate`
     *
     * - **authority**: `get_self()`
     *
     * Set grant or bounty state
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
     * ## ACTION `approve`
     *
     * - **authority**: `funder_user_id`
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
     * $ cleos push action work.pomelo approve '[bounty1, hunter.eosn]' -p funder.eosn
     * ```
     */
    [[eosio::action]]
    void approve( const name bounty_id, const name applicant_user_id );

    /**
     * ## ACTION `release`
     *
     * - **authority**: `funder_user_id`
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
     * $ cleos push action work.pomelo release '[bounty1]' -p funder.eosn
     * ```
     */
    [[eosio::action]]
    void release( const name bounty_id );

    /**
     * ## ACTION `deny`
     *
     * - **authority**: `funder_user_id`
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
     * $ cleos push action work.pomelo deny '[bounty1]' -p funder.eosn
     * ```
     */
    [[eosio::action]]
    void deny( const name bounty_id );

    /**
     * ## ACTION `withdraw`
     *
     * - **authority**: `account` (EOS account linked to EOSN Login's bounty author)
     *
     * Author withdraws funds from bounty in "pending" state (EOS account linked with EOSN Login)
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} receiver` - receiver account (must be linked to EOSN Login funder account)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo withdraw '[bounty1, myaccount]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void withdraw( const name bounty_id, const name receiver );

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
     * - **authority**: `account` (EOS account linked with EOSN Login)
     *
     * Hunter claims bounty funds
     *
     * ### params
     *
     * - `{name} bounty_id` - bounty ID
     * - `{name} receiver` - receiver account (must be linked to EOSN Login approved applicant)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo claim '[bounty1]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void claim( const name bounty_id, const name receiver );

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
     * - `{string} memo` - transfer memo, i.e. "autho.eosn:123"
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

private:
    // getters
    extended_asset calculate_fee( const extended_asset ext_quantity );
    pomelo::configs_row get_configs();
    pomelo::tokens_row get_token( const extended_symbol ext_sym );
    pomelo::tokens_row get_token( const extended_asset ext_quantity );
    pomelo::tokens_row get_token( const symbol_code symcode );
    bool is_token_enabled( const symbol_code symcode );
    double calculate_value( const extended_asset ext_quantity );
    name get_user_id( const name account );
    bool is_user( const name user_id );

    // notifiers
    void deposit_bounty( const name bounty_id, const name from, const extended_asset ext_quantity, const string memo );
    void save_transfer( const name bounty_id, const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const double value );

    // utils
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
    void transfer( const name from, const name to, const extended_asset value, const string memo );
    checksum256 get_trx_id();
    name parse_name(const string& str);
    void update_status( const uint32_t index, const uint32_t count ); // update counters in status singleton
};
