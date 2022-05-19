#!/usr/bin/env bats

@test "uninitialized contract" {
  run cleos transfer funder1 work.pomelo "1000.0000 EOS" "bounty"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "contract is under maintenance" ]] || false
}

@test "init globals" {

  run cleos push action work.pomelo setconfig '[testing, 500, "login.eosn", "fee.pomelo", [url]]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo configs | jq -r '.rows[0].fee')
  [ $result = "500" ]
}

@test "set token" {

  run cleos push action work.pomelo token '["4,EOS", "eosio.token", 20000, 1]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  run cleos push action work.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo tokens | jq -r '.rows | length')
  [ $result = "2" ]

}
