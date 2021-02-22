HTB_DIR=$(pwd)

cd ../common

./INSTALL.sh
make htb OPT=-Ofast
cp htb $HTB_DIR

cd $HTB_DIR
