#pragma once

#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include <optional>
#include <set>

using namespace eosio;
using namespace std;

namespace eosn {

class [[eosio::contract("login.eosn")]] login : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `config`
     *
     * - `{vector<name>} notifiers` - accounts to be notified via inline action
     *
     * ### example
     *
     * ```json
     * {
     *   "notifiers": ["mynotify"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        vector<name>        notifiers = {};
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `users`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `bystatus`     | 2                | i64        |
     * | `byupdated`    | 3                | i64        |
     *
     * ### params
     *
     * - `{name} user_id` - (primary key) user ID
     * - `{set<public_key>} public_keys` - public key for account creation permission
     * - `{set<name>} [accounts=[]]` - EOS account names
     * - `{set<name>} [socials=[]]` - social accounts enabled
     * - `{name} [status="pending"]` - user status (ex: `created/ok/deleted`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *     "user_id": "123.eosn",
     *     "public_keys": ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"],
     *     "accounts": ["myaccount"],
     *     "socials": ["github"],
     *     "status": "ok",
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("users")]] users_row {
        name                user_id;
        set<public_key>     public_keys;
        set<name>           accounts;
        set<name>           socials;
        name                status;
        time_point_sec      created_at;
        time_point_sec      updated_at;

        uint64_t primary_key() const { return user_id.value; }
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "users"_n, users_row,
        indexed_by< "bystatus"_n, const_mem_fun<users_row, uint64_t, &users_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<users_row, uint64_t, &users_row::byupdated> >
    > users_table;

    /**
     * ## TABLE `accounts`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `byuser`       | 2                | i64        |
     *
     * ### params
     *
     * - `{name} account` - (primary key) EOS account
     * - `{name} user_id` - user ID
     *
     * ### example
     *
     * ```json
     * {
     *     "account": "myaccount",
     *     "user_id": "123.eosn"
     * }
     * ```
     */
    struct [[eosio::table("accounts")]] accounts_row {
        name                account;
        name                user_id;

        uint64_t primary_key() const { return account.value; }
        uint64_t byuser() const { return user_id.value; };
    };
    typedef eosio::multi_index< "accounts"_n, accounts_row,
        indexed_by< "byuser"_n, const_mem_fun<accounts_row, uint64_t, &accounts_row::byuser> >
    > accounts_table;

    /**
     * ## TABLE `socials`
     *
     * scope: get_self()
     *
     * ### params
     *
     * - `{name} social` - (primary key) social
     * - `{uint32_t} weight` - (pips 1 = 0.01% ) social weight (0-20000), i.e. 5000 gives 50% boost
     *
     * ### example
     *
     * ```json
     * {
     *     "social": "github",
     *     "weight": 5000
     * }
     * ```
     */
    struct [[eosio::table("socials")]] socials_row {
        name                social;
        uint32_t            weight;

        uint64_t primary_key() const { return social.value; }
    };
    typedef eosio::multi_index< "socials"_n, socials_row> socials_table;

    /**
     * ## TABLE `proofs`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `bynonce`      | 2                | i64        |
     * | `bycreated`    | 3                | i64        |
     *
     * ### params
     *
     * - `{name} account` - (primary key) account name
     * - `{uint64_t} nonce` - nonce number
     * - `{string} data` - (optional) string data
     * - `{time_point_sec} created_at` - created at time
     *
     * ### example
     *
     * ```json
     * {
     *     "account": "myaccount",
     *     "nonce": 123,
     *     "data": "any data",
     *     "created_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table("proofs")]] proofs_row {
        name                account;
        uint64_t            nonce;
        string              data;
        time_point_sec      created_at;

        uint64_t primary_key() const { return account.value; }
        uint64_t bynonce() const { return nonce; };
        uint64_t bycreated() const { return created_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "proofs"_n, proofs_row,
        indexed_by< "bynonce"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::bynonce> >,
        indexed_by< "bycreated"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::bycreated> >
    > proofs_table;

    /**
     * ## ACTION `create`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{set<public_key>} public_keys` - public keys for account creation permission
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn create '["123.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void create( const name user_id, const set<public_key> public_keys );

    /**
     * ## ACTION `status`
     *
     * - **authority**: `user_id`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} status` - user status (ex: `created/ok/deleted`)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn status '["123.eosn", "ok"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void status( const name user_id, const name status );

    /**
     * ## ACTION `deluser`
     *
     * - **authority**: `user_id`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn deluser '["123.eosn"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void deluser( const name user_id );

    /**
     * ## ACTION `link`
     *
     * > Link user with EOS account
     *
     * - **authority**: `account` AND `sig` **two signatures required**
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} account` - account name
     * - `{signature} sig` - EOSIO signature from `user_id` (backend request)
     *
     * ### Example
     *
     * ```
     * $ cleos push action login.eosn link '["123.eosn", "myaccount", "SIG_K1_KB3uRPsg3VvngDjEsLiH9HLFqqkVGMy11tLW2BePCZEfXKRhcogc536EDj8Fib4igBfGpWDa2PjfnD9DxfHpwMBySrZuLY"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void link( const name user_id, const name account, const signature sig );

    /**
     * ## ACTION `unlink`
     *
     * > Unlink user of EOS account
     *
     * - **authority**: `user_id` OR `account`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{optional<name>} [account=""]` - (optional) account name (if null is provided, erease all)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn unlink '["123.eosn", "myaccount"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void unlink( const name user_id, const optional<name> account );

    /**
     * ## ACTION `social`
     *
     * > Enable user social logins
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} social` - enable social account
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn social '["123.eosn", "github"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void social( const name user_id, const name social );

    /**
     * ## ACTION `setsocial`
     *
     * > Enable user social logins
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{set<name>} socials` - enable social accounts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn setsocial '["123.eosn", ["github"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void setsocial( const name user_id, const set<name> socials );

    /**
     * ## ACTION `unsocial`
     *
     * > Disable user social logins
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{optional<name>} [social=""]` - (optional) disable social account (if null, disable all socials)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn unsocial '["123.eosn", "github"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void unsocial( const name user_id, const optional<name> social );

    /**
     * ## ACTION `configsocial`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} social` - social
     * - `{uint32_t} weight` - social weight
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn configsocial '["github", 50]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void configsocial( const name social, const uint32_t weight );

    /**
     * ## ACTION `proof`
     *
     * - **authority**: `account`
     *
     * ### params
     *
     * - `{name} account` - account name
     * - `{uint64_t} nonce` - proof nonce
     * - `{string} [data=""]` - (optional) string data
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn proof '["myaccount", 123, "any data"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void proof( const name account, const uint64_t nonce, const optional<string> data );

    [[eosio::action]]
    void reset( const name table );

    /**
     * ## ACTION `setnotifiers`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{vector<name>} notifiers` - contracts to be notified on event
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn setnotifiers '[["app.pomelo"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void setnotifiers( const vector<name> notifiers );

    /**
     * ## STATIC `is_auth`
     *
     * Returns true/false if user ID is authorized
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} [login_contract="login.eosn"]` - (optional) EOSN login contract
     *
     * ### returns
     *
     * - `{bool}` - [true/false] if user is authorized
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * const bool is_auth  = eosn::login::is_auth( user_id );
     * //=> true
     * ```
     */
    static bool is_auth( const name user_id, const name login_contract )
    {
        login::users_table _users( login_contract, login_contract.value );
        auto users = _users.get( user_id.value, "login::is_auth: [user_id] does not exist");

        if ( has_auth( user_id ) ) return true;

        for ( const name account : users.accounts ) {
            if ( has_auth(account) ) return true;
        }
        return false;
    }

    /**
     * ## STATIC `is_user_id_exists`
     *
     * Returns true/false if user ID exists
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} [login_contract="login.eosn"]` - (optional) EOSN login contract
     *
     * ### returns
     *
     * - `{bool}` - [true/false] if user ID exists
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * const bool exists = eosn::login::is_user_id_exists( user_id );
     * //=> true
     * ```
     */
    static bool is_user_id_exists( const name user_id, const name login_contract )
    {
        login::users_table _users( login_contract, login_contract.value );
        auto users = _users.find( user_id.value );
        return users != _users.end();
    }

    /**
     * ## STATIC `require_auth_user_id`
     *
     * Asserts error if user ID is authorized or not
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} [login_contract="login.eosn"]` - (optional) EOSN login contract
     *
     * ### returns
     *
     * - `{void}` - throw error if user ID is authorized or not
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * eosn::login::require_auth_user_id( user_id );
     * ```
     */
    static void require_auth_user_id( const name user_id, const name login_contract )
    {
        check( is_auth( user_id, login_contract ), "login::require_auth_user_id: [user_id] is not authorized" );
    }

    /**
     * ## STATIC `get_user_weight`
     *
     * Calculate user weight based on socials (sum of all weights)
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} [login_contract="login.eosn"]` - (optional) EOSN login contract
     *
     * ### returns
     *
     * - `{uint32_t}` - resulting weight
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * const auto weight = eosn::login::get_user_weight( user_id );
     * //=> 150
     * ```
     */
    static uint32_t get_user_weight( const name user_id, const name login_contract )
    {
        login::users_table _users( login_contract, login_contract.value );
        login::socials_table _socials( login_contract, login_contract.value );

        auto user = _users.get( user_id.value, "login::get_user_weight: [user_id] does not exist" );
        check( user.status != "deleted"_n, "login::get_user_weight: user is deleted" );
        uint32_t total_weight = 0;
        for( const auto& social: user.socials ){
            total_weight += _socials.get( social.value, "login::get_user_weight: [user_id] has unknown social").weight;
        }
        return total_weight;
    }

    using create_action = eosio::action_wrapper<"create"_n, &eosn::login::create>;
    using status_action = eosio::action_wrapper<"status"_n, &eosn::login::status>;
    using deluser_action = eosio::action_wrapper<"deluser"_n, &eosn::login::deluser>;
    using link_action = eosio::action_wrapper<"link"_n, &eosn::login::link>;
    using unlink_action = eosio::action_wrapper<"unlink"_n, &eosn::login::unlink>;
    using social_action = eosio::action_wrapper<"social"_n, &eosn::login::social>;

private:
    void unlink_by_user( const name user_id );
    void unlink_by_account( const name account );
    void unlink_user( const name user_id, const name account );

    template <typename T>
    bool erase_table( T& table );
    void erase_stale_proofs();
    void alert_notifiers();
    void create_account( const name user_id, const set<public_key> public_keys );
    void verify_sig( const name user_id, const name account, const signature& sig );
};

} // namespace eosn