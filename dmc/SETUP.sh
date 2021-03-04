DMC_DIR=$(pwd)

cd ../common

./INSTALL.sh

make dmc OPT=-Ofast # LINK=-static
## Ofast: must not use inf or nan
## static: segfault with std::thread

cp dmc $DMC_DIR/dmc
