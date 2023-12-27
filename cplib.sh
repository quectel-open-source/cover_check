#!/bin/sh

exe="./bin/uidemo01" #修改发布程序的名称
des="./output/" #修改输出可执行文件路径

deplist=$(ldd $exe | awk '{if (match($3,"/")){ printf("%s "),$3 } }')
cp $deplist $des
