# 🍈 Pomelo for Work (Bounties) - EOSIO Smart Contract

![flow](https://user-images.githubusercontent.com/550895/169887932-4df6c5f4-0eb2-480e-ac91-40f3145a0a80.jpg)


## Security Audits

- N/A

## Usage

### `@author`

```bash
# before creating bounty, must link EOS account with EOSN login
cleos push action login.eosn link '["author.eosn", "myaccount", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p myaccount

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
cleos push action work.pomelo setconfig '[1000, "login.eosn", "fee.pomelo"]' -p work.pomelo
cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
cleos push action work.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p work.pomelo

# make bounty public
cleos push action work.pomelo publish '[bounty1]' -p work.pomelo

# set bounty state
cleos push action work.pomelo setstate '[bounty1, pending]' -p work.pomelo

# close bounty
cleos push action work.pomelo close '[bounty1]' -p work.pomelo

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

- [TABLE `configs`](#table-configs)
- [TABLE `bounties`](#tables-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `tokens`](#table-tokens)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `token`](#action-token)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `create`](#action-create)
- [ACTION `setmetadata`](#action-setmetadata)
- [ACTION `setstate`](#action-setstate)
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
- `{uint64_t} min_amount` - min amount required when donating
- `{uint64_t} oracle_id` - Defibox Oracle ID

### example

```json
{
    "sym": "4,EOS",
    "contract": "eosio.token",
    "min_amount": 10000,
    "oracle_id": 1
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
- `{Metadata} metadata={}` - bounty metadata
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


## TABLE `transfer`

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
    "ext_quantity": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
    "fee": "1.0000 EOS",
    "memo": "grant:grant1",
    "value": 100.0,
    "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
    "created_at": "2020-12-06T00:00:00"
}
```

## ACTION `setconfig`

- **authority**: `get_self()`

### params

- `{name} status` - contract status: ok/testing/disabled
- `{uint64_t} fee` - platform fee (bips - 1/100 1%)
- `{name} login_contract` - EOSN Login contract
- `{name} fee_account` - fee account
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
- `{uint64_t} min_amount` - min amount required when donating
- `{uint64_t} oracle_id` - Defibox oracle ID

### example

```bash
$ cleos push action work.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p work.pomelo
```

## ACTION `deltoken`

- **authority**: `get_self()`

Delete token from supported assets

### params

- `{symbol_code} symcode` - symbol code

### example

```bash
$ cleos push action work.pomelo deltoken '["EOS"]' -p work.pomelo
```

## ACTION `create`

- **authority**: `author_user_id`

Create bounty

### params

- `{name} author_user_id` - author (EOSN Login ID)
- `{name} bount_id` - bounty ID
- `{symbol_code} accepted_token` - accepted deposit token (ex: `"EOS"`)
- `{optional<name>} bounty_type` - bounty type (default = traditional)

### Example

```bash
$ cleos push action work.pomelo create '[author.eosn, bounty1, "USDT", null]' -p author.eosn
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
$ cleos push action work.pomelo setmetadata '[bounty1, url, "https://github.com/pomelo-io"]' -p author.eosn
```

## ACTION `setstate`

- **authority**: `get_self()`

Set grant or bounty state

### params

- `{name} bounty_id` - bounty ID
- `{name} status` - status `pending/published/banned/retired/denied'

### example

```bash
$ cleos push action work.pomelo setstate '[bounty1, published]' -p work.pomelo
```

## ACTION `forfeit`

- **authority**: `hunter_user_id`

Hunter forfeits the bounty and puts the bounty into `open` state

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

- **authority**: `admin`

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

> Bounty state must be "completed"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo release '[bounty1]' -p author.eosn
```

## ACTION `deny`

- **authority**: `author_user_id`

Author denies completed bounty

> Bounty state is reverted back to "progress"

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo deny '[bounty1]' -p author.eosn
```

## ACTION `withdraw`

- **authority**: `account` (EOS account linked to EOSN Login's bounty author)

Author withdraws funds from bounty in "pending" state (EOS account linked with EOSN Login)

### params

- `{name} bounty_id` - bounty ID
- `{name} receiver` - receiver account (must be linked to EOSN Login author account)

### example

```bash
$ cleos push action work.pomelo withdraw '[bounty1, myaccount]' -p myaccount
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

## ACTION `complete`

- **authority**: `user_id`

hunter signals work is completed

> Funds are auto-released after 72 hours if no explicit approval from author

### params

- `{name} bounty_id` - bounty ID

### example

```bash
$ cleos push action work.pomelo complete '[bounty1]' -p hunter.eosn
```

## ACTION `claim`

- **authority**: `account` (EOS account linked with EOSN Login)

Hunter claims bounty funds

### params

- `{name} bounty_id` - bounty ID
- `{name} receiver` - receiver account (must be linked to EOSN Login approved applicant)

### example

```bash
$ cleos push action work.pomelo claim '[bounty1]' -p myaccount
```

## TRANSFER NOTIFY HANDLER `on_transfer`

Process incoming transfer

### params

- `{name} from` - from EOS account (donation sender)
- `{name} to` - to EOS account (process only incoming)
- `{asset} quantity` - quantity received
- `{string} memo` - transfer memo, i.e. "autho.eosn:123"
