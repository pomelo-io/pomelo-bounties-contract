void pomelo::handle_bridge_transfer( const name chain, const string receiver, const extended_asset value, const string memo, const string default_memo )
{
    // handle EOS Native transfer
    if ( chain == "eos"_n) {
        const name account = utils::parse_name(receiver);
        check( account.value, "pomelo::handle_bridge_transfer: [receiver] is required" );
        check( is_account( account ), "pomelo::handle_bridge_transfer: [receiver] account does not exist");
        transfer(get_self(), account, value, memo.length() ? memo : default_memo );

    // handle EOS EVM transfer
    } else if (chain == "eos.evm"_n) {
        const auto config = get_configs();

        // include ingress fee
        // TO-DO pull ingress fee from `eosio.evm2o` contract
        const extended_asset ingress_fee = {100, value.get_extended_symbol()};
        transfer(config.fee_account, get_self(), ingress_fee, "ingress fee" );
        transfer(get_self(), "eosio.evmin"_n, value + ingress_fee, receiver );

    // unsupported chain
    } else {
        check( false, "pomelo::handle_bridge_transfer: [chain] not supported" );
    }
}
