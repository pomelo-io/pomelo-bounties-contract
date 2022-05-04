#!/usr/bin/env bats

@test "cleos version check" {
  result=$(cleos get info | jq -r .server_version_string)
  echo $result
  [[ $result =~ "v2." ]] || false
}
