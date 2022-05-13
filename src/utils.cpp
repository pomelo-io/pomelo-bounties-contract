#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <sx.utils/utils.hpp>

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
    auto parts = sx::utils::split(memo, ",");
    if (parts.size() > 2) return {};
    name bounty = sx::utils::parse_name(parts[0]);
    name user_id = parts.size() == 2 ? sx::utils::parse_name(parts[1]) : name{};
    return { bounty, user_id };
}

template <typename T>
void pomelo::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

void pomelo::update_status( const uint32_t index, const uint32_t count )
{
    status_table _status( get_self(), get_self().value );
    auto status = _status.get_or_default();

    if ( status.counters.size() <= index ) status.counters.resize( index + 1);
    status.counters[index] += count;
    status.last_updated = current_time_point();
    _status.set( status, get_self() );
}

void pomelo::transfer( const name from, const name to, const extended_asset value, const string memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}