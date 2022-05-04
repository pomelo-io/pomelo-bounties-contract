#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# enable tokens
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo

# create matching round and start it
cleos push action app.pomelo setconfig '[1, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setproject '["myaccount", "grant", "mygrant", "myaccount", ["EOS", "USDT"]]' -p myaccount
cleos push action app.pomelo enable '["mygrant", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["mygrant", 1]' -p myaccount
