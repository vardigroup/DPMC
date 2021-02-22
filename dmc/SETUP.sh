DMC_DIR=$(pwd)

cd ../common

./INSTALL.sh
make dmc OPT=-Ofast
cp dmc $DMC_DIR

cd $DMC_DIR
