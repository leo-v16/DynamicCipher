#include <iostream>
#include <cstdlib>
#include <ctime>

#define BLOCK_SIZE 100
#define TIME_SIZE 10
#define EXTRA 0

class Edata {
    std::string data = "";
    std::string const chars = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%^&*()_+-={}[]:<,>.?/ "; // len-88

    int lfsr(int upperLimit, int lowerLimit, unsigned &state) {  // Linear Feedback Shift Registor (32-bit)
        bool b = state&1;
        bool f = state>>31;
        state >>= 1;
        state += (b^f)<<31;
        return state % (upperLimit - lowerLimit) + lowerLimit;
    }

    std::string hashCharSet(std::string chars, unsigned &state) {
        std::string charSet = "";
        unsigned const length = chars.size();
        for (int i = 0, prp; i < length; i++) {
            prp = lfsr(chars.size(), 0, state);  // Pseudo Random Position (prp)
            charSet += chars[prp];
            chars = chars.erase(prp, 1);  // Removes the character from the string
        }
        return charSet;
    }

    std::string GenerateKey(std::string key, int blockLen) {
        std::string newKey = "";
        for (int i = 0, j = 0; i < blockLen; i++) {
            newKey += key[j];
            j += (j == key.size()-1)? -j : 1;
            if (j == 0) newKey += std::to_string(key.size());
        }
        return newKey;
    }

    std::string FillBlock(std::string data, int blockLen, std::string chars, int firstState) {
        std::string block = "";
        std::string const ref = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%^&*()_+-={}[]:<,>.?/ ";
        for (int i = 0, sum = 0; i < blockLen; i++) {
            if (i == data.size()) {
                block += "~";
                continue;
            }
            if (i < data.size()) {
                block += chars[ref.find(data[i])];
            } else {
                int pos = (chars.find(block[i-1]) + sum)%chars.size();
                sum += chars.find(block[i-1]) + sum;
                block += chars[pos];
            }
        }
        return block + std::to_string(firstState);
    }

    std::string hashBlock(std::string block, std::string newKey, bool unHash = false) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            int I = (unHash)? BLOCK_SIZE-1 - i : i;
            unsigned state = newKey[I]^I;
            int prp = lfsr(BLOCK_SIZE, 0, state);
            std::swap(block[I], block[prp]);
        }
        return block;
    }

    std::string encrypt_(std::string key, std::string data, std::string chars) {
        unsigned firstState = time(0);
        unsigned state = firstState;
        chars = hashCharSet(chars, state);
        std::string block = FillBlock(data, BLOCK_SIZE-TIME_SIZE-EXTRA, chars, firstState);
        block = hashBlock(block, GenerateKey(key, BLOCK_SIZE));
        return block;
    }

    std::string decrypt_(std::string key, std::string block, std::string ref) {
        block = hashBlock(block, GenerateKey(key, BLOCK_SIZE), true); // unHash that's why true
        unsigned state = unsigned(std::stoi(block.substr(90, 10)));
        std::string chars = hashCharSet(ref, state);
        std::string data = "";
        for (int i = 0; i < block.size(); i++) {
            if (chars.find(block[i]) > chars.size()) break;
            data += ref[chars.find(block[i])];
        }
        return data;
    }

public:

    Edata(std::string data, std::string key) {
        this->data = encrypt_(key,data, this->chars);
    }

    std::string GetData(std::string key) {
        return decrypt_(key, data, this->chars);
    }

    std::string encrypt(std::string planeText, std::string key) {
        return encrypt_(key, planeText, this->chars);
    }

    std::string decrypt(std::string cipherText, std::string key) {
        return decrypt_(key, cipherText, this->chars);
    }
    
};

int main() {
    // Plain Text must be less then 90 charcaters and avoid using special chracter which are not in ascii along with "~, `"
    Edata msg("Hey Ken your OTP is 679954", "K");
    std::cout << msg.GetData("K");
    return 0;
}
