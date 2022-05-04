#!/usr/bin/env bats

@test "uninitialized contract" {
  run cleos transfer user1 work.pomelo "1000.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::on_transfer: contract is under maintenance" ]] || false
}

@test "init globals" {

  run cleos push action work.pomelo setconfig '[500, "login.eosn", "fee.pomelo"]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo globals | jq -r '.rows[0].season_id')
  [ $result = "0" ]
}

@test "set token" {

  run cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  run cleos push action work.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  run cleos push action work.pomelo token '["4,PLAY", "play.pomelo", 10000, 1]' -p work.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table work.pomelo work.pomelo tokens | jq -r '.rows | length')
  [ $result = "3" ]

}
