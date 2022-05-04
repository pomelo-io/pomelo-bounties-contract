#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

// static values

static constexpr extended_symbol VALUE_SYM = { symbol {"USDT", 4}, "tethertether"_n };
static set<name> STATUS_TYPES = set<name>{"published"_n, "pending"_n, "retired"_n, "banned"_n, "denied"_n};
static constexpr uint32_t DAY = 86400;

static string ERROR_INVALID_MEMO = "invalid transfer memo (ex: \"eosio.grants:123\")";

namespace pomelo {

class [[eosio::contract("work.pomelo")]] work : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `status`
     *
     * - `{vector<uint32_t>} counters` - counters
     *   - `{uint32_t} counters[0]` - total created
     *   - `{uint32_t} counters[1]` - total approved
     *   - `{uint32_t} counters[2]` - total denied
     *   - `{uint32_t} counters[3]` - total completed
     *   - `{uint32_t} counters[3]` - total claimed
     * - `time_point_sec` last_updated;
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
     * ## TABLE `globals`
     *
     * - `{uint64_t} fee` - platform fee (bips - 1/100 1%)
     * - `{name} login_contract` - EOSN Login contract
     * - `{name} fee_account` - fee
     *
     * ### example
     *
     * ```json
     * {
     *     "fee": 500,
     *     "login_contract": "login.eosn",
     *     "fee_account": "fee.pomelo",
     * }
     * ```
     */
    struct [[eosio::table("globals")]] globals_row {
        uint64_t        fee = 500;
        name            login_contract = "login.eosn"_n;
        name            fee_account = "fee.pomelo"_n;
    };
    typedef eosio::singleton< "globals"_n, globals_row > globals_table;

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
     * *scope*: `{name} author_user_id`
     *
     * ## params
     *
     * - `{name} id` - (primary key) bounty ID
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{extended_asset} bounty - funds to be released once bounty is completed
     * - `{string} url - bounty URL information
     * - `{name} status = "pending" - status (`pending/published/banned/retired/denied`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *     "id": 123,
     *     "author_user_id": "123.eosn",
     *     "bounty": {"quantity": "10.0000 USDT", "contract": "tethertether"},
     *     "status": "published",
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table]] bounties_row {
        uint64_t                id;
        name                    type;
        name                    author_user_id;
        name                    funding_account;
        set<symbol_code>        accepted_tokens = { symbol_code{"EOS"} };
        name                    status = "pending"_n;
        time_point_sec          created_at;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return id; }
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
     * - `{name} from` - EOS account sender
     * - `{name} to` - EOS account receiver
     * - `{extended_asset} ext_quantity` - amount of tokens transfered
     * - `{asset} fee` - fee charged and sent to `global.fee_account`
     * - `{string} memo` - transfer memo
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{name} bounty_id` - bounty ID
     * - `{double} value` - valuation at time of received
     * - `{checksum256} trx_id` - transaction ID
     * - `{time_point_sec} created_at` - created at time
     *
     * ### example
     *
     * ```json
     * {
     *     "transfer_id": 10001,
     *     "from": "myaccount",
     *     "to": "work.pomelo",
     *     "ext_quantity": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
     *     "fee": "1.0000 EOS",
     *     "memo": "grant:grant1",
     *     "author_user_id": "author.eosn",
     *     "bounty_id": 123,
     *     "value": 100.0,
     *     "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
     *     "created_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table]] transfers_row {
        uint64_t                transfer_id;
        name                    from;
        name                    to;
        extended_asset          ext_quantity;
        asset                   fee;
        string                  memo;
        name                    author_user_id;
        uint64_t                bounty_id;
        double                  value;
        checksum256             trx_id;
        time_point_sec          created_at;

        uint64_t primary_key() const { return transfer_id; };
    };
    typedef eosio::multi_index< "transfers"_n, transfers_row> transfers_table;


    /**
     * ## ACTION `setconfig`
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
     * $ cleos push action work.pomelo setconfig '[500, "login.eosn", "fee.pomelo"]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const optional<uint64_t> fee, const optional<name> login_contract, const optional<name> fee_account );

    /**
     * ## ACTION `createbounty`
     *
     * Create/update grant - wrapper for setproject for grants
     *
     * ### params
     *
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{symbol_code} accepted_token` - accepted deposit token (ex: `"EOS"`)
     * - `{string} url` - bounty URL information (ex: Github issue url)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action work.pomelo createbounty '["author.eosn", "USDT", "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void createbounty( const name author_user_id, const symbol_code accepted_token, const string url );

    /**
     * ## ACTION `setstate`
     *
     * Set grant or bounty state
     *
     * ### params
     *
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{name} bounty_id` - bounty ID
     * - `{name} status` - status `pending/published/banned/retired/denied'
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo setstate '["author.eosn", 123, "published"]' -p work.pomelo
     * ```
     */
    [[eosio::action]]
    void setstate( const name author_user_id, const name bounty_id, const name state );

    /**
     * ## ACTION `approve`
     *
     * Author select hunter for bounty
     *
     * ### params
     *
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{name} bounty_id` - bounty ID
     * - `{name} user_id - applicant account "hunter" (EOSN Login ID)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo approve '[author.eosn, 123, hunter.eosn]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void approve( const name author_user_id, const name bounty_id, const name user_id );

    /**
     * ## ACTION `release`
     *
     * Author release funds for completed bounty
     *
     * ### params
     *
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo release '[author.eosn, 123]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void release( const name author_user_id, const name bounty_id );

    /**
     * ## ACTION `deny`
     *
     * Author denies completed bounty
     *
     * Hunter has 72 hours to respond to deny request
     *
     # Bounty is set to un-completed state after 72 hours of no response (funds can be withdrawn by author).
     *
     * ### params
     *
     * - `{name} author_user_id - author (EOSN Login ID)
     * - `{name} bounty_id` - bounty ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action work.pomelo deny '[author.eosn, 123]' -p author.eosn
     * ```
     */
    [[eosio::action]]
    void deny( const name author_user_id, const name bounty_id );

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
     *
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope );

    /**
     * ## ACTION `token`
     *
     * Set token information
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

    [[eosio::action]]
    void deltoken( const symbol_code symcode );

private:
    void transfer( const name from, const name to, const extended_asset value, const string memo );

    // // getters
    // double calculate_value(const extended_asset ext_quantity );
    // name get_user_id( const name user );
    // bool is_user( const name user_id );
    // void validate_round( const uint16_t round_id );
    // uint16_t get_active_round( const name grant_id );
    // extended_asset calculate_fee( const extended_asset ext_quantity );

    // // tokens
    // tokens_row get_token( const extended_symbol ext_sym );
    // tokens_row get_token( const extended_asset ext_quantity );
    // bool is_token_enabled( const symbol_code symcode );

    // // globals key/value
    // // void set_key_value( const name key, const uint64_t value );
    // // uint64_t get_key_value( const name key );
    // // bool del_key( const name key );

    // globals_row get_globals();

    // template <typename T>
    // void donate_project(const T& table, const name project_id, const name from, const extended_asset ext_quantity, const string memo );

    // void donate_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value);

    // template <typename T>
    // void set_project(T& table, const name project_type, const name project_id, const name author_id, const name funding_account, const set<symbol_code> accepted_tokens );

    // void save_transfer( const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const name project_type, const name project_id, const double value );

    // int get_index(const vector<name>& vec, name value);
    // int get_index(const vector<contribution_t>& vec, name id);
    // int get_index(const vector<uint16_t>& vec, uint16_t id);

    // template <typename T>
    // vector<T> remove_element(const vector<T>& vec, name id);

    // template <typename T>
    // void clear_table( T& table, uint64_t rows_to_clear );

    // // update counters in status singleton
    // void update_status( const uint32_t index, const uint32_t count );

    // void update_social( const name user_id );
};

};