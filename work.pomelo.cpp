#include <eosn.login/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <oracle.defi/oracle.defi.hpp>
#include <utils/utils.hpp>

#include "work.pomelo.hpp"

#include "src/getters.cpp"
#include "src/notifiers.cpp"

// logging (used for syncing with backend)
#include "src/logs.cpp"

// DEBUG (used to help testing)
#ifdef DEBUG
#include "src/debug.cpp"
#endif

// @admin
[[eosio::action]]
void pomelo::token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t oracle_id )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );

    const asset supply = token::get_supply( contract, sym.code() );
    check( supply.symbol == sym, "pomelo::token: [sym] symbol does not match with token supply");
    check( supply.amount, "pomelo::token: [sym] has no supply");

    // check if Oracle exists; if not it will assert fail
    if ( is_account( oracle_code ) && extended_symbol{ sym, contract } != VALUE_SYM )
        defi::oracle::get_value( {10000, extended_symbol{ sym, contract }}, oracle_id );

    const auto insert = [&]( auto & row ) {
        row.sym = sym;
        row.contract = contract;
        row.min_amount = min_amount;
        row.oracle_id = oracle_id;
    };

    const auto itr = tokens.find( sym.code().raw() );
    if ( itr == tokens.end() ) tokens.emplace( get_self(), insert );
    else tokens.modify( itr, get_self(), insert );
}

// @admin
[[eosio::action]]
void pomelo::deltoken( const symbol_code symcode )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );
    const auto & itr = tokens.get( symcode.raw(), "pomelo::deltoken: [symcode] token not found" );
    tokens.erase( itr );
}

// @user
[[eosio::action]]
void pomelo::create( const name author_user_id, const name bounty_id, const symbol_code accepted_token, const optional<name> bounty_type )
{
    eosn::login::require_auth_user_id( author_user_id, get_configs().login_contract );

    const auto type = bounty_type ? *bounty_type : "traditional"_n;
    check( BOUNTY_TYPES.count(type), "pomelo::create: unknown [bounty_type]" );

    // tables
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto itr = _bounties.find( bounty_id.value );
    check( itr == _bounties.end(), "pomelo::create: [bounty_id] already exists" );

    auto token = get_token( accepted_token );

    // create bounty
    _bounties.emplace( get_self(), [&]( auto & row ) {
        row.bounty_id = bounty_id;
        row.author_user_id = author_user_id;
        row.funders = {};
        row.amount = extended_asset{0, { token.sym, token.contract }};
        row.fee = extended_asset{0, { token.sym, token.contract }};
        row.claimed = asset{0, token.sym};
        row.applicant_user_ids = {};
        row.approved_user_id = {};
        row.status = "pending"_n;
        row.type = type;
        row.permissions = "approval"_n;
        row.created_at = current_time_point();
        row.updated_at = current_time_point();
    });
    pomelo::createlog_action createlog( get_self(), { get_self(), "active"_n });
    createlog.send( bounty_id, author_user_id, extended_symbol{ token.sym, token.contract }, type, "approval"_n );
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "pending"_n, "create"_n );
}

// @author
[[eosio::action]]
void pomelo::setmetadata( const name bounty_id, const name key, const string value )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    auto & bounty = _bounties.get( bounty_id.value, "pomelo::setmetadata: [bounty_id] does not exist" );

    // require auth by author
    eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );
    check( get_configs().metadata_keys.count(key), "pomelo::setmetadata: [metadata_key] not allowed" );

    if (value == "") check( bounty.metadata.count(key), "pomelo::setmetadata: [metadata_key] not set" );
    else check( bounty.metadata.count(key) == 0 || bounty.metadata.at(key) != value , "pomelo::setmetadata: [metadata_key] was not modified" );

    // modify
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        if (value == "") row.metadata.erase(key);
        else row.metadata[key] = value;
        row.updated_at = current_time_point();
    });
}

// @admin
[[eosio::action]]
void pomelo::setstate( const name bounty_id, const name status )
{
    require_auth( get_self() );

    // validate input
    check( STATUS_TYPES.count(status), "pomelo::setstate: invalid [status]" );

    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    auto & bounty = _bounties.get( bounty_id.value, "pomelo::setstate: [bounty_id] does not exist");

    check( bounty.status != status, "pomelo::setstate: status was not modified");
    if (status == "open"_n) check( bounty.amount.quantity.amount > 0, "pomelo::setstate: `open` bounty needs to be funded" );
    if (status == "started"_n) check( bounty.approved_user_id.value, "pomelo::setstate: `started` bounty needs to have a hunter" );
    if (status == "submitted"_n) check( bounty.submitted_at.sec_since_epoch(), "pomelo::setstate: `submitted` bounty needs to be completed first" );
    if (status == "released"_n) check( bounty.submitted_at.sec_since_epoch(), "pomelo::setstate: `released` bounty needs to be completed first" );
    if (status == "done"_n) check( bounty.claimed.amount > 0, "pomelo::setstate: `done` bounty needs to be claimed first" );

    // modify
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = status;
        row.updated_at = current_time_point();
    });

    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, status, "setstate"_n );
}

// @admin
[[eosio::action]]
void pomelo::syncbounty(const name bounty_id, const name status, const vector<name> applicant_user_ids, const optional<name> approved_user_id, const time_point_sec updated_at, const optional<time_point_sec> submitted_at, const optional<time_point_sec> completed_at)
{
    require_auth( get_self() );

    // validate input
    check( STATUS_TYPES.count(status), "pomelo::syncbounty: invalid [status]" );

    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    auto & bounty = _bounties.get( bounty_id.value, "pomelo::syncbounty: [bounty_id] does not exist");

    // modify
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = status;
        row.applicant_user_ids = set<name>( applicant_user_ids.begin(), applicant_user_ids.end() );
        row.updated_at = updated_at;
        if (approved_user_id) row.approved_user_id = *approved_user_id;
        if (submitted_at) row.submitted_at = *submitted_at;
        if (completed_at) row.completed_at = *completed_at;
    });

    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, status, "syncbounty"_n );
}

// @admin
[[eosio::action]]
void pomelo::setconfig( const optional<name> status, const optional<uint64_t> fee, const optional<name> login_contract, const optional<name> fee_account, const set<name> metadata_keys )
{
    require_auth( get_self() );

    pomelo::configs_table _configs( get_self(), get_self().value );
    auto configs = _configs.get_or_default();

    if ( status ) configs.status = *status;
    if ( fee ) configs.fee = *fee;
    if ( login_contract ) configs.login_contract = *login_contract;
    if ( fee_account ) configs.fee_account = *fee_account;
    if ( metadata_keys.size() ) configs.metadata_keys = metadata_keys;
    _configs.set( configs, get_self() );
}

// @author
[[eosio::action]]
void pomelo::approve( const name bounty_id, const name applicant_user_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::approve: [bounty_id] does not exists" );

    // require auth by author
    eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "open"_n, "pomelo::approve: [bounty.status] must be `open` to `approve`" );
    check( bounty.applicant_user_ids.find( applicant_user_id ) != bounty.applicant_user_ids.end(), "pomelo::approve: [applicant_user_id] did not apply" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.approved_user_id = applicant_user_id;
        row.status = "started"_n;
        row.updated_at = current_time_point();
    });

    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "started"_n, "approve"_n );
}

// @author or @admin
[[eosio::action]]
void pomelo::forfeit( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::forfeit: [bounty_id] does not exists" );

    // require auth by hunter
    eosn::login::require_auth_user_id( bounty.approved_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "started"_n, "pomelo::forfeit: [bounty.status] must be `started` to forfeit" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.applicant_user_ids.erase(bounty.approved_user_id);
        row.approved_user_id = {};
        row.status = "open"_n;
        row.updated_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "open"_n, "forfeit"_n );
}

// @author or @admin
[[eosio::action]]
void pomelo::release( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::release: [bounty_id] does not exists" );

    // require auth by author
   eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "submitted"_n, "pomelo::release: [bounty.status] must be `submitted` to `release`" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "released"_n;
        row.updated_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "released"_n, "release"_n );
}

// @author
[[eosio::action]]
void pomelo::deny( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::deny: [bounty_id] does not exists" );

    // require auth by author
    eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "submitted"_n, "pomelo::deny: [bounty.status] must be `submitted` to `deny`" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "started"_n;
        row.updated_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "started"_n, "deny"_n );
}

// @author
[[eosio::action]]
void pomelo::close( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::close: [bounty_id] does not exists" );

    // require auth by author
    eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "pending"_n || bounty.status == "open"_n, "pomelo::close: [bounty.status] must be `pending` or `open`" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "closed"_n;
        row.updated_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "closed"_n, "close"_n );
}

// @admin
[[eosio::action]]
void pomelo::publish( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::publish: [bounty_id] does not exists" );

    require_auth(get_self());

    // validate input
    check( bounty.status == "pending"_n, "pomelo::publish: [bounty.status] must be `pending` to publish" );
    check( bounty.amount.quantity.amount != 0, "pomelo::publish: bounty must be funded to publish");

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "open"_n;
        row.updated_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "open"_n, "publish"_n );
}

// @author
[[eosio::action]]
void pomelo::withdraw( const name bounty_id, const name receiver )
{
    require_auth( receiver );

    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::withdraw: [bounty_id] does not exists" );

    // require auth by author
    check( eosn::login::is_auth( bounty.author_user_id, get_configs().login_contract ), "pomelo::withdraw: [receiver] must be linked with EOSN Login to [author_user_id]" );

    // validate input
    check( bounty.status == "pending"_n || bounty.status == "closed"_n, "pomelo::withdraw: [bounty.status] must be `pending` or `closed` to withdraw" );

    const auto refund = bounty.amount + bounty.fee;
    check( refund.quantity.amount > 0, "pomelo::withdraw: nothing to withdraw" );
    check( bounty.claimed.amount == 0, "pomelo::withdraw: [bounty_id] already claimed" );

    // tranfer bounty funds to receiver
    transfer(get_self(), receiver, refund, "🍈 withdraw " + bounty_id.to_string() + " bounty" );

    // set bounty amount to zero
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.amount.quantity.amount = 0;
        row.fee.quantity.amount = 0;
        row.funders = {};
        row.updated_at = current_time_point();
    });
    pomelo::withdrawlog_action withdrawlog( get_self(), { get_self(), "active"_n });
    withdrawlog.send( bounty_id, bounty.status, bounty.author_user_id, receiver, refund );
}

// @applicant
[[eosio::action]]
void pomelo::apply( const name bounty_id, const name user_id )
{
    // require auth by applicant
    eosn::login::require_auth_user_id( user_id, get_configs().login_contract );

    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::apply: [bounty_id] does not exists" );

    // validate input
    check( bounty.status == "open"_n, "pomelo::apply: [bounty.status] must be `open` to `apply`" );
    check( bounty.applicant_user_ids.find( user_id ) == bounty.applicant_user_ids.end(), "pomelo::apply: [user_id] is already applicant" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.applicant_user_ids.insert( user_id );
        row.updated_at = current_time_point();
    });
}

// @applicant
[[eosio::action]]
void pomelo::complete( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::apply: [bounty_id] does not exists" );

    check( bounty.approved_user_id.value, "pomelo::apply: bounty must have approved user" );

    // require auth by applicant
    eosn::login::require_auth_user_id( bounty.approved_user_id, get_configs().login_contract );

    // validate input
    check( bounty.status == "started"_n, "pomelo::apply: [bounty.status] must be `started` to `complete`" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "submitted"_n;
        row.updated_at = current_time_point();
        row.submitted_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "submitted"_n, "complete"_n );
}

// @applicant
[[eosio::action]]
void pomelo::claim( const name bounty_id, const name receiver )
{
    require_auth( receiver );

    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::claim: [bounty_id] does not exists" );

    // can only claim when released or submitted (after 72 hours)
    check( bounty.status == "released"_n || bounty.status == "submitted"_n, "pomelo::claim: [bounty.status] must be `released` or `submitted` to `claim`" );

    // require auth by applicant
    check( eosn::login::is_auth( bounty.approved_user_id, get_configs().login_contract ), "pomelo::claim: [receiver] must be linked with EOSN Login to [approved_user_id]" );

    // bounty can be claimed after 72 hours of being submitted
    const uint32_t sec_since_submitted = current_time_point().sec_since_epoch() - bounty.submitted_at.sec_since_epoch();
    if ( sec_since_submitted < DAY * 3 ) {
        check( bounty.status == "released"_n, "pomelo::claim: [bounty.status] must be `released` to `claim` or wait 72 hours" );
    }

    // allow claim once only
    // TODO: consider flow when bounty is re-opened after claim and funded again allowing second claim
    check( bounty.claimed.amount == 0, "pomelo::claim: bounty already claimed" );

    const auto balance = utils::get_balance((bounty.amount + bounty.fee).get_extended_symbol(), get_self());
    check( balance >= bounty.amount + bounty.fee, "pomelo::claim: not enough balance to claim" );

    // tranfer bounty funds to receiver
    transfer(get_self(), receiver, bounty.amount, "🍈 claim " + bounty_id.to_string() + " bounty" );

    // transfer fee to fee account
    transfer( get_self(), get_configs().fee_account, bounty.fee, "🍈 Pomelo team");

    // set bounty amount to zero
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.claimed += bounty.amount.quantity;
        row.status = "done"_n;
        row.updated_at = current_time_point();
        row.completed_at = current_time_point();
    });
    pomelo::statelog_action statelog( get_self(), { get_self(), "active"_n });
    statelog.send( bounty_id, "done"_n, "claim"_n );

    const uint32_t sec_since_created = current_time_point().sec_since_epoch() - bounty.created_at.sec_since_epoch();
    pomelo::claimlog_action claimlog( get_self(), { get_self(), "active"_n });
    claimlog.send( bounty_id, receiver, bounty.amount, bounty.fee.quantity, bounty.status, bounty.approved_user_id, sec_since_created / DAY );
}

void pomelo::transfer( const name from, const name to, const extended_asset value, const string memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}