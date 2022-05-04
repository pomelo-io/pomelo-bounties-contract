#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create account
cleos create account eosio app.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fee.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio login.eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio oracle.defi EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio play.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio tethertether EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fake.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjbounty1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user11 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user12 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user13 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user.noeosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# contract
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract tethertether ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract fake.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract login.eosn ./include/eosn.login login.eosn.wasm login.eosn.abi
cleos set contract oracle.defi ./include/oracle.defi oracle.defi.wasm oracle.defi.abi
cleos set contract play.pomelo ./include/pomelo.play play.pomelo.wasm play.pomelo.abi
cleos set contract app.pomelo . app.pomelo.wasm app.pomelo.abi

# @eosio.code permission
cleos set account permission app.pomelo active --add-code
cleos set account permission login.eosn active --add-code
cleos set account permission play.pomelo active login.eosn --add-code
cleos set account permission eosn active login.eosn --add-code

# create tokens
cleos push action eosio.token create '["eosio", "10000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 EOS", "init"]' -p eosio
cleos push action tethertether create '["eosio", "100000000.0000 USDT"]' -p tethertether
cleos push action tethertether issue '["eosio", "10000000.0000 USDT", "init"]' -p eosio
cleos push action play.pomelo create '["play.pomelo", "10000000000.0000 PLAY"]' -p play.pomelo
cleos push action play.pomelo issue '["play.pomelo", "1.0000 PLAY", "init"]' -p play.pomelo

# create fake tokens
cleos push action fake.token create '["eosio", "100000000.0000 EOS"]' -p fake.token
cleos push action fake.token issue '["eosio", "5000000.0000 EOS", "init"]' -p eosio

# transfer tokens
cleos transfer eosio user1 "1000000.0000 EOS" ""
cleos transfer eosio user1 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user1 "1000000.0000 EOS" "" --contract fake.token
cleos transfer eosio user2 "1000000.0000 EOS" ""
cleos transfer eosio user2 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user3 "1000000.0000 EOS" ""
cleos transfer eosio user3 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user4 "1000000.0000 EOS" ""
cleos transfer eosio user4 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user5 "1000000.0000 EOS" ""
cleos transfer eosio user5 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user11 "1000000.0000 EOS" ""
cleos transfer eosio user11 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user12 "1000000.0000 EOS" ""
cleos transfer eosio user12 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user13 "1000000.0000 EOS" ""
cleos transfer eosio user13 "1000000.0000 USDT" "" --contract tethertether
cleos transfer eosio user.noeosn "1000000.0000 EOS" ""

# set price in defibox contract
cleos push action oracle.defi setprice '[1, ["4,EOS", "eosio.token"], 4, 100000]' -p oracle.defi

# set socials weights
cleos push action login.eosn configsocial '["sms", 25]' -p login.eosn
cleos push action login.eosn configsocial '["facebook", 25]' -p login.eosn
cleos push action login.eosn configsocial '["twitter", 25]' -p login.eosn
cleos push action login.eosn configsocial '["github", 25]' -p login.eosn
cleos push action login.eosn configsocial '["passport", 25]' -p login.eosn
cleos push action login.eosn configsocial '["eden", 50]' -p login.eosn