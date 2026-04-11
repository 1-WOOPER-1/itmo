
DISC="nvme0n1"; \
for T in none mq-deadline kyber bfq; do \
    echo $T > /sys/block/$DISC/queue/scheduler; \
    cat /sys/block/$DISC/queue/scheduler; \
    sync && /sbin/hdparm -tT /dev/$DISC $$ echo "----"; \
    sleep 15; \
done
