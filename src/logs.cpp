[[eosio::action]]
void pomelo::depositlog( const name bounty_id, const name funder_user_id, const name from, const extended_asset ext_quantity, const asset fee, const double value, const string& memo )
{
    require_auth( get_self() );
}

[[eosio::action]]
void pomelo::createlog( const name bounty_id, const name author_user_id, extended_symbol ext_sym, const name type, const name permissions )
{
    require_auth( get_self() );
}

[[eosio::action]]
void pomelo::statelog( const name bounty_id, const name status, const name action )
{
    require_auth( get_self() );
}

[[eosio::action]]
void pomelo::claimlog( const name bounty_id, const name chain, const string receiver, const extended_asset bounty, const asset fee, const name status, const name approved_user_id, const uint32_t days_since_created )
{
    require_auth( get_self() );
}

[[eosio::action]]
void pomelo::withdrawlog( const name bounty_id, const name chain, const string receiver, const extended_asset refund, const name status, const name author_user_id )
{
    require_auth( get_self() );
}
