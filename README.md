# 🍈 Pomelo for Work (Bounties) - EOSIO Smart Contract

## Security Audits

- N/A

## Usage

### `@author`

```bash
# before creating bounty, must link EOS account with EOSN login
cleos push action login.eosn link '["author.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

# create bounty
cleos push action work.pomelo createbounty '[author.eosn, bounty1, "USDT", null]' -p author.eosn

# fund bounty
cleos transfer myaccount work.pomelo "100.0000 USDT" "bounty1,funder.eosn" --contract tethertether

# make bounty public
cleos push action work.pomelo setstate '[bounty1, published]' -p work.pomelo

# author select hunter for bounty
cleos push action work.pomelo approve '[bounty1, hunter.eosn]' -p author.eosn

# OPTION 1. Author releases funds to hunter when bounty is completed
# > Bounty state must be "completed"
cleos push action work.pomelo release '[bounty1]' -p author.eosn

# OPTION 2. Author denies completed bounty
# > Bounty state is reverted back to "progress"
cleos push action work.pomelo deny '[bounty1]' -p author.eosn

# OPTION 3. funds auto-released to hunter after 72 hours
# /* no action */
```

### `@user`

```bash
# before applying, hunter must link their EOS account with EOSN login
cleos push action login.eosn link '["hunter.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

# hunter apply to bounty
cleos push action work.pomelo apply '[bounty1, hunter.eosn]' -p hunter.eosn

# hunter signals work is completed (funds are auto-released after 72 hours if no explicit approval from author)
cleos push action work.pomelo complete '[bounty1]' -p hunter.eosn

# hunter claims bounty funds (EOS account linked with EOSN Login)
cleos push action work.pomelo claim '[bounty1]' -p myaccount
```

### `@admin`

```bash
# configure app
cleos push action work.pomelo setconfig '[1000, "login.eosn", "fee.pomelo"]' -p work.pomelo
cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
cleos push action work.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p work.pomelo
```

### `@author` (Optional)

```bash
# set bounty metadata
cleos push action work.pomelo setmetadata '[bounty1, {"url": "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"}]' -p author.eosn

# author withdraws funds bounty in "pending" state (EOS account linked with EOSN Login)
cleos push action work.pomelo withdraw '[bounty1]' -p myaccount
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
| Author        | Bounty Author                |
| Funders       | Bounty Funders                |
| SC            | Smart Contract                |

## Table of Content

- [TABLE `configs`](#table-configs)
- [TABLE `bounties`](#tables-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `tokens`](#table-tokens)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `token`](#action-token)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `createbounty`](#action-createbounty)
- [ACTION `setbounty`](#action-setbounty)
- [ACTION `setstate`](#action-setstate)
- [ACTION `approve`](#action-approve)
- [ACTION `release`](#action-release)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `deny`](#action-deny)
- [ACTION `apply`](#action-apply)
- [ACTION `complete`](#action-complete)
- [ACTION `claim`](#action-claim)
