set -e

make -C ../../ RUN_FILE=aaaaaaaaaaaaaaaaaaaaaa PROJECT=O2L
sudo cp O2L.service /lib/systemd/system/
sudo systemctl enable O2L && sudo systemctl restart O2L
