#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <random>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/detail/endian.hpp>

#include <opencv/cv.hpp>
#include <opencv2/highgui.hpp>

#include "cmdline.h"
#include "util.h"

using namespace std;
namespace fs = boost::filesystem;

struct image {
    cv::Mat data;
    uint8_t label;
};

template <typename ImageIterator>
void gen_mnist_labels(const string& label_file, ImageIterator begin, ImageIterator end)
{
    ofstream ofs(label_file.c_str(), ios::binary|ios::out);

    if (ofs.bad() || ofs.fail())
        throw runtime_error("failed to open file:" + label_file);

    uint32_t magic_number = 0x00000801;
    uint32_t num_items = distance(begin, end);

#if defined(BOOST_LITTLE_ENDIAN)
    reverse_endian(&magic_number);
    reverse_endian(&num_items);
#endif

    ofs.write((char*) &magic_number, 4);
    ofs.write((char*) &num_items, 4);

    for (; begin != end; ++begin)
        ofs.write((char*) &begin->label, 1);
}

template <typename ImageIterator>
void gen_mnist_images(const string& image_file, ImageIterator begin, ImageIterator end)
{
    ofstream ofs(image_file.c_str(), ios::binary | ios::out);

    if (ofs.bad() || ofs.fail())
        throw runtime_error("failed to open file:" + image_file);

    uint32_t magic_number = 0x00000803;
    uint32_t num_items = distance(begin, end);
    uint32_t num_rows = begin->data.rows;
    uint32_t num_cols = begin->data.cols;

#if defined(BOOST_LITTLE_ENDIAN)
    reverse_endian(&magic_number);
    reverse_endian(&num_items);
    reverse_endian(&num_rows);
    reverse_endian(&num_cols);
#endif

    ofs.write((char*) &magic_number, 4);
    ofs.write((char*) &num_items, 4);
    ofs.write((char*) &num_rows, 4);
    ofs.write((char*) &num_cols, 4);

    for (; begin != end; ++begin) {
        cv::Mat_<uint8_t>& m = (cv::Mat_<uint8_t>&)begin->data;

        for (auto v : m)
            ofs.write((const char*)&v, 1);
    }
}

int read_images(const fs::path& path, uint8_t label, vector<image>& images, int w, int h)
{
    int num_images = 0;
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path), fs::directory_iterator())) {
        if (fs::is_directory(p)) continue;

        image img;
        cv::Mat srcimg = cv::imread(p.string(), cv::IMREAD_GRAYSCALE);
        if (srcimg.data == nullptr)
            continue;

        cv::resize(srcimg, img.data, cv::Size(w, h)); // gray, linear interpolation

        //cv::imshow("data", img.data);
        //cv::waitKey(5);

        img.label = label;
        images.push_back(img);
        num_images++;
    }
    return num_images;
}

void dump_map(const map<string, uint8_t>& m, const string& filename)
{
    ofstream ofs(filename.c_str());
    if (ofs.fail() || ofs.bad())
        throw runtime_error("failed to create file:" + filename);

    for (auto v : m)
        ofs << v.first << "," << v.second << endl;
}

string add_prefix(const string& prefix, const string& base)
{
    return prefix.empty() ? base : prefix + "_" + base;
}

void exec(const string& dir, const string& output_prefix, int num_tests, int w, int h, bool data_shuffle = true)
{
    auto map = make_table_from_subdir_to_label(dir);
    vector<image> images;
    fs::path path(dir);

    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path), fs::directory_iterator())) {
        if (fs::is_directory(p)) {
            int n = read_images(p, map[p.string()], images, w, h);
            cout << (int)map[p.string()] << ":" << p.stem().string() << " " << n << "images" << endl;
        }
    }

    // shuffle
    if (data_shuffle)
        shuffle(images.begin(), images.end(), default_random_engine(0));

    // split train/test data
    if ((int)images.size() <= num_tests)
        throw runtime_error("total images are less than num_tests");

    cout << "total " << images.size() << "images found." << endl;

    string train_img   = add_prefix(output_prefix, "train_images.idx3");
    string train_label = add_prefix(output_prefix, "train_labels.idx1");
    string test_img    = add_prefix(output_prefix, "test_images.idx3");
    string test_label  = add_prefix(output_prefix, "test_labels.idx1");

    if (num_tests > 0) {
        gen_mnist_images(test_img,   images.begin(), images.begin() + num_tests);
        gen_mnist_labels(test_label, images.begin(), images.begin() + num_tests);
    }
    gen_mnist_images(train_img,   images.begin() + num_tests, images.end());
    gen_mnist_labels(train_label, images.begin() + num_tests, images.end());

    dump_map(map, "label.txt");
}

int main(int argc, char *argv[])
{
  cmdline::parser a;
  a.add<string>("dir", 'd', "target directory", true);
  a.add<string>("output", 'o', "output file prefix", false);
  a.add<int>("num-tests", 'n', "number of test data", false, 0);
  a.add<string>("size", 's', "size of output data (WxH)", false, "32x32");
  a.add("without-shuffle", 'w', "create data without shuffling");

  a.parse_check(argc, argv);

  try {
      auto size = parse_size(a.get<string>("size"));
      exec(a.get<string>("dir"), a.get<string>("output"), a.get<int>("num-tests"), size.first, size.second, !a.exist("without-shuffle"));
  }
  catch (const exception& e) {
      cout << "error:" << e.what() << endl;
  }
  catch (...) {
      cout << "error:unknown exception" << endl;
  }

  return 0;
}
