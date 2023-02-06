#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build

if command -v blanc++ &> /dev/null
then
  CDT="blanc++"
elif command -v cdt-cpp &> /dev/null
then
  CDT="cdt-cpp"
else
  echo "Error: neither blanc++ nor cdt-cpp was found on this system."
  exit 1
fi

echo "Compiling with $CDT"

echo "compiling... [work.pomelo]"
if [[ $* == --debug ]]
then
    echo "DEBUG mode enabled"
    $CDT work.pomelo.cpp -I include -D=DEBUG
else
    $CDT work.pomelo.cpp -I include
fi

# additional builds
if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    echo "compiling... [eosio.token]"
    $CDT ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/eosn.login/login.eosn.wasm" ]; then
    echo "compiling... [login.eosn]"
    $CDT ./include/eosn.login/login.eosn.cpp -I include -o include/eosn.login/login.eosn.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/oracle.defi/oracle.defi.wasm" ]; then
    echo "compiling... [oracle.defi]"
    $CDT ./include/oracle.defi/oracle.defi.cpp -I include -o include/oracle.defi/oracle.defi.wasm --no-missing-ricardian-clause
fi
