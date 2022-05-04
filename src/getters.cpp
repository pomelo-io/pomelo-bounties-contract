#include <oracle.defi/oracle.defi.hpp>

using namespace sx;

extended_asset pomelo::calculate_fee( const extended_asset ext_quantity )
{
    const int64_t amount = ext_quantity.quantity.amount * get_globals().grant_fee / 10000;
    return { amount, ext_quantity.get_extended_symbol() };
}

pomelo::globals_row pomelo::get_globals()
{
    pomelo::globals_table _globals( get_self(), get_self().value );
    check( _globals.exists(), "pomelo::get_global: contract is under maintenance");
    return _globals.get();
}

pomelo::tokens_row pomelo::get_token( const extended_symbol ext_sym )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    const auto& token = _tokens.get( ext_sym.get_symbol().code().raw(), "pomelo::get_token: [symcode] not supported" );
    check(token.contract == ext_sym.get_contract(), "pomelo::get_token: [token.contract] is invalid");
    return token;
}

pomelo::tokens_row pomelo::get_token( const extended_asset ext_quantity )
{
    return get_token(ext_quantity.get_extended_symbol());
}

bool pomelo::is_token_enabled( const symbol_code symcode )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    auto itr = _tokens.find( symcode.raw() );
    return itr != _tokens.end();
}

double pomelo::calculate_value( const extended_asset ext_quantity )
{
    const auto& token = get_token( ext_quantity );
    return defi::oracle::get_value( ext_quantity, token.oracle_id );
}

name pomelo::get_user_id( const name account )
{
    const name login_contract = get_globals().login_contract;
    eosn::login::accounts_table _accounts( login_contract, login_contract.value );
    return _accounts.get(account.value, "pomelo::get_user_id: account is not linked to EOSN account").user_id;
}

bool pomelo::is_user( const name user_id )
{
    const name login_contract = get_globals().login_contract;
    eosn::login::users_table users( login_contract, login_contract.value );

    return users.find(user_id.value) != users.end();
}

void pomelo::validate_round( const uint16_t round_id )
{
    pomelo::rounds_table rounds( get_self(), get_self().value );
    pomelo::seasons_table seasons( get_self(), get_self().value );

    check(round_id != 0, "pomelo::validate_round: [round_id] is not active");

    const auto now = current_time_point().sec_since_epoch();
    const auto round = rounds.get( round_id, "pomelo::validate_round: [round_id] not found");
    const auto season = seasons.get( round.season_id, "pomelo::validate_round: [season_id] not found");
    check(season.start_at.sec_since_epoch() <= now, "pomelo::validate_round: [season_id] has not started");
    check(now <= season.end_at.sec_since_epoch(), "pomelo::validate_round: [season_id] has expired");
}

uint16_t pomelo::get_active_round( const name grant_id )
{
    const auto season_id = get_globals().season_id;
    if( season_id == 0) return 0;

    pomelo::seasons_table _seasons( get_self(), get_self().value );
    pomelo::rounds_table _rounds( get_self(), get_self().value );

    uint16_t active_round_id = 0;
    const auto& season = _seasons.get( season_id, "pomelo::get_active_round: [season_id] not found");
    for( const auto round_id: season.round_ids ){
        const auto round = _rounds.get( round_id, "pomelo::get_active_round: [round_id] not found");
        if( get_index( round.grant_ids, grant_id ) != -1){
            check(active_round_id == 0, "pomelo::get_active_round: [grant_id] exist in multiple active rounds");
            active_round_id = round_id;
        }
    }
    return active_round_id;
}
