HTB_DIR=$(pwd)

cd ../common

./INSTALL.sh

make clean

make htb OPT=-Ofast # LINK=-static
## Ofast: must not use inf or nan
## static: segfault with std::thread

cp htb $HTB_DIR/htb
