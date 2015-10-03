#!/bin/sh

./run-before-client.sh

./TestStressTest > TestStressTestOutput.txt
wait $!
grep 'Assertion' TestStressTestOutput.txt

greprc=$?

if [ "$greprc" -eq 0 ] ; then
    echo "Test superato correttamente!\n";
else
    if [ "$greprc" -eq 1 ] ; then
        echo "Qualcosa e\' andata storta..\n";
    fi
fi
