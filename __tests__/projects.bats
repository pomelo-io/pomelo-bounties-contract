#!/usr/bin/env bats

@test "create unauthorized bounty" {
  run cleos push action work.pomelo create '[author1.eosn, bounty1, "EOS", null]' -p author2.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "create wrong type bounty" {
  run cleos push action work.pomelo create '[author1.eosn, bounty1, "EOS", badtype]' -p author1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "unknown [bounty_type]" ]] || false
}

@test "create wrong token bounty" {
  run cleos push action work.pomelo create '[author1.eosn, bounty1, "SYM", null]' -p author1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[symcode] not supported" ]] || false
}

@test "create bounty by non-existing EOSN user" {
  run cleos push action work.pomelo create '[author1, bounty1, "EOS", null]' -p author1
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]] || false
}

@test "successfully create bounty" {
  run cleos push action work.pomelo create '[author1.eosn, bounty1, "EOS", null]' -p author1.eosn
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].bounty_id')
  [ $result = "bounty1" ]
}

@test "create already existing bounty" {
  run cleos push action work.pomelo create '[author2.eosn, bounty1, "USDT", null]' -p author2.eosn
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

@test "change to invalid states" {
  run cleos push action work.pomelo setstate '[bounty1, done]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty needs to be claimed first" ]] || false

  run cleos push action work.pomelo setstate '[bounty1, released]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty needs to be completed first" ]] || false

  run cleos push action work.pomelo setstate '[bounty1, submitted]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty needs to be completed first" ]] || false

  run cleos push action work.pomelo setstate '[bounty1, started]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty needs to have a hunter" ]] || false

  run cleos push action work.pomelo setstate '[bounty1, open]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty needs to be funded" ]] || false
}

@test "publish unfunded bounty" {
  run cleos push action work.pomelo publish '[bounty1]' -p work.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty must be funded to publish" ]] || false
}

@test "withdraw non-funded bounty" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "nothing to withdraw" ]] || false
}

@test "fund the bounty by author1" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1"
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].amount.quantity + " " + .rows[0].fee.quantity')
  [ "$result" = "1.9000 EOS 0.1000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders[0].key + " " + .rows[0].funders[0].value')
  [ "$result" = "author1.eosn 2.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo transfers | jq -r '.rows | length')
  [ $result = "1" ]

  pomelo_balance=$(cleos get currency balance eosio.token work.pomelo)
  [ "$pomelo_balance" = "2.0000 EOS" ]
}

@test "withdraw non-existing bounty" {
  run cleos push action work.pomelo withdraw '[bounty11, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "withdraw bounty to wrong account" {
  run cleos push action work.pomelo withdraw '[bounty1, funder1]' -p funder1
  [ $status -eq 1 ]
  [[ "$output" =~ "[receiver] must be linked with EOSN Login to [author_user_id]" ]] || false

  run cleos push action work.pomelo withdraw '[bounty1, author2]' -p author2
  [ $status -eq 1 ]
  [[ "$output" =~ "[receiver] must be linked with EOSN Login to [author_user_id]" ]] || false
}


@test "withdraw bounty" {

  author_balance=$(cleos get currency balance eosio.token author1)
  [ "$author_balance" = "1000000.0000 EOS" ]

  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].amount.quantity + " " + .rows[0].fee.quantity')
  [ "$result" = "0.0000 EOS 0.0000 EOS" ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].funders | length')
  [ $result = "0" ]

  author_balance=$(cleos get currency balance eosio.token author1)
  [ "$author_balance" = "1000002.0000 EOS" ]
  pomelo_balance=$(cleos get currency balance eosio.token work.pomelo)
  [ "$pomelo_balance" = "0.0000 EOS" ]
}


@test "fund the bounty by funder1" {
  run cleos transfer funder1 work.pomelo "2.0000 EOS" "bounty1"
  [ $status -eq 0 ]
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
  [ $result = "4" ]
}

@test "apply for bounty in wrong state" {
  run cleos push action work.pomelo apply '[bounty1, hunter1.eosn]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "publish bounty" {
  run cleos push action work.pomelo publish '[bounty1]' -p work.pomelo
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
  [ $result = "5" ]
}

@test "withdraw bounty in open state" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
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

@test "claim unapproved bounty" {
  run cleos push action work.pomelo claim '[bounty1, hunter1]' -p hunter1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "release unapproved bounty" {
  run cleos push action work.pomelo release '[bounty1]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "withdraw unapproved" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
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

@test "change state to pending and back to started" {
  run cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "pending" ]

  run cleos push action work.pomelo setstate '[bounty1, started]' -p work.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "started" ]
}

@test "claim incomplete bounty" {
  run cleos push action work.pomelo claim '[bounty1, hunter1]' -p hunter1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "release incomplete bounty" {
  run cleos push action work.pomelo release '[bounty1]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "withdraw incomplete bounty" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "complete non-existing bounty" {
  run cleos push action work.pomelo complete '[bounty11]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "complete bounty by wrong hunter" {
  run cleos push action work.pomelo complete '[bounty1]' -p hunter2.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "complete bounty successfully" {
  run cleos push action work.pomelo complete '[bounty1]' -p hunter1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "submitted" ]
}

@test "complete already completed bounty" {
  run cleos push action work.pomelo complete '[bounty1]' -p hunter1.eosn -f
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "claim unreleased bounty" {
  run cleos push action work.pomelo claim '[bounty1, hunter1]' -p hunter1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "withdraw completed bounty" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "deny non-existing bounty" {
  run cleos push action work.pomelo deny '[bounty11]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "forfeit submitted bounty" {
  run cleos push action work.pomelo forfeit '[bounty1]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "deny bounty successfully" {
  run cleos push action work.pomelo deny '[bounty1]' -p author1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "started" ]
}

@test "forfeit non-existing bounty" {
  run cleos push action work.pomelo forfeit '[bounty11]' -p hunter1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "forfeit unauthorized bounty" {
  run cleos push action work.pomelo forfeit '[bounty1]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "deny already denied bounty" {
  run cleos push action work.pomelo deny '[bounty1]' -p author1.eosn -f
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "complete denied bounty" {
  run cleos push action work.pomelo complete '[bounty1]' -p hunter1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "submitted" ]
}

@test "release non-existing bounty" {
  run cleos push action work.pomelo release '[bounty11]' -p author1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "release unauthorized bounty" {
  run cleos push action work.pomelo release '[bounty1]' -p author2.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] is not authorized" ]] || false
}

@test "release bounty successfully" {
  run cleos push action work.pomelo release '[bounty1]' -p author1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "released" ]
}

@test "withdraw released bounty" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "claim non-existing bounty" {
  run cleos push action work.pomelo claim '[bounty11, hunter1]' -p hunter1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty_id] does not exists" ]] || false
}

@test "claim bounty for non-linked account" {
  run cleos push action work.pomelo claim '[bounty1, hunter2]' -p hunter2
  [ $status -eq 1 ]
  [[ "$output" =~ "[receiver] must be linked with EOSN Login to [approved_user_id]" ]] || false
}

@test "claim bounty successfully" {
  fee_balance=$(cleos get currency balance eosio.token fee.pomelo)
  [ "$fee_balance" = "" ]
  hunter_balance=$(cleos get currency balance eosio.token hunter1)
  [ "$hunter_balance" = "" ]

  run cleos push action work.pomelo claim '[bounty1, hunter1]' -p hunter1
  [ $status -eq 0 ]
  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "done" ]

  fee_balance=$(cleos get currency balance eosio.token fee.pomelo)
  [ "$fee_balance" = "0.6500 EOS" ]
  hunter_balance=$(cleos get currency balance eosio.token hunter1)
  [ "$hunter_balance" = "12.3500 EOS" ]
}

@test "claim already claimed bounty" {
  run cleos push action work.pomelo setstate '[bounty1, submitted]' -p work.pomelo
  [ $status -eq 0 ]
  run cleos push action work.pomelo release '[bounty1]' -p author1.eosn -f
  [ $status -eq 0 ]
  run cleos push action work.pomelo claim '[bounty1, hunter1]' -p hunter1 -f
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty already claimed" ]] || false
  run cleos push action work.pomelo setstate '[bounty1, done]' -p work.pomelo
  [ $status -eq 0 ]
}

@test "fund already completed bounty" {
  run cleos transfer funder1 work.pomelo "5.0000 EOS" "bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "bounty not available for funding" ]] || false
}

@test "withdraw claimed bounty" {
  run cleos push action work.pomelo withdraw '[bounty1, author1]' -p author1
  [ $status -eq 1 ]
  [[ "$output" =~ "[bounty.status] must be" ]] || false
}

@test "create, fund, publish, apply, approve, complete, deny, forfeit and close bounty2" {
  run cleos push action work.pomelo create '[author2.eosn, bounty2, "USDT", traditional]' -p author2.eosn
  [ $status -eq 0 ]

  run cleos transfer funder2 work.pomelo "2.0000 USDT" "bounty2,funder2.eosn" --contract tethertether
  [ $status -eq 0 ]

  pomelo_balance=$(cleos get currency balance tethertether work.pomelo)
  [ "$pomelo_balance" = "2.0000 USDT" ]

  run cleos push action work.pomelo publish '[bounty2]' -p work.pomelo
  [ $status -eq 0 ]

  run cleos push action work.pomelo apply '[bounty2, hunter2.eosn]' -p hunter2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo approve '[bounty2, hunter2.eosn]' -p author2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo complete '[bounty2]' -p hunter2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo deny '[bounty2]' -p author2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo forfeit '[bounty2]' -p hunter2.eosn
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[1].status + .rows[1].approved_user_id')
  [ $result = "open" ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[1].applicant_user_ids | length')
  [ $result = "0" ]

  run cleos push action work.pomelo close '[bounty2]' -p author2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo withdraw '[bounty2, author2]' -p author2
  [ $status -eq 0 ]

  pomelo_balance=$(cleos get currency balance tethertether work.pomelo)
  [ "$pomelo_balance" = "0.0000 USDT" ]

  pomelo_balance=$(cleos get currency balance tethertether author2)
  [ "$pomelo_balance" = "1000002.0000 USDT" ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[1].status')
  [ $result = "closed" ]
}


@test "create and close bounty3 by admin" {
  run cleos push action work.pomelo create '[author2.eosn, bounty3, "EOS", null]' -p author2.eosn
  [ $status -eq 0 ]

  run cleos push action work.pomelo close '[bounty3]' -p work.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo bounties | jq -r '.rows[2].status')
  [ $result = "closed" ]
}
