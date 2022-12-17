if [[ ! -f $FIXREVERTER_DA_APM ]] ; then
    echo $FIXREVERTER_DA_APM 'does not exist, aborting.'
    exit
fi

ulimit -s unlimited
ulimit -a

phasar-llvm -c plugin.conf
