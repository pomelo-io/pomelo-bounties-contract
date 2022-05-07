#include <oracle.defi/oracle.defi.hpp>


extended_asset pomelo::calculate_fee( const extended_asset ext_quantity )
{
    const int64_t amount = ext_quantity.quantity.amount * get_globals().fee / 10000;
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
