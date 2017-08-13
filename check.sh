#!/bin/bash

echo -n "Generating keys .. "  
rm -f test*
ssh-keygen -b 768 -f test -q -N ""
ssh-keygen -f test.pub -e -m pkcs8 > pub.pem
mv test test.priv
mv pub.pem test.pub
echo done
echo -n "Generating data .. "
dd if=/dev/urandom of=in.bin bs=10212 count=5 > /dev/null 2>&1
echo done
md5sum in.bin
./enc in.bin out.bin test.priv
echo "Signing .. "
openssl dgst -sha512 -sign test.priv -out sign.bin out.bin
echo done
cat out.bin sign.bin > send.bin
rm -f out.bin sign.bin
echo "Sending .. "
echo done
echo "Receiving .. "
echo done
echo "Verifying .. "
fsize=`stat "-c%s" send.bin`
to_seek=$(($fsize-96))
dd if=send.bin of=sign.bin bs=1 skip=$to_seek > /dev/null 2>&1
dd if=send.bin of=out.bin bs=$to_seek count=1 > /dev/null 2>&1
openssl dgst -sha512 -verify test.pub -signature sign.bin out.bin
echo done
./dec out.bin out-dec.bin test.pub
md5sum out-dec.bin
echo -n "Comparing .. "
cmp in.bin out-dec.bin 
echo done
