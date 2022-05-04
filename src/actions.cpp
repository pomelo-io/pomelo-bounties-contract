#include <oracle.defi/oracle.defi.hpp>

// @admin
[[eosio::action]]
void pomelo::setfunding( const name grant_id, const name funding_account)
{
    require_auth( get_self() );
    pomelo::grants_table grants( get_self(), get_self().value );

    auto & grant = grants.get(grant_id.value, "pomelo::setgrantid: [grant_id] does not exists");
    grants.modify( grant, get_self(), [&]( auto & row ) {
        row.funding_account = funding_account;
    });
}

// @admin
[[eosio::action]]
void pomelo::setgrantid( const name grant_id, const name new_grant_id )
{
    require_auth( get_self() );
    pomelo::grants_table grants( get_self(), get_self().value );

    auto & grant = grants.get(grant_id.value, "pomelo::setgrantid: [grant_id] does not exists");
    grants.emplace( get_self(), [&]( auto & row ) {
        row.id              = new_grant_id;
        row.type            = grant.type;
        row.author_user_id  = grant.author_user_id;
        row.funding_account = grant.funding_account;
        row.accepted_tokens = grant.accepted_tokens;
        row.status          = grant.status;
        row.created_at      = grant.created_at;
        row.updated_at      = grant.updated_at;
    });
    grants.erase( grant );
}

// @admin
[[eosio::action]]
void pomelo::token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t oracle_id )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );

    const asset supply = token::get_supply( contract, sym.code() );
    check( supply.symbol == sym, "pomelo::token: [sym] symbol does not match with token supply");
    check( supply.amount, "pomelo::token: [sym] has no supply");

    // check if Oracle exists; if not it will assert fail
    if ( is_account( oracle_code ) && extended_symbol{ sym, contract } != VALUE_SYM )
        defi::oracle::get_value( {10000, extended_symbol{ sym, contract }}, oracle_id );

    const auto insert = [&]( auto & row ) {
        row.sym = sym;
        row.contract = contract;
        row.min_amount = min_amount;
        row.oracle_id = oracle_id;
    };

    const auto itr = tokens.find( sym.code().raw() );
    if ( itr == tokens.end() ) tokens.emplace( get_self(), insert );
    else tokens.modify( itr, get_self(), insert );
}

[[eosio::action]]
void pomelo::setseason( const uint16_t season_id, const optional<time_point_sec> start_at, const optional<time_point_sec> end_at, const optional<time_point_sec> submission_start_at, const optional<time_point_sec> submission_end_at, const optional<string> description, const optional<double> match_value )
{
    require_auth( get_self() );

    check( season_id > 0, "pomelo::setseason: [season_id] must be positive");

    pomelo::seasons_table seasons( get_self(), get_self().value );
    const auto itr = seasons.find( season_id );

    const auto insert = [&]( auto & row ) {
        row.season_id = season_id;
        if(description) row.description = *description;
        if(match_value) row.match_value = *match_value;
        if(start_at) row.start_at = *start_at;
        if(end_at) row.end_at = *end_at;
        if(submission_start_at) row.submission_start_at = *submission_start_at;
        if(submission_end_at) row.submission_end_at = *submission_end_at;
        row.updated_at = current_time_point();
        if( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();

        // validate times
        check( row.end_at > row.start_at, "pomelo::setseason: [end_at] must be after [start_at]");
        check( row.end_at.sec_since_epoch() - row.start_at.sec_since_epoch() >= DAY * 1, "pomelo::setseason: active minimum period must be at least 1 day");
        check( row.submission_end_at > row.submission_start_at, "pomelo::setseason: [submission_end_at] must be after [submission_start_at]");
        check( row.submission_end_at.sec_since_epoch() - row.submission_start_at.sec_since_epoch() >= DAY * 1, "pomelo::setseason: submission minimum period must be at least 1 day");
        check( row.submission_start_at <= row.start_at, "pomelo::setseason: [submission_start_at] must be before [start_at]");
        check( row.submission_end_at <= row.end_at, "pomelo::setseason: [submission_end_at] must be before [end_at]");
    };

    // erase if all parameters are undefined
    if( !description && !match_value && !start_at && !end_at) seasons.erase(itr);
    else if ( itr == seasons.end() ) seasons.emplace( get_self(), insert );
    else seasons.modify( itr, get_self(), insert );
}


// @admin
[[eosio::action]]
void pomelo::deltoken( const symbol_code symcode )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );
    const auto & itr = tokens.get( symcode.raw(), "pomelo::deltoken: [symcode] token not found" );
    tokens.erase( itr );
}

// @user
[[eosio::action]]
void pomelo::setproject( const name author_id, const name project_type, const name project_id, const name funding_account, const set<symbol_code> accepted_tokens )
{
    eosn::login::require_auth_user_id( author_id, get_globals().login_contract );

    // tables
    pomelo::grants_table grants( get_self(), get_self().value );
    pomelo::bounties_table bounties( get_self(), get_self().value );

    // set project
    if ( project_type == "grant"_n ) {
        set_project( grants, "grant"_n, project_id, author_id, funding_account, accepted_tokens );
        check( bounties.find(project_id.value) == bounties.end(), "pomelo::setproject: Bounty with [project_id] already exists" );
    }
    else if ( project_type == "bounty"_n ) {
        set_project( bounties, "bounty"_n, project_id, author_id, funding_account, accepted_tokens );
        check( grants.find(project_id.value) == grants.end(), "pomelo::setproject: Grant with [project_id] already exists" );
    }
    else check( false, "pomelo::setproject: invalid [project_type]");
}

// @user
[[eosio::action]]
void pomelo::setgrant( const name author_id, const name project_id, const name funding_account, const set<symbol_code> accepted_tokens )
{
    setproject( author_id, "grant"_n, project_id, funding_account, accepted_tokens);
}

// @user
[[eosio::action]]
void pomelo::joinround( const name grant_id, const uint16_t round_id )
{
    // authenticate user
    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::joinround: [grant_id] does not exist" );
    eosn::login::require_auth_user_id( grant.author_user_id, get_globals().login_contract );
    const auto now = current_time_point().sec_since_epoch();

    // join round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    pomelo::seasons_table seasons( get_self(), get_self().value );
    const auto & round = rounds.get( round_id, "pomelo::joinround: [round_id] does not exist");
    const auto & season = seasons.get( round.season_id, "pomelo::joinround: [round.season_id] does not exist");
    check( get_index(round.grant_ids, grant_id ) == -1, "pomelo::joinround: grant already exists in this round");
    check( season.submission_start_at.sec_since_epoch() <= now, "pomelo::joinround: [round_id] submission period has not started");
    check( now <= season.submission_end_at.sec_since_epoch(), "pomelo::joinround: [round_id] submission period has ended");

    for(const auto ex_round_id: season.round_ids){
        const auto round = rounds.get(ex_round_id, "pomelo::joinround: bad existing round in a season");
        check( get_index(round.grant_ids, grant_id ) == -1, "pomelo::joinround: grant already exists in this season");
    }

    rounds.modify( round, get_self(), [&]( auto & row ) {
        row.grant_ids.push_back(grant_id);
        row.updated_at = current_time_point();
    });

}

// @admin
[[eosio::action]]
void pomelo::unjoinround( const name grant_id, const uint16_t round_id )
{
    require_auth( get_self() );

    pomelo::rounds_table _rounds( get_self(), get_self().value );
    const auto round_itr = _rounds.find( round_id );
    check( round_itr != _rounds.end(),  "pomelo::unjoinround: [round_id] does not exist" );
    check( get_index(round_itr->grant_ids, grant_id ) != -1, "pomelo::unjoinround: grant does not exist in this round");

    // remove from match table
    pomelo::match_table _match( get_self(), round_id );
    const auto match_itr = _match.find( grant_id.value );
    double sum_value = 0, sum_boost = 0, sum_square = 0;
    if(match_itr != _match.end()){
        // recalculate matchings for this round
        _match.erase( match_itr );
        for (const auto& grant: _match) {
            sum_value += grant.sum_value;
            sum_boost += grant.sum_boost;
            sum_square += grant.square;
        }
    }

    // remove from rounds table
    _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.grant_ids = remove_element(row.grant_ids, grant_id);
        row.sum_value = sum_value;
        row.sum_boost = sum_boost;
        row.sum_square = sum_square;
        row.updated_at = current_time_point();
        // no easy way to update row.accepted_tokens since they were already converted to values
    });

    // update users table by removing all donations to this project
    pomelo::users_table _users( get_self(), round_id );
    auto users_itr = _users.begin();
    while (users_itr != _users.end()) {
        int index = get_index(users_itr->contributions, grant_id);
        if (index != -1) {
            if (users_itr->contributions.size() == 1) {
                users_itr = _users.erase(users_itr);
                continue;
            }
            _users.modify( users_itr, get_self(), [&]( auto & row ) {
                const auto value = users_itr->contributions[index].value;
                const auto multiplier = users_itr->boost / users_itr->value;
                const auto donated = value / (multiplier + 1);      // edge case: change social between donations
                const auto boost = donated * multiplier;
                row.contributions.erase(row.contributions.begin() + index);
                row.value -= donated;
                row.boost -= boost;
                row.updated_at = current_time_point();
            });
        }
        ++users_itr;
    }
}

// @admin
[[eosio::action]]
void pomelo::setstate( const name project_id, const name status )
{
    require_auth( get_self() );
    check( STATUS_TYPES.count(status), "pomelo::setstate: invalid [status]" );

    pomelo::grants_table _grants( get_self(), get_self().value );
    pomelo::bounties_table _bounties( get_self(), get_self().value );

    const auto modify = [&]( auto & row ) {
        check( row.status != status, "pomelo::setstate: status was not modified");
        row.status = status;
        row.updated_at = current_time_point();
    };

    if( auto it = _grants.find( project_id.value ); it != _grants.end())
        _grants.modify( it, get_self(), modify );
    else if ( auto it = _bounties.find( project_id.value ); it != _bounties.end())
        _bounties.modify( it, get_self(), modify );
    else check( false, "pomelo::setstate: [project_id] does not exist");

}

// @admin
[[eosio::action]]
void pomelo::setround(  const uint16_t round_id,
                        const uint16_t season_id,
                        const optional<string> description,
                        const optional<double> match_value )
{
    require_auth( get_self() );

    check( season_id > 0,  "pomelo::setround: [season_id] must exist");
    pomelo::rounds_table rounds( get_self(), get_self().value );
    pomelo::seasons_table seasons( get_self(), get_self().value );

    // add round to season
    const auto & season = seasons.get(season_id, "pomelo::setround: [season_id] does not exists");

    if ( get_index(season.round_ids, round_id) == -1 ) {
        seasons.modify( season, get_self(), [&]( auto & row ) {
            row.round_ids.push_back(round_id);
            // check( row.start_at >= current_time_point(), "pomelo::setround: [current_time_point] must be before [start_at]");
        });
    }

    const auto insert = [&]( auto & row ) {
        row.round_id = round_id;
        check(row.season_id == 0 || row.season_id == season_id, "pomelo::setround: [round_id] already exists in another season");
        row.season_id = season_id;
        if(description) row.description = *description;
        if(match_value) row.match_value = *match_value;
        row.updated_at = current_time_point();
        if( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
    };

    const auto itr = rounds.find( round_id );
    if ( itr == rounds.end() ) rounds.emplace( get_self(), insert );
    else rounds.modify( itr, get_self(), insert );
}

// @admin
[[eosio::action]]
void pomelo::setconfig( const optional<uint16_t> season_id, const optional<uint64_t> grant_fee, const optional<uint64_t> bounty_fee, const optional<name> login_contract, const optional<name> fee_account )
{
    require_auth( get_self() );

    pomelo::globals_table _globals( get_self(), get_self().value );
    auto globals = _globals.get_or_default();

    if ( season_id ) globals.season_id = *season_id;
    if ( grant_fee ) globals.grant_fee = *grant_fee;
    if ( bounty_fee ) globals.bounty_fee = *bounty_fee;
    if ( login_contract ) globals.login_contract = *login_contract;
    if ( fee_account ) globals.fee_account = *fee_account;
    _globals.set( globals, get_self() );
}

// @admin
[[eosio::action]]
void pomelo::cleartable( const name table_name, const optional<uint16_t> round_id, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = *max_rows == 0 ? -1 : *max_rows;
    const uint64_t scope = round_id ? *round_id : get_self().value;

    // tables
    pomelo::transfers_table transfers( get_self(), scope );
    pomelo::rounds_table rounds( get_self(), scope );
    pomelo::match_table match( get_self(), scope );
    pomelo::bounties_table bounties( get_self(), scope );
    pomelo::grants_table grants( get_self(), scope );
    pomelo::globals_table globals( get_self(), scope );
    pomelo::tokens_table tokens( get_self(), scope );
    pomelo::status_table status( get_self(), scope );
    pomelo::users_table users( get_self(), scope );
    pomelo::seasons_table seasons( get_self(), scope );

    if (table_name == "transfers"_n) clear_table( transfers, rows_to_clear );
    else if (table_name == "rounds"_n) clear_table( rounds, rows_to_clear );
    else if (table_name == "match"_n) clear_table( match, rows_to_clear );
    else if (table_name == "bounties"_n) clear_table( bounties, rows_to_clear );
    else if (table_name == "grants"_n) clear_table( grants, rows_to_clear );
    else if (table_name == "tokens"_n) clear_table( tokens, rows_to_clear );
    else if (table_name == "users"_n) clear_table( users, rows_to_clear );
    else if (table_name == "seasons"_n) clear_table( seasons, rows_to_clear );
    else if (table_name == "globals"_n) globals.remove();
    else if (table_name == "status"_n) status.remove();
    else check(false, "pomelo::cleartable: [table_name] unknown table to clear" );
}

// @admin
[[eosio::action]]
void pomelo::removeuser( const name user_id, const uint16_t round_id )
{
    require_auth( get_self() );

    // remove from users table
    pomelo::users_table _users( get_self(), round_id );
    auto user_itr = _users.find( user_id.value );
    check(user_itr != _users.end(), "pomelo::removeuser: no donations from [user_id] during [round_id]" );
    const auto user = *user_itr;
    _users.erase(user_itr);

    // update match table
    pomelo::match_table _match( get_self(), round_id );
    for (const auto grant: user.contributions) {
        const auto match_itr = _match.find( grant.id.value );
        if (match_itr->total_users == 1) {
            _match.erase(match_itr);
            continue;
        }
        _match.modify( match_itr, get_self(), [&]( auto & row ) {
            const auto donated = grant.value / (1 + user.multiplier);
            row.total_users--;
            row.sum_value -= donated;
            row.sum_boost -= donated * user.multiplier;
            row.sum_sqrt -= sqrt( grant.value );
            row.square = row.sum_sqrt * row.sum_sqrt;
            row.updated_at = current_time_point();
        });

    }

    // remove user_id and update sums in rounds table
    double sum_value = 0, sum_boost = 0, sum_square = 0;
    for(const auto& grant: _match) {
        sum_value += grant.sum_value;
        sum_boost += grant.sum_boost;
        sum_square += grant.square;
    }

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  "pomelo::removeuser: [round_id] does not exist" );
    check( get_index(round_itr->user_ids, user_id ) != -1, "pomelo::removeuser: grant does not exist in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.user_ids = remove_element(row.user_ids, user_id);
        row.sum_value = sum_value;
        row.sum_boost = sum_boost;
        row.sum_square = sum_square;
        row.updated_at = current_time_point();
        // no easy way to update row.accepted_tokens since they were already converted to values
    });
}

[[eosio::action]]
void pomelo::collapse(set<name> user_ids, name user_id, uint16_t round_id)
{
    require_auth( get_self() );
    check(user_ids.count(user_id) == 0, "pomelo::collapse: [user_ids] cannot contain [user_id] itself" );

    pomelo::users_table _users( get_self(), round_id );
    pomelo::match_table _match( get_self(), round_id );
    pomelo::rounds_table _rounds( get_self(), get_self().value );
    auto user = _users.get( user_id.value, "pomelo::collapse: no donations from [user_id] during [round_id]" );
    auto round_itr = _rounds.find(round_id);
    check( round_itr != _rounds.end(),  "pomelo::collapse: [round_id] does not exist" );
    vector<name> round_users = round_itr->user_ids;

    for(const auto& user_to_erase: user_ids) {
        auto erase_itr = _users.find(user_to_erase.value);
        check(erase_itr != _users.end(), "pomelo::collapse: [user_ids] contains user_ids that didn't take part in [round_id]: " + (*user_ids.begin()).to_string() );
        for(const auto c: erase_itr->contributions){
            const auto donated = c.value / (erase_itr->multiplier + 1);
            const auto old_contribution = user.contributions[get_index(user.contributions, c.id)].value;
            user.contributions[get_index(user.contributions, c.id)].value += donated * (user.multiplier + 1);
            auto match_itr = _match.find(c.id.value);
            _match.modify( match_itr, get_self(), [&]( auto & row ) {
                row.total_users--;
                row.sum_boost += donated * (user.multiplier - erase_itr->multiplier);
                row.sum_sqrt += sqrt(user.contributions[get_index(user.contributions, c.id)].value) - sqrt(donated * (erase_itr->multiplier + 1)) - sqrt(old_contribution);
                row.square = row.sum_sqrt * row.sum_sqrt;
                row.updated_at = current_time_point();
            });
        }
        user.value += erase_itr->value;
        user.boost += erase_itr->value * user.multiplier;
        round_users = remove_element(round_users, user_to_erase);
        _users.erase(erase_itr);
    }
    const auto user_itr = _users.find( user_id.value );
    _users.modify( user_itr, get_self(), [&]( auto & row ) {
        row.contributions = user.contributions;
        row.value = user.value;
        row.boost = user.boost;
        row.updated_at = current_time_point();
    });

    // recalculate all matched amounts for this round
    double round_sum_boost = 0, round_sum_square = 0;
    for(const auto& grant: _match){
        round_sum_boost += grant.sum_boost;
        round_sum_square += grant.square;
    }
    _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.sum_boost = round_sum_boost;
        row.sum_square = round_sum_square;
        row.user_ids = round_users;
        row.updated_at = current_time_point();
    });
}

void pomelo::transfer( const name from, const name to, const extended_asset value, const string memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}