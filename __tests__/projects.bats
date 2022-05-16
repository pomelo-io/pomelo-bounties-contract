#!/usr/bin/env bats

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

@test "create already existing bounty" {
  run cleos push action work.pomelo createbounty '[author2.eosn, bounty1, "USDT", null]' -p author2.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] already exists" ]] || false
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
  run cleos transfer funder1 work.pomelo "5.0000 EOS" "bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty not available for funding" ]] || false
}

@test "change state back to pending" {
  run cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "pending" ]
}


@test "fund the bounty by author1" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].amount.quantity + " " + .rows[0].fee.quantity')
  [ "$result" = "1.9000 EOS 0.1000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders[0].key + " " + .rows[0].funders[0].value')
  [ "$result" = "author1.eosn 2.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo transfers | jq -r '.rows | length')
  [ $result = "1" ]
}

@test "fund the bounty by funder1" {
  run cleos transfer funder1 work.pomelo "5.0000 EOS" "bounty1,funder1.eosn"
  [ $status -eq 0 ]
  run cleos transfer funder1 work.pomelo "3.0000 EOS" "bounty1,funder1.eosn"
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].amount.quantity + " " + .rows[0].fee.quantity')
  [ "$result" = "9.5000 EOS 0.5000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders[0].key + " " + .rows[0].funders[0].value')
  [ "$result" = "author1.eosn 2.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders[1].key + " " + .rows[0].funders[1].value')
  [ "$result" = "funder1.eosn 8.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders | length')
  [ $result = "2" ]
  result=$(cleos get table work.pomelo work.pomelo transfers | jq -r '.rows | length')
  [ $result = "3" ]
}

@test "apply for bounty in wrong state" {
  run cleos push action work.pomelo apply '[bounty1, hunter1.eosn]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "change state to open" {
  run cleos push action work.pomelo setstate '[bounty1, open]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "open" ]
}

@test "fund bounty in open state" {
  run cleos transfer funder1 work.pomelo "3.0000 EOS" "bounty1"
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].amount.quantity + " " + .rows[0].fee.quantity')
  [ "$result" = "12.3500 EOS 0.6500 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders[0].key + " " + .rows[0].funders[0].value')
  [ "$result" = "author1.eosn 5.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders | length')
  [ $result = "2" ]
  result=$(cleos get table work.pomelo work.pomelo transfers | jq -r '.rows | length')
  [ $result = "4" ]
}

@test "apply for bounty with non-existing eosn user" {
  run cleos push action work.pomelo apply '[bounty1, user.noeosn]' -p user.noeosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]] || false
}

@test "apply for non-existing bounty" {
  run cleos push action work.pomelo apply '[bounty11, hunter1.eosn]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "apply for bounty for wrong user" {
  run cleos push action work.pomelo apply '[bounty1, hunter1.eosn]' -p hunter2.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "apply for bounty" {
  run cleos push action work.pomelo apply '[bounty1, hunter1.eosn]' -p hunter1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].applicant_user_ids[0]')
  [ "$result" = "hunter1.eosn" ]
}

@test "apply for bounty for the second time" {
  run cleos push action work.pomelo apply '[bounty1, hunter1.eosn]' -p hunter1.eosn -f
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is already applicant" ]] || false
}

@test "approve wrong bounty" {
  run cleos push action work.pomelo approve '[bounty11, hunter1.eosn]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "approve wrong hunter" {
  run cleos push action work.pomelo approve '[bounty1, hunter2.eosn]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[applicant_user_id] did not apply" ]] || false
}

@test "approve bounty successfully" {
  run cleos push action work.pomelo approve '[bounty1, hunter1.eosn]' -p author1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].approved_user_id')
  [ "$result" = "hunter1.eosn" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "started" ]
}