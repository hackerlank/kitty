#!/bin/sh
find . -iname \*.[ch] | xargs sed -i "s/$1/$2/g"
find . -iname \*.cpp | xargs sed -i "s/$1/$2/g"
