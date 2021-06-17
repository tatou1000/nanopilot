#!/usr/bin/bash -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"

ARCHNAME="x86_64"
BLAS_TARGET="X64_AUTOMATIC"
if [ -f /TICI ]; then
  ARCHNAME="larch64"
  BLAS_TARGET="ARMV8A_ARM_CORTEX_A57"
elif [ -f /EON ]; then
  ARCHNAME="aarch64"
  BLAS_TARGET="ARMV8A_ARM_CORTEX_A57"
fi

if [ ! -d acados/ ]; then
  #git clone https://github.com/acados/acados.git $DIR/acados
  git clone https://github.com/commaai/acados.git $DIR/acados
fi
cd acados
git fetch
git checkout 9a1bab3f8fc4814a295fbf424fdc8125c63fdd08
git submodule update --recursive --init

# build
mkdir -p build
cd build
cmake -DACADOS_WITH_QPOASES=ON -UBLASFEO_TARGET -DBLASFEO_TARGET=$BLAS_TARGET ..
make -j4 install

INSTALL_DIR="$DIR/$ARCHNAME"
rm -rf $INSTALL_DIR
mkdir -p $INSTALL_DIR

rm $DIR/acados/lib/*.json

cp -r $DIR/acados/include $DIR
cp -r $DIR/acados/lib $INSTALL_DIR
cp -r $DIR/acados/interfaces/acados_template/acados_template $DIR/../../pyextra

#pip3 install -e $DIR/acados/interfaces/acados_template
