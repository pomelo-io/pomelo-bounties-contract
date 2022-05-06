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

/**
 * ## STATIC `parse_name`
 *
 * Parse string for account name. Return default name if invalid. Caller can check validity with name::value.
 *
 * ### params
 *
 * - `{string} str` - string to parse
 *
 * ### returns
 *
 * - `{name}` - name
 *
 * ### example
 *
 * ```c++
 * const name contract = utils::parse_name( "tethertether" );
 * // contract => "tethertether"_n
 * ```
 */
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