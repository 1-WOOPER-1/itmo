DISK="nvme0n1"
SCHEDULERS=$(cat /sys/block/$DISK/queue/scheduler | sed 's/[][]//g')

echo "=============================================="
echo "Тестирование планировщиков для диска: $DISK"
echo "Доступные планировщики: $SCHEDULERS"
echo "=============================================="

for SCHED in $SCHEDULERS; do
    echo ""
    echo ">>> Переключение на планировщик: $SCHED"
    echo $SCHED > /sys/block/$DISK/queue/scheduler
    echo "Запуск теста FIO (Random Read/Write 70/30)..."

    OUTPUT=$(fio --name=test_sched \
        --filename=./fio_test_file \
        --size=1G \
        --ioengine=libaio \
        --direct=1 \
        --rw=randrw \
        --rwmixread=70 \
        --bs=4k \
        --iodepth=32 \
        --numjobs=1 \
        --runtime=20 \
        --time_based \
        --group_reporting \
        --output-format=json)

    READ_IOPS=$(echo "$OUTPUT" | grep -A 10 '"read" :' | grep "iops" | head -1 | awk -F': ' '{print $2}' | awk -F',' '{print $1}')
    WRITE_IOPS=$(echo "$OUTPUT" | grep -A 10 '"write" :' | grep "iops" | head -1 | awk -F': ' '{print $2}' | awk -F',' '{print $1}')

    LATENCY=$(echo "$OUTPUT" | grep -A 40 '"clat_ns" :' | grep "mean" | head -1 | awk -F': ' '{print $2}' | awk -F',' '{print $1}')
    LAT_MS=$(echo "scale=3; $LATENCY / 1000000" | bc)

    echo "----------------------------------------------"
    echo "РЕЗУЛЬТАТ ($SCHED):"
    echo "  Read IOPS:  $READ_IOPS"
    echo "  Write IOPS: $WRITE_IOPS"
    echo "  Latency:    $LAT_MS ms (средняя)"
    echo "----------------------------------------------"

    rm ./fio_test_file
    sleep 3
done
