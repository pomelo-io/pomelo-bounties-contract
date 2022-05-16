#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create account
cleos create account eosio work.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fee.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio login.eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio oracle.defi EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio tethertether EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fake.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio author1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio author2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio funder1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio funder2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio funder3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio hunter1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio hunter2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio hunter3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user.noeosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# contract
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract tethertether ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract fake.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract login.eosn ./include/eosn.login login.eosn.wasm login.eosn.abi
cleos set contract oracle.defi ./include/oracle.defi oracle.defi.wasm oracle.defi.abi
cleos set contract work.pomelo . work.pomelo.wasm work.pomelo.abi

# @eosio.code permission
cleos set account permission work.pomelo active --add-code
cleos set account permission login.eosn active --add-code
cleos set account permission eosn active login.eosn --add-code

# create tokens
cleos push action eosio.token create '["eosio", "10000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 EOS", "init"]' -p eosio
cleos push action tethertether create '["eosio", "100000000.0000 USDT"]' -p tethertether
cleos push action tethertether issue '["eosio", "10000000.0000 USDT", "init"]' -p eosio
cleos push action eosio.token create '["eosio", "10000000000.0000 PLAY"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 PLAY", "init"]' -p eosio

# create fake tokens
cleos push action fake.token create '["eosio", "100000000.0000 EOS"]' -p fake.token
cleos push action fake.token issue '["eosio", "5000000.0000 EOS", "init"]' -p eosio

# transfer tokens
cleos transfer eosio author1 "1000000.0000 EOS" ""
cleos transfer eosio author1 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio author2 "1000000.0000 EOS" ""
cleos transfer eosio author2 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio funder1 "1000000.0000 EOS" ""
cleos transfer eosio funder1 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio funder1 "1000000.0000 PLAY" ""
cleos transfer eosio funder1 "1000000.0000 EOS" "" --contract fake.token
cleos transfer eosio funder2 "1000000.0000 EOS" ""
cleos transfer eosio funder2 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio hunter1 "1000000.0000 EOS" ""
cleos transfer eosio hunter1 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio hunter2 "1000000.0000 EOS" ""
cleos transfer eosio hunter2 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio hunter3 "1000000.0000 EOS" ""
cleos transfer eosio hunter3 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user.noeosn "1000000.0000 EOS" ""

# set price in defibox contract
cleos push action oracle.defi setprice '[1, ["4,EOS", "eosio.token"], 4, 100000]' -p oracle.defi
