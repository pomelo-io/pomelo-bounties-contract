# 🍈 Pomelo for Work (Bounties) - EOSIO Smart Contract

## Security Audits

- N/A

## Usage

### `@author`

"https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"

```bash
# before creating bounty, must link EOS account with EOSN login
cleos push action login.eosn link '["author.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

# create bounty
cleos push action work.pomelo createbounty '["author.eosn", "USDT", [{"key": "url", "value": ["string", "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"]} ]]' -p author.eosn

# OPTIONAL. update/modify bounty metadata
cleos push action work.pomelo setbounty '["author.eosn", 123, [{"key": "url", "value": ["string", "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"]} ]]' -p author.eosn

# fund bounty
cleos transfer myaccount work.pomelo "100.0000 USDT" "author.eosn:123" --contract tethertether

# make bounty public
cleos push action work.pomelo setstate '[author.eosn, 123, "published"]' -p work.pomelo

# author select hunter for bounty
cleos push action work.pomelo approve '[author.eosn, 123, hunter.eosn]' -p author.eosn

# OPTION 1. author release funds for completed bounty
cleos push action work.pomelo release '[author.eosn, 123]' -p author.eosn

# OPTION 2. Author denies completed bounty
# Hunter has 72 hours to respond to deny request
# Bounty is set to un-completed state after 72 hours of no response (funds can be withdrawn by author).
cleos push action work.pomelo deny '[author.eosn, 123]' -p author.eosn

# OPTION 3. funds auto-released after 72 hours
# /* no action */

# author withdraws funds from no-completed bounty (using EOSN login linked EOS account)
cleos push action work.pomelo withdraw '[author.eosn, 123]' -p myaccount
```

### `@user`

```bash
# before applying, hunter must link their EOS account with EOSN login
cleos push action login.eosn link '["hunter.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

# hunter apply to bounty
cleos push action work.pomelo apply '[author.eosn, 123, hunter.eosn]' -p hunter.eosn

# hunter completes work (funds are auto-released after 72 hours of no response from author)
cleos push action work.pomelo complete '[author.eosn, 123]' -p hunter.eosn

# hunter claims bounty funds (using EOSN login linked EOS account)
cleos push action work.pomelo claim '[author.eosn, 123]' -p myaccount
```

### `@admin`

```bash
# configure app
cleos push action work.pomelo setconfig '[500, "login.eosn", "fee.pomelo", [{ "name": "url", "type": "string" }]]' -p work.pomelo
cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
cleos push action work.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p work.pomelo
```

## Dependencies

- [eosn.login](https://github.com/pomelo-io/eosn.login)
- [sx.utils](https://github.com/stableex/sx.utils)
- [eosio.token](https://github.com/EOSIO/eosio.contracts)

## Testing

```bash
# build contract
$ ./scripts/build.sh

# restart node, create EOSIO users, deploy contracts, issue tokens
$ ./scripts/restart

# run tests
$ ./test.sh
```

## Definitions

### Roles

| `role`        | `description`                 |
|---------------|-------------------------------|
| Backend       | Pomelo Backend                |
| Admin         | Pomelo Admins                 |
| Owners        | Grant/Bounty Owners           |
| SC            | Smart Contract                |

## Table of Content

- [TABLE `configs`](#table-configs)
- [TABLE `bounties`](#tables-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `tokens`](#table-tokens)
- [ACTION `createbounty`](#action-createbounty)
- [ACTION `setbounty`](#action-setbounty)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `token`](#action-token)
- [ACTION `setstate`](#action-setstate)
- [ACTION `approve`](#action-approve)
- [ACTION `release`](#action-release)
- [ACTION `deny`](#action-deny)
- [ACTION `apply`](#action-apply)
- [ACTION `complete`](#action-complete)
- [ACTION `claim`](#action-claim)