HTB_DIR="$(pwd)"

cd ../common

./INSTALL.sh
make htb
cp htb $HTB_DIR

cd $HTB_DIR
