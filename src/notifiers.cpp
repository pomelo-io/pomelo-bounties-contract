[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // tables
    pomelo::grants_table _grants( get_self(), get_self().value );
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    pomelo::globals_table _globals( get_self(), get_self().value );

    // state
    check( _globals.exists(), "pomelo::on_transfer: contract is under maintenance");

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check( memo_parts.size() == 2, ERROR_INVALID_MEMO);
    const name project_type = sx::utils::parse_name(memo_parts[0]);
    const name project_id = sx::utils::parse_name(memo_parts[1]);

    check(project_id.value, "pomelo::on_transfer: invalid project id");

    // handle token transfer
    const extended_asset ext_quantity = { quantity, get_first_receiver() };
    get_token( ext_quantity ); // check if valid token & exists

    if (project_type == "grant"_n) {
        donate_project(_grants, project_id, from, ext_quantity, memo );
    } else if (project_type == "bounty"_n) {
        donate_project(_bounties, project_id, from, ext_quantity, memo );
    } else {
        check( false, ERROR_INVALID_MEMO);
    }
    update_status(0, 1);
}

[[eosio::on_notify("*::social")]]
void pomelo::on_social( const name user_id, const name social )
{
    update_social( user_id );
}

[[eosio::on_notify("*::unsocial")]]
void pomelo::on_unsocial( const name user_id, const optional<name> social )
{
    update_social( user_id );
}

void pomelo::update_social( const name user_id )
{
    const uint16_t season_id = get_globals().season_id;

    const name login_contract = get_globals().login_contract;
    if ( season_id == 0 || get_first_receiver() != login_contract ) return;

    pomelo::seasons_table _seasons( get_self(), get_self().value );
    pomelo::rounds_table _rounds( get_self(), get_self().value );

    const auto weight = eosn::login::get_user_weight( user_id, get_globals().login_contract );
    const double new_multiplier = static_cast<double>( weight ) / 100;

    const auto& season = _seasons.get( season_id, "pomelo::update_social: [season_id] not found");
    for( const auto& round_id: season.round_ids ){
        pomelo::match_table _match( get_self(), round_id );
        pomelo::users_table _users( get_self(), round_id );

        const auto round_itr = _rounds.find( round_id );
        if (round_itr == _rounds.end() || get_index( round_itr->user_ids, user_id ) == -1) continue;

        const auto user_itr = _users.find( user_id.value );
        if (user_itr == _users.end() || user_itr->multiplier == new_multiplier) continue;

        const auto old_multiplier = user_itr->multiplier;
        for (int i = 0; i < user_itr->contributions.size(); i++) {

            const auto old_value = user_itr->contributions[i].value;
            const auto donated = old_value / (1 + old_multiplier);
            const auto new_value = donated * (1 + new_multiplier);
            const auto old_boost = donated * old_multiplier;
            const auto new_boost = donated * new_multiplier;

            const auto match_itr = _match.find( user_itr->contributions[i].id.value );
            double old_square = match_itr->square, new_square = 0;

            _match.modify( match_itr, get_self(), [&]( auto & row ) {
                row.sum_boost += new_boost - old_boost;
                row.sum_sqrt += sqrt(new_value) - sqrt(old_value);
                new_square = row.square = row.sum_sqrt * row.sum_sqrt;
                row.updated_at = current_time_point();
            });

            _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
                row.sum_boost += new_boost - old_boost;
                row.sum_square += new_square - old_square;
                row.updated_at = current_time_point();
            });

            _users.modify( user_itr, get_self(), [&]( auto & row ) {
                row.multiplier = new_multiplier;
                row.boost = row.value * new_multiplier;
                row.contributions[i].value = new_value;
                row.updated_at = current_time_point();
            });
        }
    }
}