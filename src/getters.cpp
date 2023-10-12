#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <utils/utils.hpp>

checksum256 pomelo::get_trx_id()
{
    size_t size = transaction_size();
    char buf[size];
    size_t read = read_transaction( buf, size );
    check( size == read, "pomelo::get_trx_id: read_transaction failed");
    return sha256( buf, read );
}

pair<name, name> pomelo::parse_memo(const string& memo)
{
    if (memo.length() == 0) return {};
    auto parts = utils::split(memo, ",");
    if (parts.size() > 2) return {};
    name bounty = utils::parse_name(parts[0]);
    name user_id = parts.size() == 2 ? utils::parse_name(parts[1]) : name{};
    return { bounty, user_id };
}

extended_asset pomelo::calculate_fee( const extended_asset ext_quantity )
{
    const int64_t deposit = ext_quantity.quantity.amount;
    const double rate = 1.0 + static_cast<double>(get_configs().fee) / 10000.0;
    const int64_t amount = deposit - (deposit / rate);
    return { amount, ext_quantity.get_extended_symbol() };
}

pomelo::configs_row pomelo::get_configs()
{
    pomelo::configs_table _configs( get_self(), get_self().value );
    check( _configs.exists() && (_configs.get().status == "ok"_n ||  _configs.get().status == "testing"_n), "pomelo::get_global: contract is under maintenance");
    return _configs.get();
}

pomelo::tokens_row pomelo::get_token( const extended_symbol ext_sym )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    const auto& token = _tokens.get( ext_sym.get_symbol().code().raw(), "pomelo::get_token: [symcode] not supported" );
    check(token.contract == ext_sym.get_contract(), "pomelo::get_token: [token.contract] is invalid");
    return token;
}

pomelo::tokens_row pomelo::get_token( const symbol_code symcode )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    return _tokens.get( symcode.raw(), "pomelo::get_token: [symcode] not supported" );
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

name pomelo::get_user_id( const name account )
{
    const name login_contract = get_configs().login_contract;
    eosn::login::accounts_table _accounts( login_contract, login_contract.value );
    return _accounts.get(account.value, "pomelo::get_user_id: account is not linked to EOSN account").user_id;
}

bool pomelo::is_user( const name user_id )
{
    const name login_contract = get_configs().login_contract;
    eosn::login::users_table users( login_contract, login_contract.value );
    return users.find(user_id.value) != users.end();
}
