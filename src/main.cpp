#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "cmdline.h"
#include "util.h"

using namespace std;
namespace fs = boost::filesystem;

struct image {
    //cv::Mat data;
    int label;
};

void read_images(const fs::path& path, int label, vector<image>& images)
{

    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path), fs::directory_iterator())) {
        if (!fs::is_directory(p))
           // read_images(p, map[p.string()], images);
    }
}

void exec(const string& dir, const string& output_prefix, int num_tests, int w, int h)
{
    auto map = make_table_from_subdir_to_label(dir);
    vector<image> images;

    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path), fs::directory_iterator())) {
        if (fs::is_directory(p))
            read_images(p, map[p.string()], images);
    }
}


/**
 * mnisten -d image-dir -o output-file-prefix -s 32x32 -n 1000
 **/
int main(int argc, char *argv[])
{
  cmdline::parser a;
  a.add<string>("dir", 'd', "target directory", true);
  a.add<string>("output", 'o', "output file prefix", true);
  a.add<int>("num-tests", 'n', "number of test data", false, 0);
  a.add<string>("size", 's', "size of output data (WxH)", false, "32x32");

  a.parse_check(argc, argv);

  try {
      auto size = parse_size(a.get<string>("size"));
      exec(a.get<string>("dir"), a.get<string>("output"), a.get<int>("num-tests"), size.first, size.second);
  }
  catch (...) {
      cout << "unknown exception" << endl;
  }

  return 0;
}
