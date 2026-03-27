#pragma once

#include "common.hpp"

namespace browservice {

// Signed data URLs are data URLs which contain arbitrary data as plaintext such
// that we can check whether URL was created us (using message
// authentication code)

string generateDataURLSignKey();

string createSignedDataURL(string data, string signKey);

// Returns data contained by given data URL if it has been signed with given
// key otherwise returns empty
optional<string> readSignedDataURL(string dataURL, string signKey);

}
