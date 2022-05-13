#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>


using namespace eosio;
using namespace eosiosystem;
using namespace std;

namespace eosiosystem {

    class [[eosio::contract("eosio.system")]] system_contract {
        public:
            /**
             * Deposit to REX fund action. Deposits core tokens to user REX fund.
             * All proceeds and expenses related to REX are added to or taken out of this fund.
             * An inline transfer from 'owner' liquid balance is executed.
             * All REX-related costs and proceeds are deducted from and added to 'owner' REX fund,
             *    with one exception being buying REX using staked tokens.
             * Storage change is billed to 'owner'.
             *
             * @param owner - REX fund owner account,
             * @param amount - amount of tokens to be deposited.
             */
            [[eosio::action]]
            void deposit( const name& owner, const asset& amount );

            /**
             * Rentcpu action, uses payment to rent as many SYS tokens as possible as determined by market price and
             * stake them for CPU for the benefit of receiver, after 30 days the rented core delegation of CPU
             * will expire. At expiration, if balance is greater than or equal to `loan_payment`, `loan_payment`
             * is taken out of loan balance and used to renew the loan. Otherwise, the loan is closed and user
             * is refunded any remaining balance.
             * Owner can fund or refund a loan at any time before its expiration.
             * All loan expenses and refunds come out of or are added to owner's REX fund.
             *
             * @param from - account creating and paying for CPU loan, 'from' account can add tokens to loan
             *    balance using action `fundcpuloan` and withdraw from loan balance using `defcpuloan`
             * @param receiver - account receiving rented CPU resources,
             * @param loan_payment - tokens paid for the loan, it has to be greater than zero,
             *    amount of rented resources is calculated from `loan_payment`,
             * @param loan_fund - additional tokens can be zero, and is added to loan balance.
             *    Loan balance represents a reserve that is used at expiration for automatic loan renewal.
             */
            [[eosio::action]]
            void rentcpu( const name& from, const name& receiver, const asset& loan_payment, const asset& loan_fund );

            /**
             * Rentnet action, uses payment to rent as many SYS tokens as possible as determined by market price and
             * stake them for NET for the benefit of receiver, after 30 days the rented core delegation of NET
             * will expire. At expiration, if balance is greater than or equal to `loan_payment`, `loan_payment`
             * is taken out of loan balance and used to renew the loan. Otherwise, the loan is closed and user
             * is refunded any remaining balance.
             * Owner can fund or refund a loan at any time before its expiration.
             * All loan expenses and refunds come out of or are added to owner's REX fund.
             *
             * @param from - account creating and paying for NET loan, 'from' account can add tokens to loan
             *    balance using action `fundnetloan` and withdraw from loan balance using `defnetloan`,
             * @param receiver - account receiving rented NET resources,
             * @param loan_payment - tokens paid for the loan, it has to be greater than zero,
             *    amount of rented resources is calculated from `loan_payment`,
             * @param loan_fund - additional tokens can be zero, and is added to loan balance.
             *    Loan balance represents a reserve that is used at expiration for automatic loan renewal.
             */
            [[eosio::action]]
            void rentnet( const name& from, const name& receiver, const asset& loan_payment, const asset& loan_fund );

            /**
             * Buy ram action, increases receiver's ram quota based upon current price and quantity of
             * tokens provided. An inline transfer from receiver to system contract of
             * tokens will be executed.
             *
             * @param payer - the ram buyer,
             * @param receiver - the ram receiver,
             * @param quant - the quntity of tokens to buy ram with.
             */
            [[eosio::action]]
            void buyram( const name& payer, const name& receiver, const asset& quant );

            /**
             * Buy a specific amount of ram bytes action. Increases receiver's ram in quantity of bytes provided.
             * An inline transfer from receiver to system contract of tokens will be executed.
             *
             * @param payer - the ram buyer,
             * @param receiver - the ram receiver,
             * @param bytes - the quntity of ram to buy specified in bytes.
             */
            [[eosio::action]]
            void buyrambytes( const name& payer, const name& receiver, uint32_t bytes );

            /**
             * Delegate bandwidth and/or cpu action. Stakes SYS from the balance of `from` for the benefit of `receiver`.
             *
             * @param from - the account to delegate bandwidth from, that is, the account holding
             *    tokens to be staked,
             * @param receiver - the account to delegate bandwith to, that is, the account to
             *    whose resources staked tokens are added
             * @param stake_net_quantity - tokens staked for NET bandwidth,
             * @param stake_cpu_quantity - tokens staked for CPU bandwidth,
             * @param transfer - if true, ownership of staked tokens is transfered to `receiver`.
             *
             * @post All producers `from` account has voted for will have their votes updated immediately.
             */
            [[eosio::action]]
            void delegatebw( const name& from, const name& receiver,
                             const asset& stake_net_quantity, const asset& stake_cpu_quantity, bool transfer );


            using deposit_action = eosio::action_wrapper<"deposit"_n, &system_contract::deposit>;
            using buyrambytes_action = eosio::action_wrapper<"buyrambytes"_n, &system_contract::buyrambytes>;
            using buyram_action = eosio::action_wrapper<"buyram"_n, &system_contract::buyram>;
            using rentcpu_action = eosio::action_wrapper<"rentcpu"_n, &system_contract::rentcpu>;
            using rentnet_action = eosio::action_wrapper<"rentnet"_n, &system_contract::rentnet>;
            using delegatebw_action = eosio::action_wrapper<"delegatebw"_n, &system_contract::delegatebw>;
    };
}