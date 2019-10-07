#pragma once

#include <map>
#include <memory>
#include <string>

template <class Handler, typename _Char = char>
class Token {
public:
    Token() = default;
    explicit Token(std::basic_string<_Char> ch) : currentMatch(ch) {}
    Token(std::basic_string<_Char> ch, Handler handler)
        : currentMatch(ch), handler(handler) {}

    Handler* findMatch(std::basic_string<_Char> ref);

    void addSubToken(std::basic_string<_Char> ch, Handler handler);

    const std::basic_string<_Char>& getMatcherString() const {
        return currentMatch;
    }

    Handler* getHandler() {
        return handler;
    }

private:
    std::basic_string<_Char> currentMatch;
    Handler* handler;
    std::map<_Char, Token<Handler, _Char>> nextTokens;
};

template<class Handler, typename _Char>
Handler *Token<Handler, _Char>::findMatch(std::basic_string<_Char> ref) {
    try {
        Token<Handler, _Char> *current = this;
        do {
            auto other = ref.substr(0, current->currentMatch.length());
            if (other != current->currentMatch) {
                return nullptr;
            }
            if (other.length() == ref.length()) {
                return current->handler;
            }

            ref.erase(0, current->currentMatch.length());

            auto nextIt = nextTokens.find(ref[0]);
            if (nextIt == nextTokens.end()) {
                return nullptr;
            }
            current = &nextIt->second;
        } while(!ref.empty());
    } catch (const std::out_of_range& e) {
        return nullptr;
    }
    return nullptr;
}

template<class Handler, typename _Char>
void Token<Handler, _Char>::addSubToken(std::basic_string<_Char> ch, Handler h) {
    std::basic_string<_Char> currentTokenMatch;
    for (typename std::basic_string<_Char>::size_type i = 0U; i < ch.length() && i < currentMatch.length(); ++i) {
        if (ch[i] == currentMatch[i]) {
            currentTokenMatch += ch[i];
        } else break;
    }

    // The requested token matches entirely this token, so add it as a sub
    auto substr = ch.substr(currentTokenMatch.length());
    if (currentTokenMatch.length() == currentMatch.length()) {
        // The requested token is this token, replace the handler
        if (substr.length() == 0) {
            handler = std::move(h);
            return;
        }
        auto firstCh = substr[0];
        auto next = nextTokens.find(firstCh);
        if (next != nextTokens.end()) {
            next->second.addSubToken(substr, std::move(h));
            return;
        }
        nextTokens.insert_or_assign(firstCh, std::move(Token(substr, std::move(h))));
        return;
    }

    // This token shares some parts with the requested token, and currentTokenMatch holds that data
    // We must:
    // 1. Create a new token with the diff and holding the handler of the current token
    // 2. Replace the current token with the shared part and a nullptr handler
    // 3. Add the new token to the current token as a sub token
    // 4. Add the requested token to the new token as a sub token

    Token newToken(currentMatch.substr(currentTokenMatch.length()), std::move(handler));
    currentMatch = currentTokenMatch;
    nextTokens.insert_or_assign(newToken.currentMatch[0], std::move(newToken));
    nextTokens.insert_or_assign(substr[0], std::move(Token(substr, std::move(h))));
}
