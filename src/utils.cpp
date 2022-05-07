#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

checksum256 get_trx_id()
{
    size_t size = transaction_size();
    char buf[size];
    size_t read = read_transaction( buf, size );
    check( size == read, "pomelo::get_trx_id: read_transaction failed");
    return sha256( buf, read );
}

static name parse_name(const string& str) {

    if (str.length() == 0 || str.length() > 12) return {};
    int i=0;
    for (const auto c: str) {
        if ((c >= 'a' && c <= 'z') || ( c >= '0' && c <= '5') || c == '.') {
            if (i == str.length() - 1 && c == '.') return {};                  //can't end with a .
        }
        else return {};
        i++;
    }
    return name{str};
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