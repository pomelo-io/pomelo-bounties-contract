[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // parse memo
    const auto [ bounty_id, user_id ] = parse_memo(memo);
    check(bounty_id.value, ERROR_INVALID_MEMO);

    // handle token transfer
    const extended_asset ext_quantity = { quantity, get_first_receiver() };
    deposit_bounty( bounty_id, user_id, from, ext_quantity, memo );
    update_status(0, 1);
}

void pomelo::deposit_bounty( const name bounty_id, const name user_id, const name from, const extended_asset ext_quantity, const string memo )
{
    const auto fee_account = get_configs().fee_account;

    pomelo::bounties_table _bounties( get_self(), get_self().value );
    auto & bounty = _bounties.get(bounty_id.value, "pomelo::deposit_bounty: [bounty_id] does not exists");

    const asset quantity = ext_quantity.quantity;
    const symbol_code symcode = quantity.symbol.code();
    const int64_t min_amount = get_token( ext_quantity ).min_amount;
    const name funder_user_id = user_id.value ? user_id : bounty.author_user_id;    //if no user id specified - assume it came from the author

    // validate incoming transfer
    check( quantity.amount >= min_amount, "pomelo::deposit_bounty: [quantity=" + ext_quantity.quantity.to_string() + "] is less than [tokens.min_amount=" + to_string( min_amount ) + "]");

    // TO-DO: bounty can only deposit when state == "pending/open/started"
    check( STATUS_DEPOSIT_TYPES.find(bounty.status) != STATUS_DEPOSIT_TYPES.end(), "pomelo::deposit_bounty: bounty not available for funding");

    // check incoming token deposit
    check( bounty.amount.get_extended_symbol() == ext_quantity.get_extended_symbol(), "pomelo::deposit_bounty: quantity extended symbol not allowed");
    check( is_token_enabled( symcode ), "pomelo::deposit_bounty: [token=" + symcode.to_string() + "] is not supported");

    // require EOSN linked login to allow withdrawals
    check( is_user(funder_user_id), "pomelo::deposit_bounty: [funder_user_id] must be EOSN account");

    // calculate fee
    const extended_asset fee = calculate_fee( ext_quantity );
    double value = calculate_value( ext_quantity - fee );
    print("\n", ext_quantity - fee, " == ", value);
    transfer( get_self(), fee_account, fee, "🍈 Pomelo team");

    // update bounty deposit
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        if(row.funders.count(user_id) == 0) row.funders[user_id] = asset{0, quantity.symbol};
        row.funders[user_id] += quantity;
        row.amount += ext_quantity;
        row.updated_at = current_time_point();
    });

    // save for logging purposes
    save_transfer( bounty_id, funder_user_id, from, get_self(), ext_quantity, fee.quantity, memo, value );
}

void pomelo::save_transfer( const name bounty_id, const name funder_user_id, const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const double value )
{
    pomelo::transfers_table _transfers( get_self(), get_self().value );
    _transfers.emplace( get_self(), [&]( auto & row ) {
        row.transfer_id = _transfers.available_primary_key();
        row.bounty_id = bounty_id;
        row.funder_user_id = funder_user_id;
        row.from = from;
        row.to = to;
        row.ext_quantity = ext_quantity;
        row.fee = fee;
        row.memo = memo;
        row.value = value;
        row.trx_id = get_trx_id();
        row.created_at = current_time_point();
    });
}
