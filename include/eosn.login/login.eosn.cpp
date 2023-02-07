#include <eosio/native.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio/crypto.hpp>
#include <pomelo.play/play.pomelo.hpp>
#include <signature/signature.hpp>

#include "login.eosn.hpp"

namespace eosn {

[[eosio::action]]
void login::create( const name user_id, const set<public_key> public_keys )
{
    require_auth( get_self() );
    alert_notifiers();

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    const name suffix = user_id.suffix();
    auto itr = _users.find( user_id.value );
    check( itr == _users.end(), "login::create: [user_id=" + user_id.to_string() + "] already exists" );
    check( public_keys.size(), "login::create: [public_keys] is empty" );
    check( suffix.value, "login::create: [user_id=" + user_id.to_string() + "] does not include a suffix" );
    check( suffix == "eosn"_n, "login::create: [user_id=" + user_id.to_string() + "] suffix must be *.eosn" );

    // create user row
    _users.emplace( get_self(), [&]( auto & row ) {
        row.user_id = user_id;
        row.public_keys = public_keys;
        row.status = "created"_n;
        row.created_at = current_time_point();
        row.updated_at = current_time_point();
    });

    // if on-chain account is not yet created
    if ( !is_account( user_id ) ) create_account( user_id, public_keys );
}

void login::create_account( const name user_id, const set<public_key> public_keys )
{
    const name suffix = user_id.suffix();

    // setup account permission
    vector<eosiosystem::key_weight> keys;
    for ( const public_key key : public_keys ) {
        keys.push_back({ key, 1 });
    }

    const vector<eosiosystem::permission_level_weight> accounts = {{ { get_self(), "eosio.code"_n }, 1 }};
    const eosiosystem::authority owner = { 1, {}, accounts, {} };
    const eosiosystem::authority active = { 1, keys, {}, {} };

    eosiosystem::native::newaccount_action newaccount( "eosio"_n, { suffix, "active"_n} );
    newaccount.send( suffix, user_id, owner, active );

    // purchase 1,735 bytes worth of RAM (no excess RAM)
    eosiosystem::system_contract::buyrambytes_action buyrambytes( "eosio"_n, { get_self(), "active"_n });
    buyrambytes.send( get_self(), user_id, 1735 );

    // open EOS token balance
    eosio::token::open_action open( "eosio.token"_n, { get_self(), "active"_n });
    open.send( get_self(), symbol{"EOS", 4}, get_self() );
}

[[eosio::action]]
void login::status( const name user_id, const name status )
{
    require_auth( user_id );
    alert_notifiers();

    // validate user ID
    login::users_table _users( get_self(), get_self().value );
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::status: [user_id=" + user_id.to_string() + "] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        check( row.status != status, "login::status: was not modified" );
        row.status = status;
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::deluser( const name user_id )
{
    require_auth( user_id );
    alert_notifiers();

    // validate user ID
    login::users_table _users( get_self(), get_self().value );
    auto itr = _users.find( user_id.value );
    _users.erase( itr );
    unlink_by_user( user_id );
}

void login::verify_sig(const name user_id, const name account, const signature& sig )
{
    login::users_table _users( get_self(), get_self().value );
    auto user = _users.get( user_id.value, "login::verify_sig: [user_id] does not exist" );

    checksum256 digest = sha256((const char*) &account.value, sizeof(account.value));
    const auto pk = recover_key( digest, sig);
    check( user.public_keys.count(pk), "login::verify_sig: invalid signature");
}

[[eosio::action]]
void login::link( const name user_id, const name account, const signature sig)
{
    require_auth( account );
    verify_sig( user_id, account, sig );
    alert_notifiers();

    // unlink account related to user id
    login::accounts_table _accounts( get_self(), get_self().value );
    auto accounts_itr = _accounts.find( account.value );
    if( accounts_itr != _accounts.end()){
        unlink_user( accounts_itr->user_id, account );
    }
    unlink_by_account( account );
    unlink_by_user( user_id );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::link: [user_id=" + user_id.to_string() + "] does not exist" );
    check( is_account( account ), "login::link: [account=" + account.to_string() + "] account does not exist" );
    check( user_id != account, "login::link: [user_id=" + user_id.to_string() + "] cannot be the same as [account=" + account.to_string() + "]" );
    check( itr->accounts.count( account ) == 0, "login::link: [user_id=" + user_id.to_string() + "] is already linked to [account=" + account.to_string() + "]" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.accounts.clear();
        row.accounts.insert( account );
        row.updated_at = current_time_point();

        // account can only have one linked account
        check( row.accounts.size() <= 1, "login::link: [user_id=" + user_id.to_string() + "] can only have one linked account" );
    });

    // re-instantiate accounts table
    login::accounts_table _accounts1( get_self(), get_self().value );
    _accounts1.emplace( get_self(), [&]( auto & row ) {
        row.account = account;
        row.user_id = user_id;
    });

    if (get_self() == "d.login.eosn"_n && is_account("play.pomelo"_n)) {
        const auto play_balance = pomelo::playtoken::get_balance("play.pomelo"_n, account, symbol_code("PLAY"));
        if ( !play_balance.symbol.is_valid() ) {  // if already issued before - quietly don't issue
            pomelo::playtoken::faucet_action faucet( "play.pomelo"_n, { "play.pomelo"_n, "active"_n });
            faucet.send( account, symbol_code("PLAY") );
        }
    }
}

[[eosio::action]]
void login::unlink( const name user_id, const optional<name> account )
{
    const bool is_authed = has_auth( get_self() ) || is_auth( user_id, get_self() ) || has_auth( *account);
    check( is_authed, "login::unlink: is not authorized");
    alert_notifiers();

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::unlink: [user_id=" + user_id.to_string() + "] does not exist" );
    check( itr->accounts.size(), "login::unlink: [user_id=" + user_id.to_string() + "] has no linked accounts" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        if ( account ) {
            check( row.accounts.count( *account ), "login::unlink: [account=" + account->to_string() + "] is already unlinked" );
            row.accounts.erase( *account );
            unlink_by_account( *account );
        } else row.accounts = {};

        // remove any accounts linked to user id
        if ( row.accounts.size() == 0 ) unlink_by_user( user_id );
        row.updated_at = current_time_point();
    });
}

void login::unlink_by_account( const name account )
{
    login::accounts_table _accounts( get_self(), get_self().value );

    auto itr = _accounts.find( account.value );
    if ( itr != _accounts.end() ) _accounts.erase( itr );
}

void login::unlink_by_user( const name user_id )
{
    login::accounts_table _accounts( get_self(), get_self().value );

    auto index = _accounts.get_index<"byuser"_n>();
    auto itr = index.find( user_id.value );
    if ( itr != index.end() ) index.erase( itr );
}

void login::unlink_user( const name user_id, const name account )
{
    login::users_table _users( get_self(), get_self().value );

    auto itr = _users.find( user_id.value );
    if ( itr != _users.end() ) {
        _users.modify( itr, get_self(), [&]( auto & row ) {
            row.accounts.erase( account );
            row.updated_at = current_time_point();
        });
    }
}

[[eosio::action]]
void login::social( const name user_id, const name social )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );
    login::socials_table _socials( get_self(), get_self().value );

    // notify contracts
    alert_notifiers();

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::social: [user_id=" + user_id.to_string() + "] does not exist" );
    check( _socials.find( social.value ) != _socials.end(), "login::social: [social=" + social.to_string() + "] is unknown" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        check( !row.socials.count( social ), "login::social: [social=" + social.to_string() + "] is already exists" );
        row.socials.insert( social );
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::setsocial( const name user_id, const set<name> socials )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );
    login::socials_table _socials( get_self(), get_self().value );

    // notify contracts
    alert_notifiers();

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::social: [user_id=" + user_id.to_string() + "] does not exist" );
    for ( const name social : socials ) {
        check( _socials.find( social.value ) != _socials.end(), "login::social: [social=" + social.to_string() + "] is unknown" );
    }

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        check( row.socials != socials, "login::setsocial: not modified" );
        row.socials = socials;
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::unsocial( const name user_id, const optional<name> social )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );
    login::socials_table _socials( get_self(), get_self().value );

    // notify contracts
    alert_notifiers();

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::unsocial: [user_id] does not exist" );
    check( _socials.find( social->value ) != _socials.end(), "login::unsocial: [social=" + social->to_string() + "] is unknown" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        if ( social ) {
            check( row.socials.count( *social ), "login::unsocial: [social=" + social->to_string() + "] is already removed" );
            row.socials.erase( *social );
        } else row.socials = {};
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::proof( const name account, const uint64_t nonce, const optional<string> data )
{
    require_auth( account );

    erase_stale_proofs();
    login::proofs_table _proofs( get_self(), get_self().value );

    auto insert = [&]( auto & row ) {
        row.account = account;
        row.nonce = nonce;
        row.data = *data;
        row.created_at = current_time_point();
    };

    // create/modify row
    auto itr = _proofs.find( account.value );
    if ( itr == _proofs.end() ) _proofs.emplace( get_self(), insert );
    else _proofs.modify( itr, get_self(), insert );
}

[[eosio::action]]
void login::setnotifiers( const vector<name> notifiers )
{
    require_auth( get_self() );

    for ( const name notifier : notifiers ) {
        check( is_account( notifier ), "login::setnotifiers: [notifier=" + notifier.to_string() + "] does not exist");
    }
    login::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    config.notifiers = notifiers;
    _config.set( config, get_self() );
}

void login::alert_notifiers()
{
    login::config_table _config( get_self(), get_self().value );

    for ( const name notifier : _config.get_or_default().notifiers ) {
        require_recipient( notifier );
    }
}

void login::erase_stale_proofs()
{
    // delete last 10 rows after 24 hours
    login::proofs_table _proofs( get_self(), get_self().value );
    auto index = _proofs.get_index<"bycreated"_n>();
    auto itr = index.begin();
    vector<name> to_erase;
    while ( true ) {
        const uint64_t time_delta = current_time_point().sec_since_epoch() - itr->created_at.sec_since_epoch();
        if ( itr == index.end() ) break;
        if ( time_delta <= 86400 ) break;
        if ( to_erase.size() >= 10 ) break;
        print(itr->account.to_string() + ":" + to_string(time_delta) + "\n");
        to_erase.push_back( itr->account );
        itr++;
    }
    for ( const name account : to_erase ) {
        _proofs.erase(_proofs.find( account.value ));
    }
}

[[eosio::action]]
void login::reset( const name table )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );
    login::proofs_table _proofs( get_self(), get_self().value );
    login::accounts_table _accounts( get_self(), get_self().value );
    login::config_table _config( get_self(), get_self().value );

    if ( table == "users"_n ) erase_table( _users );
    else if ( table == "proofs"_n ) erase_table( _proofs );
    else if ( table == "accounts"_n ) erase_table( _accounts );
    else if ( table == "config"_n ) _config.remove();
    else check( false, "invalid table name");
}

[[eosio::action]]
void login::configsocial( const name social, const uint32_t weight )
{
    require_auth( get_self() );
    check( weight <= 200, "login::setsocial: [weight=" + to_string(weight) + "] should be <= 200");

    login::socials_table _socials( get_self(), get_self().value );

    auto insert = [&]( auto & row ) {
        row.social = social;
        row.weight = weight;
    };
    const auto itr = _socials.find( social.value );
    if ( itr == _socials.end() ) _socials.emplace( get_self(), insert );
    else {
        if ( weight == 0 ) _socials.erase( itr );
        else _socials.modify( itr, get_self(), insert );
    }
}

template <typename T>
bool login::erase_table( T& table )
{
    auto itr = table.begin();
    bool erased = false;
    while ( itr != table.end() ) {
        itr = table.erase( itr );
        erased = true;
    }
    return erased;
}

} // namespace eosn
