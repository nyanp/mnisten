#pragma once

#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace std; // oops
namespace fs = boost::filesystem;

inline map<string, int> make_table_from_subdir_to_label(const string& basedir)
{
    map<string, int> res;
    fs::path path(basedir);

    if (!fs::exists(path)) {
        throw std::runtime_error("path not exist");
    }

    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path), fs::directory_iterator())) {
        if (fs::is_directory(p))
            res[p.string()] = 0;
    }

    // assign label by alphabetical order (0-origin)
    int n = 0;
    for (auto& p : res)
        p.second = n++;
    return res;
}

// "32x24" => [32, 24]
inline pair<int, int> parse_size(const string& size)
{
    size_t idx = size.find_first_of('x');
    if (idx == string::npos)
        throw runtime_error("invalid size format");

    string w = size.substr(0, idx);
    string h = size.substr(idx + 1, size.length() - idx - 1);
    return make_pair(stoi(w), stoi(h));
}
