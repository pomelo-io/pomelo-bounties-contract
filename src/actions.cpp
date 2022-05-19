#include <oracle.defi/oracle.defi.hpp>
#include <sx.utils/utils.hpp>

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

    // modify
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        check( row.status != status, "pomelo::setstate: status was not modified");
        row.status = status;
        row.updated_at = current_time_point();
    });
}

// @admin
[[eosio::action]]
void pomelo::setconfig( const optional<uint64_t> fee, const optional<name> login_contract, const optional<name> fee_account )
{
    require_auth( get_self() );

    pomelo::configs_table _configs( get_self(), get_self().value );
    auto configs = _configs.get_or_default();

    if ( fee ) configs.fee = *fee;
    if ( login_contract ) configs.login_contract = *login_contract;
    if ( fee_account ) configs.fee_account = *fee_account;
    _configs.set( configs, get_self() );
}

// @admin
[[eosio::action]]
void pomelo::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    pomelo::transfers_table _transfers( get_self(), value );
    pomelo::bounties_table _bounties( get_self(), value );
    pomelo::configs_table _configs( get_self(), value );
    pomelo::tokens_table _tokens( get_self(), value );
    pomelo::status_table _status( get_self(), value );

    if (table_name == "transfers"_n) clear_table( _transfers, rows_to_clear );
    else if (table_name == "bounties"_n) clear_table( _bounties, rows_to_clear );
    else if (table_name == "tokens"_n) clear_table( _tokens, rows_to_clear );
    else if (table_name == "configs"_n) _configs.remove();
    else if (table_name == "status"_n) _status.remove();
    else check(false, "pomelo::cleartable: [table_name] unknown table to clear" );
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
}

// @author
[[eosio::action]]
void pomelo::forfeit( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::forfeit: [bounty_id] does not exists" );

    // require auth by author
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
}

// @author
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
}

// @author or @admin
[[eosio::action]]
void pomelo::close( const name bounty_id )
{
    // get bounty
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    const auto & bounty = _bounties.get( bounty_id.value, "pomelo::close: [bounty_id] does not exists" );

    // require auth by admin or author
    if( !has_auth(get_self()) ){
        eosn::login::require_auth_user_id( bounty.author_user_id, get_configs().login_contract );
    }

    // validate input
    check( bounty.status == "pending"_n || bounty.status == "open"_n, "pomelo::close: [bounty.status] must be `pending` or `open`" );

    // update bounty
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        row.status = "closed"_n;
        row.updated_at = current_time_point();
    });
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

    const auto balance = sx::utils::get_balance((bounty.amount + bounty.fee).get_extended_symbol(), get_self());
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
}
