// @debug
template <typename T>
void pomelo::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void pomelo::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    pomelo::bounties_table _bounties( get_self(), value );
    pomelo::tokens_table _tokens( get_self(), value );
    pomelo::configs_table _configs( get_self(), value );
    pomelo::deposits_table _deposits( get_self(), value );

    if (table_name == "bounties"_n) clear_table( _bounties, rows_to_clear );
    else if (table_name == "tokens"_n) clear_table( _tokens, rows_to_clear );
    else if (table_name == "deposits"_n) clear_table( _deposits, rows_to_clear );
    else if (table_name == "configs"_n) _configs.remove();
    else check(false, "pomelo::cleartable: [table_name] unknown table to clear" );
}