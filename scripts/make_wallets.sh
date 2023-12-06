#!/bin/bash
WALLETS=4
for i in $(seq 1 $WALLETS);
do
    rm -r wallet_$i
    mkdir wallet_$i
    cp dsc-config.yaml wallet_$i
    cp dsc-key.yaml wallet_$i
    cd wallet_$i
    ../../bin/dsc wallet create
    cd ..
done