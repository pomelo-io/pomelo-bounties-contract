#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build

echo "compiling... [work.pomelo]"
if [[ $* == --debug ]]
then
    echo "DEBUG mode enabled"
    blanc++ work.pomelo.cpp -I include -D=DEBUG
else
    blanc++ work.pomelo.cpp -I include
fi

# additional builds
if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    echo "compiling... [eosio.token]"
    blanc++ ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/eosn.login/login.eosn.wasm" ]; then
    echo "compiling... [login.eosn]"
    blanc++ ./include/eosn.login/login.eosn.cpp -I include -o include/eosn.login/login.eosn.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/oracle.defi/oracle.defi.wasm" ]; then
    echo "compiling... [oracle.defi]"
    blanc++ ./include/oracle.defi/oracle.defi.cpp -I include -o include/oracle.defi/oracle.defi.wasm --no-missing-ricardian-clause
fi
