[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || from == "eosio.ram"_n ) return;

    // parse memo
    auto [ bounty_id, user_id ] = parse_memo(memo);
    check(bounty_id.value, ERROR_INVALID_MEMO);

    // handle token transfer
    const extended_asset ext_quantity = { quantity, get_first_receiver() };
    deposit_bounty( bounty_id, user_id, from, ext_quantity, memo );
}

void pomelo::deposit_bounty( const name bounty_id, const name user_id, const name from, const extended_asset ext_quantity, const string memo )
{
    const auto fee_account = get_configs().fee_account;

    pomelo::bounties_table _bounties( get_self(), get_self().value );
    auto bounty = _bounties.find(bounty_id.value);
    check(bounty != _bounties.end(), "pomelo::deposit_bounty: [bounty_id=" + bounty_id.to_string() + "] does not exists");

    const asset quantity = ext_quantity.quantity;
    const symbol_code symcode = quantity.symbol.code();
    const auto token = get_token( ext_quantity );
    const int64_t min_amount = token.min_amount;
    const int64_t max_amount = token.max_amount;
    name funder_user_id = user_id.value ? user_id : bounty->author_user_id;         // if no user id specified - assume it came from the author
    if ( eosn::login::is_user_id_exists(from, get_self() ) ) funder_user_id = from; // if EOSN linked account is sender - override as funder

    // TO-DO: bounty can only deposit when state == "pending/open/started"
    check( STATUS_DEPOSIT_TYPES.find(bounty->status) != STATUS_DEPOSIT_TYPES.end(), "pomelo::deposit_bounty: bounty not available for funding");

    // check incoming token deposit
    check( bounty->amount.get_extended_symbol() == ext_quantity.get_extended_symbol(), "pomelo::deposit_bounty: quantity extended symbol not allowed");
    check( is_token_enabled( symcode ), "pomelo::deposit_bounty: [token=" + symcode.to_string() + "] is not supported");

    // require EOSN linked login to allow withdrawals
    check( is_user(funder_user_id), "pomelo::deposit_bounty: [funder_user_id] must be EOSN account");

    // calculate fee
    const extended_asset fee = calculate_fee( ext_quantity );
    const extended_asset amount = ext_quantity - fee;
    const double value = utils::asset_to_double(amount.quantity);
    print("\n", amount, " == ", value);

    // update bounty deposit
    _bounties.modify( bounty, get_self(), [&]( auto & row ) {
        if(row.funders.count(funder_user_id) == 0) row.funders[funder_user_id] = asset{0, quantity.symbol};
        row.funders[funder_user_id] += quantity;
        row.amount += amount;
        row.fee += fee;
        row.updated_at = current_time_point();

        // validate deposit min & max amounts
        const int64_t total = row.amount.quantity.amount;
        check( total >= min_amount, "pomelo::deposit_bounty: [quantity=" + ext_quantity.quantity.to_string() + "] is less than [tokens.min_amount=" + to_string( min_amount ) + "]");
        check( total <= max_amount, "pomelo::deposit_bounty: [quantity=" + ext_quantity.quantity.to_string() + "] is greater than [tokens.max_amount=" + to_string( max_amount ) + "]");
    });

    pomelo::depositlog_action depositlog( get_self(), { get_self(), "active"_n });
    depositlog.send( bounty_id, funder_user_id, from, amount, fee.quantity, value, memo );
}
