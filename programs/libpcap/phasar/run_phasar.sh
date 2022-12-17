if [[ ! -f $FIXREVERTER_DA_APM ]] ; then
    echo $FIXREVERTER_DA_APM 'does not exist, aborting.'
    exit
fi

if [[ ! -f "/src/psr_driver.c" ]] ; then
    echo "/src/psr_driver.c" 'does not exist, aborting.'
    exit
fi

if [[ ! -f "/src/psrFuzzingEngine.o" ]] ; then
    echo "/src/psrFuzzingEngine.o" 'does not exist, aborting.'
    exit
fi

if [[ ! -f "/src/.psrFuzzingEngine.o.bc" ]] ; then
    echo "/src/.psrFuzzingEngine.o.bc" 'does not exist, aborting.'
    exit
fi

ulimit -s unlimited
ulimit -a

/phasar/build/tools/phasar-llvm/phasar-llvm -c plugin.conf
