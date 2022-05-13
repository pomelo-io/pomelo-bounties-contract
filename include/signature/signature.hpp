#include <eosio/crypto.hpp>


using namespace std;
using namespace eosio;

namespace sig {

    enum key_type : uint8_t
    {
        k1 = 0,
        r1 = 1,
        wa = 2,
    };
    constexpr char base58_chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    constexpr auto create_base58_map()
    {
        std::array<int8_t, 256> base58_map{{0}};
        for (unsigned i = 0; i < base58_map.size(); ++i)
            base58_map[i] = -1;
        for (unsigned i = 0; i < sizeof(base58_chars); ++i)
            base58_map[base58_chars[i]] = i;
        return base58_map;
    }

    constexpr auto base58_map = create_base58_map();

    template <typename Container>
    void base58_to_binary(Container& result, std::string_view s)
    {
        std::size_t offset = result.size();
        for (auto& src_digit : s)
        {
            int carry = base58_map[static_cast<uint8_t>(src_digit)];
            check(carry >= 0, "base58_to_binary: failed to convert");
            for (std::size_t i = offset; i < result.size(); ++i)
            {
                auto& result_byte = result[i];
                int x = static_cast<uint8_t>(result_byte) * 58 + carry;
                result_byte = x;
                carry = x >> 8;
            }
            if (carry)
                result.push_back(static_cast<uint8_t>(carry));
        }
        for (auto& src_digit : s)
            if (src_digit == '1')
                result.push_back(0);
            else
                break;
        std::reverse(result.begin() + offset, result.end());
    }


    template <typename Key>
    Key string_to_key(std::string_view s, key_type type, std::string_view suffix)
    {
        std::vector<char> whole;
        // whole.push_back(uint8_t{type});  //TODO: figure out other types
        base58_to_binary(whole, s);
        check(whole.size() > 5, "string_to_key: failed");
    //   auto ripe_digest =
    //       digest_suffix_ripemd160(std::string_view(whole.data() + 1, whole.size() - 5), suffix);
    //   check(memcmp(ripe_digest.data(), whole.data() + whole.size() - 4, 4) == 0,
    //         convert_json_error(from_json_error::expected_key));
        whole.erase(whole.end() - 4, whole.end());
        Key sig;
        for(int i=0; i<whole.size(); i++){
            get<0>(sig)[i] = whole[i];
        }
        return sig;
    }

    signature signature_from_string(std::string_view s)
    {
        if (s.size() >= 7 && s.substr(0, 7) == "SIG_K1_")
            return string_to_key<signature>(s.substr(7), key_type::k1, "K1");
        else if (s.size() >= 7 && s.substr(0, 7) == "SIG_R1_")
            return string_to_key<signature>(s.substr(7), key_type::r1, "R1");
        else if (s.size() >= 7 && s.substr(0, 7) == "SIG_WA_")
            return string_to_key<signature>(s.substr(7), key_type::wa, "WA");

        check(false, "signature_from_string: failed to convert");
        return {};
    }
}