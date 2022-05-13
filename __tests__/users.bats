#!/usr/bin/env bats

@test "create users" {

  run cleos push action login.eosn create '["author1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].user_id')
  [ $result = "author1.eosn" ]

  run cleos push action login.eosn create '["author2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["funder1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["funder2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["funder3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["hunter1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["hunter2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["hunter3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  result=$(cleos get table login.eosn login.eosn users -l 20 | jq -r '.rows | length')
  [ $result = "8" ]

  # no account linking required ?
  # run cleos push action login.eosn link '["prjman1.eosn", "prjman1", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p prjman1
  # [ $status -eq 0 ]
  # result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].accounts[0]')
  # [ $result = "prjman1" ]
  # result=$(cleos get table play.pomelo prjman1 accounts  | jq -r '.rows | length')
  # [ $result = "1" ]

}
