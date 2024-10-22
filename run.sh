make -C /root/Bela PROJECT_TYPE=cpp SHOULD_BUILD=true HAS_RUN_FILE=false PROJECT=O2L AT= $@
cp O2L.service /lib/systemd/system/
systemctl enable O2L && systemctl start O2L
