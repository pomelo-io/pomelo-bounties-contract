#!/usr/bin/env bats


@test "fund non-existing bounty" {
  run cleos transfer funder1 work.pomelo "1000.0000 EOS" "badbounty,funder1.eosn"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "apply for non-existing bounty" {
  run cleos push action work.pomelo apply '[badbounty, hunter1.eosn]' -p hunter1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "create unauthorized bounty" {
  run cleos push action work.pomelo createbounty '[author1.eosn, bounty1, "EOS", null]' -p author2.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "create wrong type bounty" {
  run cleos push action work.pomelo createbounty '[author1.eosn, bounty1, "EOS", badtype]' -p author1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "unknown [bounty_type]" ]] || false
}

@test "create wrong token bounty" {
  run cleos push action work.pomelo createbounty '[author1.eosn, bounty1, "SYM", null]' -p author1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[symcode] not supported" ]] || false
}

@test "create bounty by non-existing EOSN user" {
  run cleos push action work.pomelo createbounty '[author1, bounty1, "EOS", null]' -p author1
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]] || false
}

@test "successfully create bounty" {
  run cleos push action work.pomelo createbounty '[author1.eosn, bounty1, "EOS", null]' -p author1.eosn
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].bounty_id')
  [ $result = "bounty1" ]
}

@test "send token with invalid memo" {
  run cleos transfer funder1 work.pomelo "100.0000 EOS" ""
  [ $status -eq 1 ]
  [[ "$output" =~ "invalid transfer memo" ]] || false
}

@test "fund non-existing bounty" {
  run cleos transfer funder1 work.pomelo "100.0000 EOS" "bounty11"
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "fund bounty with < min_amount" {
  run cleos transfer funder1 work.pomelo "1.0000 EOS" "bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "[quantity=1.0000 EOS] is less than [tokens.min_amount=20000]" ]] || false
}

@test "fund bounty with wrong token" {
  run cleos transfer funder1 work.pomelo "1.0000 USDT" "bounty1" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "quantity extended symbol not allowed" ]] || false
}

@test "fund bounty with fake token" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1" --contract fake.token
  [ $status -eq 1 ]
  [[ "$output" =~ "[token.contract] is invalid" ]] || false
}

@test "fund bounty with non-existing token" {
  run cleos transfer funder1 work.pomelo "2.0000 PLAY" "bounty1" --contract eosio.token
  [ $status -eq 1 ]
  [[ "$output" =~ "[symcode] not supported" ]] || false
}

@test "fund bounty with non-eosn user" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1,user.noeosn"
  [ $status -eq 1 ]
  [[ "$output" =~ "[funder_user_id] must be EOSN account" ]] || false
}

@test "change state to some random state" {
  run cleos push action work.pomelo setstate '[bounty1, badstate]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "invalid [status]" ]] || false
}

@test "change state with no auth" {
  run cleos push action work.pomelo setstate '[bounty1, badstate]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "missing authority of work.pomelo" ]] || false

  run cleos push action work.pomelo setstate '[bounty1, badstate]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "missing authority of work.pomelo" ]] || false
}

@test "change state of non-existing bounty" {
  run cleos push action work.pomelo setstate '[bounty11, done]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exist" ]] || false
}

@test "change state to the same state" {
  run cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "status was not modified" ]] || false
}

@test "successfully change state" {
  run cleos push action work.pomelo setstate '[bounty1, done]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "done" ]
}

@test "fund already completed bounty" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty not available for funding" ]] || false
}

@test "change state back to pending" {
  run cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "pending" ]
}

