#!/bin/bash
# magic.sh

# TODO
# Ctrl&f doesn't work in IDF3 monitor (as it just calls make)
#
# look for local-sdks first;
# does it fix just to do that in idf-env, or do we need elsewhere too?
#
# idf-env should test for version; if 3, then use this instead of idf-py:
# PATH=~/.espressif/tools/xtensa-esp32-elf/1.22.0-97-gc752ad5-5.2.0/xtensa-esp32-elf/bin:$PATH IDF_PATH=local-sdks/esp-idf make -f idf3.mk menuconfig

# standard locals
alias cd='builtin cd'
P="$0"
USAGE="`basename ${P}` [-h(elp)] [command-name] [args]"
DBG=:
OPTIONSTRING=hdni:
R='\033[0;31m' # red (use with echo -e)
G='\033[0;32m' # green
B='\033[1;34m' # blue
Y='\033[0;93m' # yellow
M='\033[0;95m' # magenta
C='\033[0;96m' # cyan
N='\033[0m'    # no color

# colourful echo
e() { (
  c=""; case $1 in R|G|B|Y|M|C|N) c=${!1}; shift;; esac; echo -e "${c}$*${N}";
) }

# message & exit if exit num present
usage() { e G "Usage: $USAGE"; [ ! -z "$1" ] && exit $1; }

# process options
while getopts $OPTIONSTRING OPTION
do
  case $OPTION in
    h)	$P build-help; echo; usage 0 ;;
    d)	DBG=echo ;;
    n)	USEXYZ="" ;;
    i)	ABC="${OPTARG}" ;;
    *)	usage 1 ;;
  esac
done
shift `expr $OPTIND - 1`
ARGS=$*

# colours example
colours() {
  echo -e \
    "white ${R}red ${G}green ${B}blue ${Y}yellow ${M}magenta ${C}cyan ${N}"
}

# common environment
# references to _VERSION commit hashes for the SDK repos are defaults;
# set-versions has others
common-env() {
  # project-specific env
  BASEDIR="$(cd `dirname ${P}` && pwd)"
  PROJECT_NAME=`basename $BASEDIR`
  FIRMWARE_VERSION=$(grep 'int firmwareVersion =' main/main.cpp \
    |sed -e 's,[^0-9]*,,' -e 's,;.*$,,' -e 's,//.*,,')
  PROJECT_VER=`echo $FIRMWARE_VERSION |sed -e 's,\(.\),\1.,g' -e 's,\.$,,'`

  # make sure virtualenv is on PATH
  PATH=$PATH:.local/bin
  PROJECT_PATH=$PWD

  # make a guess at the port
  ESPPORT=$([ ! -z "$ESPPORT" ] && echo $ESPPORT || \
    for p in ttyUSB0 ttyUSB1 cu.SLAB_USBtoUART; do \
    [ -r /dev/${p} ] && echo /dev/${p} && break; done)

  # locations
  SDKS_DIR=`dirname $IDF_PATH`

  # IDF
  ESP_IDF_GIT=https://github.com/espressif/esp-idf.git
  ESP_IDF_VERSION=357a27760
  ESP_IDF_DIR=${SDKS_DIR}/esp-idf

  # ESP arduino core
  ESP_ARD_GIT=https://github.com/espressif/arduino-esp32.git
  # littlefs bug here ESP_ARD_VERSION=esp32s2
  ESP_ARD_VERSION=41e392f66119f350cb476b4f56d1194e0b16dfa2
  ESP_ARD_DIR=${SDKS_DIR}/arduino-esp32

  # arduino core IDF builder
  ESP_LIB_BUILDER_GIT=https://github.com/espressif/esp32-arduino-lib-builder.git
  ESP_LIB_BUILDER_VERSION=release/v4.0
  ESP_LIB_BUILDER_DIR=${SDKS_DIR}/esp32-arduino-lib-builder

  # esp rainmaker
  ESP_RAINMAKER_GIT=https://github.com/espressif/esp-rainmaker.git
  ESP_RAINMAKER_VERSION=master
  ESP_RAINMAKER_DIR=${SDKS_DIR}/esp-rainmaker

  # arduino IDE dirs
  ARD_LIB_DIR=${SDKS_DIR}/Arduino/libraries
  ARD_HW_DIR=${SDKS_DIR}/Arduino/hardware/espressif

  # arduino IDE 1.9.0-beta
  ARD_IDE_VER=1.9.0-beta
  ARD_IDE_DIR=ide_beta/
  ARD_IDE_RUNNER_VER=9
  ARD_IDE_SDKS_DIR=${SDKS_DIR}/arduino-PR-beta1.9-BUILD-*

  # arduino IDE download page; linux 64 bit by default, or arm 32 bit
  ARD_IDE_FILE=arduino-${ARD_IDE_VER}-linux64.tar.xz
  case `uname -m` in
    arm*) ARD_IDE_FILE=arduino-${ARD_IDE_VER}-linuxarm.tar.xz ;;
  esac
  ARD_IDE_URL=https://downloads.arduino.cc/${ARD_IDE_DIR}${ARD_IDE_FILE}
}

# IDF environment
idf-env() {
  if [ -d local-sdks ]; then
    EXP=local-sdks/esp-idf/export.sh; e G sourcing $EXP; . $EXP || exit 1
  elif [ -d ~/esp ]; then
    EXP=~/esp/esp-idf/export.sh; e G sourcing $EXP; . $EXP || exit 1
  elif [ -d sdks ]; then
    EXP=sdks/esp-idf/export.sh; e G sourcing $EXP; . $EXP || exit 1
  elif [ -d ~/unphone/sdks ]; then
    EXP=~/unphone/sdks/esp-idf/export.sh; e G sourcing $EXP; . $EXP || exit 1
  elif [ x$IDF_PATH != x ]; then
    EXP=$IDF_PATH/export.sh; e G sourcing $EXP; . $EXP || exit 1
  else
    e R "can't find an ESP IDF... set IDF_PATH?"
    exit 1
  fi

  common-env
  [ -d local-sdks ] || ln -s $SDKS_DIR local-sdks
  e G seem to have an IDF at $IDF_PATH
}

# check out a version set of an SDK repo
do-checkout() {
  D=$1; V=$2
  if [ ! -d $D ]
  then
    e R "oops: $D not found, ignoring"
  else
    (
      cd ${D} &&
        e M "git status in ${PWD}: $(git status)" &&
        e "going to checkout ${V}, present in these branches:" &&
        ( git branch -a --contains ${V} || : ) &&
        git checkout --recurse-submodules ${V} &&
        git submodule update --init --recursive &&
        e M "git status in ${PWD}: $(git status)"
    )
  fi
}

# IDF & Arduino core install
# https://docs.espressif.com/projects/esp-idf/en/stable/get-started/linux-setup.html
idf-install() {
  IDF_PATH=~/esp/esp-idf common-env

  e B going to install to ~/esp...; sleep 3
  e B doing prereq installs
  sudo apt-get install git wget flex bison gperf python3 python3-pip \
    python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util

  e B setting up python3
  sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
  e G running python is $(python --version)

  e B ~/esp
  mkdir -p ~/esp
  set-versions

  (
    cd ~/esp
    echo "SDK set $VNAME @ `date`" >version-set-name.txt
    echo "IDFVER=$IDFVER ARDVER=$ARDVER LBVER=$LBVER RMVER=$RMVER" \
      >>version-set-name.txt

    e B "IDF"
    [ ! -d ${ESP_IDF_DIR} ] && git clone ${ESP_IDF_GIT}
    do-checkout ${ESP_IDF_DIR} ${IDFVER}
    e B ${ESP_IDF_DIR}/install.sh... && ${ESP_IDF_DIR}/install.sh
    e G "IDF install at ${PWD} should be usable\n"

    e B "Arduino core"
    [ ! -d ${ESP_ARD_DIR} ] && git clone ${ESP_ARD_GIT}
    do-checkout ${ESP_ARD_DIR} ${ARDVER}
    e G "Arduino components done\n"

    e B "Lib Builder"
    [ ! -d ${ESP_LIB_BUILDER_DIR} ] && git clone ${ESP_LIB_BUILDER_GIT}
    do-checkout ${ESP_LIB_BUILDER_DIR} ${LBVER}
    e G "Lib Builder done\n"

    e B "RainMaker"
    [ ! -d ${ESP_RAINMAKER_DIR} ] && git clone ${ESP_RAINMAKER_GIT}
    do-checkout ${ESP_RAINMAKER_DIR} ${RMVER}
    e B "doing pip installs for rainmaker CLI..."
    e B see \
"https://rainmaker.espressif.com/docs/cli-setup.html#installing-dependencies"
    pip3 install -r ${ESP_RAINMAKER_DIR}/cli/requirements.txt
    e G "RainMaker done\n"
  )

  idf-env
  e G use idf.py at $(which idf.py)
}

# choose and set variables for an SDK version set
# these sets attempt to match IDF and the Arduino core (and lib-builder
# and RainMaker if appropriate)
set-versions() {
  # name / IDF v / Arduino core v / lib-builder v / Rainmaker v
  OIFS="$IFS"; IFS='
' VERSIONS=$(cat << EOF
idf-3.3.4--core-1.0.5-rc6 cd59d107b 1.0.5-rc6 master master
nov-2020-idf4.3-dev 357a27760 41e392f66 release/v4.0 master
idf4.2--core-b4.2 release/v4.2 idf-release/v4.2 master master
idf4.2-beta1-ish 494a124d9 29e3b640 master master
oct-2020-idf4.2-dev 357a27760 6ffe081f master master
idf3.3-core-rc4 b4c0751692a18db098a9a6139b2ab5a789d39167 1.0.5-rc4 master master
head-dev master master master master
EOF
  )

  e B "choose a version set:"
  echo -n "******** "
  e R "oops, I think only number 2 (idf4.3-dev etc.) works at present!"
  HEADINGS="name idf-ver ard-ver lib-build-ver rainmaker-ver"
  I=0
  declare -A VERMAP
  echo "$HEADINGS" |xargs printf "%20s	"; echo
  for v in $VERSIONS
  do
    I=$((I+1))
    echo -n "  $I "
    echo -en "$G"
    echo $v |xargs printf "%20s	"
    echo -e "$N"
    VERMAP[$I]="$v"
  done
  IFS="$OIFS"
  echo -ne "${B}? ${M}"
  read ANS; [ -z "$ANS" ] && ANS=0
  [ $ANS -gt $I -o $ANS == 0 ] &&
    { e R "oops, answer '$ANS' not found..."; exit 1; }
  e M "ok, going to set versions to $ANS"

  VER=${VERMAP[$ANS]}
  set $VER
  VNAME=$1; IDFVER=$2; ARDVER=$3; LBVER=$4; RMVER=$5
  e G "VNAME=$VNAME IDFVER=$IDFVER ARDVER=$ARDVER LBVER=$LBVER RMVER=$RMVER"
}

# what version of IDF are we running? (returns 3 or 4)
idf-version() {
  idf-env >/dev/null
  (
    cd $IDF_PATH
    V=`git describe`
    e G "IDF version details are: $V" >&2
    case "$V" in
      v4*) echo 4;;
      v3*) echo 3;;
    esac
  )
}

# idf.py for IDF 4, or make -f idf3.mk for IDF 3
# Note: without arguments, **the default command is idf.py flash monitor** (or
# make -f idf3.mk flash monitor in IDF 3)
idf-py() {
  idf-env

  ARGS="$*"
  if [ -z "$ARGS" ]
  then
    ARGS="flash monitor"
  fi

  V=`idf-version`
  if [ $V == 4 ]
  then
    ESPBAUD=921600 idf.py $ARGS
  elif [ $V == 3 ]
  then
    ESPBAUD=921600 make -f idf3.mk $ARGS
  else
    e R "oops; `idf-version` unrecognised"
  fi
}

# TODO
# idf-env should test for version; if 3, then use this instead of idf-py:
#  PATH=~/.espressif/tools/xtensa-esp32-elf/1.22.0-97-gc752ad5-5.2.0/xtensa-esp32-elf/bin:$PATH IDF_PATH=local-sdks/esp-idf make -f idf3.mk menuconfig
# (create sdkconfig; turn on Bluetooth; ...)
# PATH=~/.espressif/tools/xtensa-esp32-elf/1.22.0-97-gc752ad5-5.2.0/xtensa-esp32-elf/bin:$PATH IDF_PATH=local-sdks/esp-idf make -f idf3.mk


# clean up build stuff
clean() { rm -rf build; e B "maybe delete sdkconfig too if not modded"; }

# IDF help
idf-help() {
  echo; e Y "IDF help is:"
  idf-env >/dev/null 2>&1
  idf.py help
}

# local help
build-help() {
  e Y "$P commands available are:"
  tac $P >/tmp/$$
  for n in `grep -n '^[a-z0-9-]*() {' /tmp/$$ |sed 's,:.*$,,' |tac`
  do
    nn=$((n+1))
    FNAME=`sed -n "${n}p" /tmp/$$ |sed 's,().*,,'`
    [ $FNAME == e -o $FNAME == usage -o $FNAME = colours ] && continue
    echo -en "$G${FNAME}: $N"
    sed -n "${nn},/^$/p" /tmp/$$ |sed 's,^#, ,' |tac
  done
}

# report the commit hashes of the IDF and ESP arduino core clones
versions() {
  idf-env >/dev/null 2>&1
  e G "FIRMWARE_VERSION=${FIRMWARE_VERSION}\nPROJ_VER=${PROJECT_VER}      "
  e G "PROJECT_PATH=${PROJECT_PATH}\nSDKS_DIR=${SDKS_DIR}                 "
  e G "PROJECT_NAME=${PROJECT_NAME}                                       "
  for d in \
    ${ESP_IDF_DIR} ${ESP_ARD_DIR} ${ESP_RAINMAKER_DIR} ${ESP_LIB_BUILDER_DIR}
  do
    echo -e $G; cd ${d} && pwd && git log --pretty=format:'%h' -n 1
    git status
  done
  echo -e $N
}

# copy-me-to; assume $1 has the title/slug...
copy-me-to() {
  BASEDIR="$(cd `dirname ${P}` && pwd)"
  MYNAME=`basename $BASEDIR`
  PROJECT=$1
  PDIR=$PROJECT
  PNAME=`basename $PDIR`

  [ -e $PROJECT ] && { e R "oops: $PROJECT exists"; exit 1; }
  e B "creating project $PROJECT ..."

  COPYCANDIDATES=`ls -A`
  mkdir -p $PDIR
  for f in $COPYCANDIDATES
  do
    grep -qs "^${f}$" .gitignore || { cp -a ${f} $PDIR && e B $f copied; }
  done
  (
    cd $PDIR
    for f in `grep -sl ${MYNAME} * */*`
    do
      e B "sed $MYNAME to $PNAME in $f"
      sed -i "s,${MYNAME},$PNAME,g" $f
    done
    e G "$PNAME created at $PDIR"
  )
}

# push the firware for OTA
push-firmware() {
  e firmware version is ${FIRMWARE_VERSION}
  cp build/${PROJECT_NAME}.bin firmware/${FIRMWARE_VERSION}.bin
  git add -v firmware/${FIRMWARE_VERSION}.bin
  echo ${FIRMWARE_VERSION} >firmware/version
  git commit -vm "added OTA firmware version ${FIRMWARE_VERSION}" \
    firmware/${FIRMWARE_VERSION}.bin firmware/version
  git push
}

# set up the arduino IDE
# (assumes that ~/.espressif/tools/xtensa-esp32-elf has the cross compiler)
setup-arduino-ide() {
  e preparing Arduino IDE dirs
  IDF_PATH=~/esp/esp-idf idf-env

  # note: we're using sneaky symlinks instead of doing get.py from tools...
  mkdir -p ${ARD_HW_DIR};
# TODO need to choose correct xtensa/gcc somehow!
  [ -r ${ARD_HW_DIR}/esp32 ] || (
    cd ${ARD_HW_DIR} && ln -s ../../../`basename ${ESP_ARD_DIR}` esp32 &&
      cd esp32/tools && ln -s \
        ~/.espressif/tools/xtensa-esp32-elf/esp-2020r3-8.4.0/xtensa-esp32-elf &&
        ln -s $IDF_PATH/components/esptool_py/esptool
  )
  [ -d ${ARD_LIB_DIR} ] || mkdir -p ${ARD_LIB_DIR}

  if [ ! -d ${ARD_IDE_SDKS_DIR} ]; then
    e "\ndownloading Arduino IDE"
    (
      cd ${SDKS_DIR} && wget ${ARD_IDE_URL} &&
        tar xJf ${ARD_IDE_FILE} && rm ${ARD_IDE_FILE}
    )
  fi
  e "Arduino IDE available"
}

# run the Arduino IDE
arduino-ide() {
  ARDARGS=$*
  echo ARDARGS are $ARDARGS

  # IDF_PATH and also PATH settings are needed
  idf-env

  # this bit was from the Makefiles
  ARD_IDE_VERNAME=`echo ${ARD_IDE_VER} |sed 's,\.,,g'`
  ARD_DOT_DIR=${SDKS_DIR}/dot-arduino15-${ARD_IDE_VERNAME}
  [ -d ${ARD_DOT_DIR} ] || (
    PREFS_TXT=${ARD_DOT_DIR}/preferences.txt
    mkdir ${ARD_DOT_DIR}
    echo "sketchbook.path=${SDKS_DIR}/Arduino" >> ${PREFS_TXT}
    echo "board=featheresp32" >> ${PREFS_TXT}
    echo "update.check=false" >> ${PREFS_TXT}
    echo "custom_DebugLevel=featheresp32_none" >> ${PREFS_TXT}
    echo "custom_FlashFreq=featheresp32_80" >> ${PREFS_TXT}
    echo "custom_UploadSpeed=featheresp32_921600" >> ${PREFS_TXT}
    echo "serial.debug_rate=115200" >> ${PREFS_TXT}
    echo "target_package=espressif" >> ${PREFS_TXT}
    echo "target_platform=esp32" >> ${PREFS_TXT}
    :
  );
  e "running Arduino IDE"
  # old style: ./bin/arduino-ide-runner.sh -${ARD_IDE_RUNNER_VER};
  set -- -${ARD_IDE_RUNNER_VER}

  # the rest used to be in arduino-runner.sh
  # TODO there's some redundancy between the common-env ARD_ vars and the
  # below

  USAGE="`basename ${P}` [-h(elp)] [-d(ebug)] [-e(rase flash)] [-S(taging)] [-H(EAD)] [-8 (1.8.9)] [-1 (1.8.1)] [-3 (1.8.3)] [-5 (1.8.5)] [-6 (1.8.6)] [-7 (1.8.7)] [-9 (1.9.x)] [args]"
  OPTIONSTRING=hdSHx:8135967

  # specific locals
  BASEDIR="$(cd `dirname ${P}` && pwd)"
  SDKS_DIR=${BASEDIR}/local-sdks
  LIB_DIR=${SDKS_DIR}/Arduino

  # specific locals
  ERASE=
  PORT='/dev/ttyUSB0'
  IDEHOME=${SDKS_DIR}
  IDEBASE=${IDEHOME}

  # arduino IDE vars
  PREFSDIR=~/.arduino15
  SPREFSDIR=${IDEBASE}/dot-arduino15-staging
  HPREFSDIR=${IDEBASE}/dot-arduino15-head
  EIGHTPREFSDIR=${IDEBASE}/dot-arduino15-188
  ONE8ONEPREFSDIR=${IDEBASE}/dot-arduino15-181
  ONE83PREFSDIR=${IDEBASE}/dot-arduino15-183
  ONE85PREFSDIR=${IDEBASE}/dot-arduino15-185
  ONE86PREFSDIR=${IDEBASE}/dot-arduino15-186
  ONE87PREFSDIR=${IDEBASE}/dot-arduino15-187
  ONE9xPREFSDIR=${IDEBASE}/dot-arduino15-190-beta
  X=
  STAGING=
  HEAD=
  EIGHT=
  ONE8ONE=
  ONE83=
  ONE85=
  ONE86=
  ONE87=
  ONE9x=
  STAGINGDIR=${IDEBASE}/arduino-1.6.5-r5
  HEADDIR=${IDEBASE}/arduino-1.6.5-r5--local-esp
  EIGHTDIR=${IDEBASE}/arduino-1.8.9
  ONE8ONEDIR=${IDEBASE}/arduino-1.8.1
  ONE83DIR=${IDEBASE}/arduino-1.8.3
  ONE85DIR=${IDEBASE}/arduino-1.8.5
  ONE86DIR=${IDEBASE}/arduino-1.8.6
  ONE87DIR=${IDEBASE}/arduino-1.8.7
  #ONE9xDIR=${IDEBASE}/arduino-PR-beta1.9-BUILD-107
  ONE9xDIR=${IDEBASE}/arduino-PR-beta1.9-BUILD-*

  # process options
  while getopts $OPTIONSTRING OPTION
  do
    case $OPTION in
      h)	usage 0 ;;
      d)	DBG=echo ;;
      e)	ERASE=yes ;;
      S)	STAGING=yes ;;
      H)	HEAD=yes ;;
      8)	EIGHT=yes ;;
      1)	ONE8ONE=yes ;;
      3)	ONE83=yes ;;
      5)	ONE85=yes ;;
      6)	ONE86=yes ;;
      7)	ONE87=yes ;;
      9)	ONE9x=yes ;;
      x)	X="${OPTARG}" ;;
      *)	usage 1 ;;
    esac
  done
  shift `expr $OPTIND - 1`

  # main action branches
  if [ x$ERASE == xyes ]
  then
    [ -d Arduino ] || { echo "can't find Arduino dir from `pwd`"; exit 10; }
    echo running erase on $PORT ...
    echo Arduino/hardware/espressif/esp32/tools/esptool.py --port "${PORT}" erase_flash
    Arduino/hardware/espressif/esp32/tools/esptool.py --port "${PORT}" erase_flash
    exit 0
  else
    # need to specify an IDE version to run
    if [ x$STAGING == x -a x$HEAD == x -a x$EIGHT == x -a x$ONE8ONE == x -a x$ONE83 == x -a x$ONE85 == x -a x$ONE86 == x -a x$ONE87 == x -a x$ONE9x == x ]
    then
      echo "you must choose staging or head or 8 or 1 or 3 or etc."
      usage 2
    fi
  fi

  # choose version
  e G preparing to run Arduino IDE...
  if [ x$STAGING == xyes ]
  then
    USEPREFS=${SPREFSDIR}
    USEIDE=${STAGINGDIR}
  elif [ x$HEAD == xyes ]
  then
    USEPREFS=${HPREFSDIR}
    USEIDE=${HEADDIR}
  elif [ x$EIGHT == xyes ]
  then
    USEPREFS=${EIGHTPREFSDIR}
    USEIDE=${EIGHTDIR}
  elif [ x$ONE8ONE == xyes ]
  then
    USEPREFS=${ONE8ONEPREFSDIR}
    USEIDE=${ONE8ONEDIR}
  elif [ x$ONE83 == xyes ]
  then
    USEPREFS=${ONE83PREFSDIR}
    USEIDE=${ONE83DIR}
  elif [ x$ONE85 == xyes ]
  then
    USEPREFS=${ONE85PREFSDIR}
    USEIDE=${ONE85DIR}
  elif [ x$ONE86 == xyes ]
  then
    USEPREFS=${ONE86PREFSDIR}
    USEIDE=${ONE86DIR}
  elif [ x$ONE87 == xyes ]
  then
    USEPREFS=${ONE87PREFSDIR}
    USEIDE=${ONE87DIR}
  elif [ x$ONE9x == xyes ]
  then
    USEPREFS=${ONE9xPREFSDIR}
    USEIDE=${ONE9xDIR}
  else
    echo 'erk!'
    usage 3
  fi

  # link the prefs dir if it doesn't exist
  if [ -d $PREFSDIR ]
  then
    mv $PREFSDIR $HOME/.saved-`basename $PREFSDIR`-$$
  fi
  if [ ! -e $PREFSDIR ]
  then
    echo linking $USEPREFS to $PREFSDIR ...
    ( cd; ln -s $USEPREFS $PREFSDIR; )
  fi

  # the prefs dir should be a link to a version-specific dir
  if [ ! -L $PREFSDIR ]
  then
    echo $PREFSDIR should be a link
    ls -l $PREFSDIR
    usage 4
  fi

  # switch the link to the required version
  rm $PREFSDIR
  ln -s $USEPREFS $PREFSDIR
  echo using $PREFSDIR: `ls -ld $PREFSDIR`

  # run the IDE
  echo running arduino IDE from `pwd`
  if [ -x `ls $USEIDE/arduino` ]
  then
    echo running arduino from `pwd`...
    echo $USEIDE/arduino $ARDARGS
    $USEIDE/arduino $ARDARGS
  else
    cd $USEIDE
    cd build
    echo doing ant run from `pwd`...
    ant run
  fi
}

# arduino build, IDF monitor
arduino-build() {
# TODO the args don't get passed through
  arduino-ide --upload sketch/sketch.ino
  make -f idf3.mk simple_monitor
}

# run a web server in the build directory for firware OTA
ota-httpd() {
  # firmware versioning
  FIRMWARE_VERSION=$(grep 'int firmwareVersion =' main/main.cpp |
    sed -e 's,[^0-9]*,,' -e 's,;.*$,,')

  # create downloads for the httpd server, .bin and version
  cd build
  mkdir -p fwserver
  cd fwserver
  echo $FIRMWARE_VERSION >version
  [ -f Thing.bin ] || cp ../Thing.bin ${FIRMWARE_VERSION}.bin

  python -m http.server 8000
}

# setup of complete IDF + arduino core
setup() {
  # install software prerequisites
  e B "installing prerequisite packages"
  sudo apt-get install git wget flex bison gperf python3 python3-pip \
    python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util

  # make python3 the default
  e B "\nmaking python3 the default"
  sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10

  # clone the ESP-IDF and Arduino core git repositories
  ESPDIR=~/esp
  # random version of 4.3 dev tree that works with this arduino core commit
  IDFVER=357a27760
  ARDCOREVER=41e392f66
  mkdir -p $ESPDIR
  cd $ESPDIR
  e B "\ncloning, checking out and initialising ESP-IDF and the arduino core"
  git clone --recursive https://github.com/espressif/esp-idf.git
  git clone --recursive https://github.com/espressif/arduino-esp32.git
  cd esp-idf
  git checkout --recurse-submodules $IDFVER
  git submodule update --init --recursive
  cd ../arduino-esp32
  git checkout --recurse-submodules $ARDCOREVER
  git submodule update --init --recursive

  # toolchain: download the Xtensa compiler etc.
  e B "\ndownloading the toolchain"
  cd $ESPDIR/esp-idf
  ./install.sh

  # set up environment variables
  e B "\nmodifying ~/.bashrc for PATH and get_idf"
  echo "PATH=$PATH:~/.local/bin; export PATH" >>~/.bashrc
  echo "alias get_idf='. $HOME/esp/esp-idf/export.sh'" >>~/.bashrc

  # now you can run get_idf to set up or refresh the SDK in any
  # terminal session; check that this is working
  e G "\ntesting IDF environment export..."
  source "$ESPDIR/esp-idf/export.sh"
  e G "\ndoes 'which idf.py' say something sensible?"
  which idf.py
  e G "if so, you should be good to go :)"
}

# do an upload and monitor with platformio core
pio-run() {
  ARGS="$*"
  if [ -z "$ARGS" ]
  then
    ARGS="-t upload -t monitor"
  fi

  COM="pio run $ARGS"
  e G $COM
  $COM
}

# run commands
if [ -z "$*" ]
then
  COMMAND=idf-py
else
  COMMAND="$*"
fi
e Y running $COMMAND ...
eval $COMMAND
exit $?
