// Copyright (c) 2014-2021 The SHAHCOIN Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <util/translation.h>
#include <common/url.h>

// Define translation function for non-Qt builds
const std::function<std::string(const char*)> G_TRANSLATION_FUN = [](const char* psz) {
    return std::string(psz);
};

// Simple URL decode fallback implementation
std::string urlDecodeFallback(const std::string& url_encoded) {
    // Simple implementation that just returns the input for now
    return url_encoded;
}

// Define URL decode function
UrlDecodeFn* const URL_DECODE = urlDecodeFallback; 
