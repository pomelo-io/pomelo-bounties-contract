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

  run cleos push action login.eosn link '["author1.eosn", "author1", "SIG_K1_JutbxGsNuYmbFQ81fRvspRTa1vvJF7eVoKNQsmNf8mKmKoC4Q7Sk15AoqDrC8MDYpNLrATP8owLSFAui7nZB5tvARtLNcW"]' -p author1
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["hunter1.eosn", "hunter1", "SIG_K1_Kb1P15WPNqJJBqGy2jAX4umWGUcYkfwFhZeaXQTUXDnc7RyoQn6f9STfKaNSiXEjLsMSUnnHGwbbeU2Bc6dQFYkxQuEx8d"]' -p hunter1
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["hunter2.eosn", "hunter2", "SIG_K1_KjHmbiHTDLcDtfH55mFFDQTcgFpaPu1x8itXE5HLrrwpUhuQuNn96osLbLgdQrTYXfNWpL31oE4arGhKgP9w3P8jZNjJ8u"]' -p hunter2
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["hunter3.eosn", "hunter3", "SIG_K1_JyirGTtBS38YSLfqH6i5uPTRT7C5YKpDGpVLv4oqms3kef9nWGkkvUnvCNckP5cuNChpRmAKCM1DgfNcxG6pxBFV19e8NT"]' -p hunter3
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
