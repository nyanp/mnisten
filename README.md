# mnisten : image files to idx format converter
mnisten is a library to convert image files to [idx format](http://yann.lecun.com/exdb/mnist/) binaries.
- assign label id automatically by directory-name (lexicographical order)
- auto resizing
- random-shuffling
- generate train/test file

#example

image files:
```
.\
|--foo
|   |--a.bmp
|   |--b.bmp
|   +--1.txt
+--bar
    |--c.bmp
    |--d.bmp
    +--buzz
         +--e.bmp
```
- a and b.bmp => label 1
- c and d.bmp => label 0
- 1.txt and e.bmp => ignored

#usage

```
mnisten -d .\ -o prefix -s 32x24
```

then create:
- prefix_train_images.idx3
- prefix_train_labels.idx1
- label.txt
    - log directory-name => label-id

|option|parameters|meaning|required|default|
|---|---|---|---|---|
|--dir,-d|directory-name(string)|root directory of images|yes|-|
|--output,-o|prefix(string)|prefix of output idx files|no|""|
|--num-tests,-n|num(int)|number of test data (if 0, all images are treated as training data)|no|0|
|--size,-s|WxH(string)|size of each image data|no|32x32|


#prerequisites
- OpenCV
- boost (boost/filesystem)

#build
```
make
```
or build vc/mnisten.sln
