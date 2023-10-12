# 🍈 Pomelo for Work (Bounties) - Antelope Smart Contract

## Chains supported

| id        | chain |
|-----------|-------|
| `eos`     | EOS Native
| `eos.evm` | EOS EVM

## Security Audits

- N/A

## Usage

### `@author`

```bash
# create bounty
cleos push action work.pomelo create '[author.eosn, bounty1, "USDT", null]' -p author.eosn

# fund bounty
cleos transfer myaccount work.pomelo "100.0000 USDT" "bounty1,funder.eosn" --contract tethertether

# author select hunter for bounty
cleos push action work.pomelo approve '[bounty1, hunter.eosn]' -p author.eosn

# author closes unsuccessful open/pending bounty putting it into "closed" state
cleos push action work.pomelo close '[bounty1]' -p author.eosn

# OPTION 1. Author releases funds to hunter when bounty is completed
# > Bounty state must be "completed"
cleos push action work.pomelo release '[bounty1]' -p author.eosn

# OPTION 2. Author denies completed bounty
# > Bounty state is reverted back to "progress"
cleos push action work.pomelo deny '[bounty1]' -p author.eosn

# OPTION 3. funds auto-released to hunter after 72 hours
# /* no action */
```

### `@hunter`

```bash
# before applying, hunter must link their EOS account with EOSN login
cleos push action login.eosn link '["hunter.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

# hunter apply to bounty
cleos push action work.pomelo apply '[bounty1, hunter.eosn]' -p hunter.eosn

# hunter forfeits bounty
cleos push action work.pomelo forfeit '[bounty1]' -p hunter.eosn

# hunter signals work is completed (funds are auto-released after 72 hours if no explicit approval from author)
cleos push action work.pomelo complete '[bounty1]' -p hunter.eosn

# hunter claims bounty funds (EOS account linked with EOSN Login)
cleos push action work.pomelo claim '[bounty1]' -p myaccount
```

### `@admin`

```bash
# configure app
cleos push action work.pomelo setconfig '[ok, 1000, login.eosn, fee.pomelo, [url]]' -p work.pomelo
cleos push action work.pomelo token '["4,USDT", tethertether, 10000, 0]' -p work.pomelo

# disable contract
cleos push action work.pomelo setconfig '[disabled, null, null, null, []]' -p work.pomelo

# make bounty public
cleos push action work.pomelo publish '[bounty1]' -p work.pomelo

# set bounty state
cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo

# sync bounty details
cleos push action work.pomelo syncbounty '[bounty1, started, [applicant1.eosn, applicant2.eosn], applicant2.eosn, 2021-01-01T00:00:00, null, null]' -p work.pomelo

# close bounty
cleos push action work.pomelo close '[bounty1]' -p work.pomelo

```

### `@author` (Optional)

```bash
# set bounty metadata
cleos push action work.pomelo setmetadata '[bounty1, url, "https://github.com/pomelo-io/"]' -p author.eosn

# author withdraws funds bounty in "pending" state (EOS account `myaccount` should be linked to `author.eosn` EOSN Login)
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

## State Diagram

<img width="843" alt="image" src="https://user-images.githubusercontent.com/29608734/169374374-ad0129cd-60e7-4235-8e25-ddf7d659268a.png">


## Definitions

### Roles

| `role`        | `description`                 |
|---------------|-------------------------------|
| Admin         | Pomelo Admin                  |
| Author        | Bounty Author                 |
| Hunter        | Bounty Hunter                 |
| Funder        | Bounty Funder                 |
| SC            | Smart Contract                |

## Table of Content

### Tables
- [TABLE `configs`](#table-configs)
- [TABLE `bounties`](#table-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `tokens`](#table-tokens)

### Actions
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `token`](#action-token)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `create`](#action-create)
- [ACTION `setmetadata`](#action-setmetadata)
- [ACTION `setstate`](#action-setstate)
- [ACTION `syncbounty`](#action-syncbounty)
- [ACTION `publish`](#action-publish)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `apply`](#action-apply)
- [ACTION `approve`](#action-approve)
- [ACTION `release`](#action-release)
- [ACTION `deny`](#action-deny)
- [ACTION `complete`](#action-complete)
- [ACTION `forfeit`](#action-forfeit)
- [ACTION `claim`](#action-claim)
- [ACTION `close`](#action-close)

### Action Notifications
- [TRANSFER NOTIFY HANDLER `on_transfer`](#transfer-notify-handler-on_transfer)


## TABLE `configs`

### params

- `{name} status` - contract status ("ok", "testing", "maintenance")
- `{uint64_t} fee` - platform fee (bips - 1/100 1%)
- `{name} login_contract` - EOSN Login contract
- `{name} fee_account` - fee
- `{set<name>} metadata_keys` - list of keys allowed to include in bounty Metadata

### example

```json
{
    "status": "ok",
    "fee": 500,
    "login_contract": "login.eosn",
    "fee_account": "fee.pomelo",
    "metadata_keys": ["url"]
}
```

## TABLE `tokens`

### params

- `{symbol} sym` - (primary key) symbol
- `{name} contract` - token contract
- `{uint64_t} min_amount` - min amount required per bounty
- `{uint64_t} max_amount` - max amount required per bounty

### example

```json
{
    "sym": "4,USDT",
    "contract": "tethertether",
    "min_amount": 50000,
    "max_amount": 5000000
}
```

## TABLE `bounties`

*scope*: `{name} get_self()`

## params

- `{name} bount_id` - (primary key) bounty ID (ex: "bounty1")
- `{name} author_user_id` - author (EOSN Login ID)
- `{map<name, asset>} funders` - funders contributions map (EOSN Login id => amount)
- `{extended_asset} amount` - amount of tokens to be released once bounty is completed
- `{set<name>} applicant_user_ids` - list of applicants that have applied to bounty
- `{set<name>} approved_user_id` - approved account for bounty
- `{name} status="pending"` - status (`pending/open/started/submitted/done`)
- `{name} type="traditional"` - bounty type (`traditional` = "1 worker at a time, 1 is paid out")
- `{name} permissions="approval"` - bounty permissions (`approval` = "Funder must approve hunter to start work")
- `{map<name, string>} metadata={}` - bounty metadata
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} submitted_at` - submitted at time
- `{time_point_sec} completed_at` - completed at time

### example

```json
{
    "bount_id": "bounty1",
    "author_user_id": "author.eosn",
    "funders": {"funder1.eosn": "5.0000 USDT", "funder2.eosn": "5.0000 USDT"},
    "amount": {"quantity": "10.0000 USDT", "contract": "tethertether"},
    "claimed": "10.0000 USDT",
    "applicant_user_ids": ["hunter.eosn"],
    "approved_user_id": "hunter.eosn",
    "status": "pending",
    "type": "traditional",
    "permissions": "approval",
    "metadata": [{"key": "url", "value": "https://github.com/pomelo-io/pomelo-bounties-contract/issues/1"}],
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "submitted_at": "1970-01-01T00:00:00",
    "completed_at": "1970-01-01T00:00:00"
}
```


## TABLE `transfers`

- **scope**: `{name} get_self()`

### params

- `{uint64_t} transfer_id` - (primary key) token transfer ID
- `{name} bounty_id` - bounty ID
- `{name} from` - EOS account sender
- `{name} to` - EOS account receiver
- `{extended_asset} ext_quantity` - amount of tokens transfered
- `{asset} fee` - fee charged and sent to `global.fee_account`
- `{string} memo` - transfer memo
- `{double} value` - valuation at time of received
- `{checksum256} trx_id` - transaction ID
- `{time_point_sec} created_at` - created at time

### example

```json
{
    "transfer_id": 10001,
    "bounty_id": 123,
    "funder_user_id": "funder.eosn",
    "from": "myaccount",
    "to": "work.pomelo",
    "ext_quantity": {"contract": "tethertether", "quantity": "15.0000 USDT"},
    "fee": "1.0000 USDT",
    "memo": "grant:grant1",
    "value": 100.0,
    "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
    "created_at": "2020-12-06T00:00:00"
}
```

## ACTION `setconfig`

- **authority**: `get_self()`

### params

- `{name} [status]` - contract status: ok/testing/disabled
- `{uint64_t} [fee]` - platform fee (bips - 1/100 1%)
- `{name} [login_contract]` - EOSN Login contract
- `{name} [fee_account]` - fee account
- `{set<name>} metadata_keys` - allowed metadata keys

### example

```bash
$ cleos push action work.pomelo setconfig '[ok, 1000, "login.eosn", "fee.pomelo", [url]]' -p work.pomelo
```

## ACTION `token`

- **authority**: `get_self()`

Set token as supported asset

### params

- `{symbol} sym` - (primary key) symbol
- `{name} contract` - token contract
- `{uint64_t} min_amount` - min amount required per bounty
- `{uint64_t} max_amount` - max amount required per bounty

### example

```bash
$ cleos push action work.pomelo token '["4,USDT", "tethertether", 50000, 5000000]' -p work.pomelo
```

## ACTION `deltoken`

- **authority**: `get_self()`

Delete token from supported assets

### params

- `{symbol_code} symcode` - symbol code

### example

```bash
$ cleos push action work.pomelo deltoken '["USDT"]' -p work.pomelo
```

## ACTION `create`

- **authority**: `author_user_id`

Create bounty

### params

- `{name} author_user_id` - author (EOSN Login ID)
- `{name} bount_id` - bounty ID
- `{symbol_code} accepted_token` - accepted deposit token (ex: `"USDT"`)
- `{string} url` - bounty URL (ex: GitHub issue URL)
- `{optional<name>} bounty_type` - bounty type (default = traditional)

### Example

```bash
$ cleos push action work.pomelo create '[author.eosn, bounty1, "USDT", "https://github.com/pomelo-io/pomelo-rest-api/issues/735", null]' -p author.eosn
```

## ACTION `setmetadata`

- **authority**: `author_user_id`

Add/remove metadata. If key == "" - remove.

### params

- `{name} bounty_id` - bounty ID
- `{name} metadata_key` - one of the allowed metadata keys
- `{string} metadata_value` - metadata value

### example

```bash
$ cleos push action work.pomelo setmetadata '[bounty1, url, "https://github.com/pomelo-io/pomelo-rest-api/issues/735"]' -p author.eosn
```

## ACTION `setstate`

- **authority**: `get_self()`

Set bounty state. Shouldn't be used for normal flows. Only to override by admin.

### params

- `{name} bounty_id` - bounty ID
- `{name} state` - new state `pending/closed/open/started/submitted/released/done'

### example

```bash
$ cleos push action work.pomelo setstate '[bounty1, published]' -p work.pomelo
```

## ACTION `syncbounty`

- **authority**: `get_self()`

Sync bounty details from backend. Called by `/admin/sync` on backend if there are inconsistencies between backend and blockchain.

### params

- `{name} bounty_id` - bounty ID
- `{name} status` - status `pending/published/banned/retired/denied'
- `{vector<name>} applicant_user_ids` - list of applicants
- `{optional<name>} approved_user_id` - approved applicant
- `{time_point_sec} updated_at` - last update time
- `{optional<time_point_sec>} submitted_at` - submitted time
- `{optional<time_point_sec>} completed_at` - completed time

### example

```bash
$ cleos push action work.pomelo syncbounty '[bounty1, started, [applicant1.eosn, applicant2.eosn], applicant2.eosn, 2021-01-01T00:00:00, null, null]' -p work.pomelo
```

## ACTION `forfeit`

- **authority**: `hunter_user_id`

Hunter forfeits the bounty, puts the bounty into `open` state and removes himself from the list of applicants

> Bounty state must be "started"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo forfeit '[bounty1]' -p hunter.eosn
```

## ACTION `close`

- **authority**: `author_user_id`

Author closes the bounty and puts it into `closed` state

> Bounty state must be "open" or "pending"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo close '[bounty1]' -p author.eosn
```

## ACTION `publish`

- **authority**: `get_self()`

Admin publishes the bounty making it open for applications

> Bounty state must be "pending"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo publish '[bounty1]' -p work.pomelo
```

## ACTION `release`

- **authority**: `author_user_id`

Author releases funds to hunter when bounty is completed

> Bounty state must be "submitted"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo release '[bounty1]' -p author.eosn
```

## ACTION `deny`

- **authority**: `author_user_id`

Author denies completed bounty

> Bounty state is reverted back to "started"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo deny '[bounty1]' -p author.eosn
```

## ACTION `withdraw`

- **authority**: `author_user_id` (EOSN Login)

Author withdraws funds from bounty in "pending" state

### params

- `{name} bounty_id` - bounty ID
- `{name} chain` - chain name
- `{name} receiver` - receiver (Antelope account or EVM address)
- `{string} [memo=""]` - (optional) memo (only available when using Antelope receiver)

### example

```bash
// withdraw to EOS Native
$ cleos push action work.pomelo withdraw '[bounty1, eos, "myaccount", null]' -p author.eosn

// withdraw to Exchange
$ cleos push action work.pomelo withdraw '[bounty1, eos, "eosbndeposit", "100245696"]' -p author.eosn

// withdraw to EOS EVM
$ cleos push action work.pomelo withdraw '[bounty1, eos.evm, "0xaa2F34E41B397aD905e2f48059338522D05CA534", null]' -p author.eosn
```

## ACTION `apply`

- **authority**: `user_id`

Hunter apply to bounty

### params

- `{name} bounty_id` - bounty ID
- `{name} user_id` - user ID (EOS account linked to EOSN Login)

### example

```bash
$ cleos push action work.pomelo apply '[bounty1, hunter.eosn]' -p hunter.eosn
```

## ACTION `approve`

- **authority**: `author_id`

Author approves one of the hunters who applied for bounty

### params

- `{name} bounty_id` - bounty ID
- `{name} applicant_user_id` - hunter user ID

### example

```bash
$ cleos push action work.pomelo approve '[bounty1, hunter.eosn]' -p hunter.eosn
```

## ACTION `complete`

- **authority**: `user_id`

Hunter signals the work is completed

> Funds are auto-released after 72 hours if no explicit approval from author

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo complete '[bounty1]' -p hunter.eosn
```

## ACTION `claim`

- **authority**: `approved_user_id` (EOSN Login)

Hunter claims bounty funds

### params

- `{name} bounty_id` - bounty ID
- `{name} chain` - chain name
- `{name} receiver` - receiver (Antelope account or EVM address)
- `{string} [memo=""]` - (optional) memo (only available when using Antelope receiver)

### example

```bash
// claim to EOS Native
$ cleos push action work.pomelo claim '[bounty1, eos, "myaccount", null]' -p claimer.eosn

// claim to Exchange
$ cleos push action work.pomelo claim '[bounty1, eos, "eosbndeposit", "100245696"]' -p claimer.eosn

// claim to EOS EVM
$ cleos push action work.pomelo claim '[bounty1, eos.evm, "0xaa2F34E41B397aD905e2f48059338522D05CA534", null]' -p claimer.eosn
```

## TRANSFER NOTIFY HANDLER `on_transfer`

Process incoming transfer

### params

- `{name} from` - from EOS account (donation sender)
- `{name} to` - to EOS account (process only incoming)
- `{asset} quantity` - quantity received
- `{string} memo` - transfer memo, i.e. "bounty1,author.eosn"

### example

```bash
$ cleos transfer author work.pomelo "5.0000 USDT" "bounty1,author.eosn" --contract tethertether -p author.eosn
```
