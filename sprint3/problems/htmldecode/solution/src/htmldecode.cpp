#include "htmldecode.h"


std::string HtmlDecode(std::string_view str) {
    std::unordered_map<std::string, char> html_decode = {
        {"lt", '<'}, 
        {"gt", '>'}, 
        {"amp", '&'}, 
        {"apos", '\''}, 
        {"quot", '"'}
    };

    std::string res = "";
    
    int i = 0;

    while (i < str.size()) {
        if (str[i] == '&') {
            int j = i + 1;

            std::string temp = "";

            bool flag = false; //false - lower, true - upper

            if (j < str.size() && isupper(str[j])) flag = true;

            while (j < str.size() && j - i <= 4 && j != '&') {
                if ((islower(str[j]) && !flag) || (isupper(str[j]) && flag)) {
                    temp += str[j];
                    ++j;
                }
                else {
                    res += str[i];
                    ++i;
                    break;
                }

                std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
                auto it = html_decode.find(temp);

                if (it != html_decode.end()) {
                    res += it->second;
                    if (j < str.size() && str[j] == ';') i = j + 1;
                    else i = j;
                    break;
                }
            }
        }
        else {
            res += str[i];
            ++i;
        }
    }
    return res;
}
