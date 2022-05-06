[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // tables
    pomelo::globals_table _globals( get_self(), get_self().value );

    // state
    check( _globals.exists(), "pomelo::on_transfer: contract is under maintenance");

    // parse memo
    const name bounty_id = parse_name(memo);
    check(bounty_id.value, "pomelo::on_transfer: invalid bounty id");

    // handle token transfer
    const extended_asset ext_quantity = { quantity, get_first_receiver() };
    get_token( ext_quantity ); // check if valid token & exists
    deposit_bounty( bounty_id, from, ext_quantity, memo );
    update_status(0, 1);
}
