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