#include <oracle.defi/oracle.defi.hpp>

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
void pomelo::createbounty( const name author_id, const name bounty_id, const symbol_code accepted_token )
{
    eosn::login::require_auth_user_id( author_id, get_globals().login_contract );

    // tables
    pomelo::bounties_table _bounties( get_self(), get_self().value );

    // validate input
    const auto itr = projects.find( project_id.value );
    check( itr.find(bounty_id.value) == grants.end(), "pomelo::createbounty: [bounty_id] already exists" );
    check( is_token_enabled( accepted_token ), "pomelo::createbounty: [accepted_token] is not available" );

    // create bounty
    projects.emplace( get_self(), [&]( auto & row ) {
        row.bounty_id = bounty_id;
        row.author_user_id = author_id;
        row.bounty = bounty;
        row.hunters = hunters;
        row.status = "pending"_n;
        row.type = "traditional"_n;
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

    pomelo::globals_table _globals( get_self(), get_self().value );
    auto globals = _globals.get_or_default();

    if ( fee ) globals.fee = *fee;
    if ( login_contract ) globals.login_contract = *login_contract;
    if ( fee_account ) globals.fee_account = *fee_account;
    _globals.set( globals, get_self() );
}

// @admin
[[eosio::action]]
void pomelo::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = *max_rows == 0 ? -1 : *max_rows;
    const uint64_t scope = scope ? *scope.value : get_self().value;

    // tables
    pomelo::transfers_table transfers( get_self(), scope );
    pomelo::bounties_table bounties( get_self(), scope );
    pomelo::configs_table configs( get_self(), scope );
    pomelo::tokens_table tokens( get_self(), scope );
    pomelo::status_table status( get_self(), scope );

    if (table_name == "transfers"_n) clear_table( transfers, rows_to_clear );
    else if (table_name == "bounties"_n) clear_table( bounties, rows_to_clear );
    else if (table_name == "tokens"_n) clear_table( tokens, rows_to_clear );
    else if (table_name == "configs"_n) configs.remove();
    else if (table_name == "status"_n) status.remove();
    else check(false, "pomelo::cleartable: [table_name] unknown table to clear" );
}

void pomelo::transfer( const name from, const name to, const extended_asset value, const string memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}