set -e

make -C /root/Bela RUN_FILE=aaaaaaaaaaaaaaaaaaaaaa PROJECT=O2L AT= $@
cp O2L.service /lib/systemd/system/
systemctl enable O2L && systemctl restart O2L
