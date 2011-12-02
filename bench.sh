#!/bin/bash
function sure {
    read answer
    if [[ $answer != "yes" && $answer != "y" && $answer != "n" && $answer != "no" ]]; then
        echo Invalid response
        exit 1
    fi
    if [[ $answer == "no" || $answer == "n" ]]; then
        return 0
    fi
    if [[ $answer == "yes" || $answer == "y" ]]; then
        return 1
    fi
}
function lipsum {
    if [[ $size == "" ]]; then
        echo "No test size given. Using default size of 10485760 (10M)..."
        size=10240
    fi
    TIMEFORMAT='Generated file in: %E seconds'
    time {
        echo "Generating lipsum test file of size $(($size * 1024)) ..."

        #Lorem ipsum placeholder text of exactly 1024 bytes for file creation
        echo "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent vestibulum tempor ipsum, ac lobortis purus ultricies consectetur. Duis pretium consectetur turpis, at iaculis neque aliquam in. Phasellus ultrices gravida mollis. Vestibulum nisl erat, lacinia eget tincidunt id, vestibulum et velit. Nulla varius, neque id suscipit feugiat, justo urna semper nisl, a pretium neque orci id nibh. Vivamus urna lectus, sagittis ut aliquam eget, mollis vitae quam. Quisque molestie lacus vel lectus volutpat vitae commodo sem convallis. Sed vel suscipit nisl. Sed risus erat, laoreet ut varius accumsan, egestas viverra tellus. Sed erat lacus, rhoncus dictum blandit nec, facilisis vitae nulla. Sed sit amet justo dolor, ut sodales est. Cras nec diam odio. Aliquam mattis venenatis condimentum. Cras vulputate justo ac sapien tincidunt quis venenatis ipsum tincidunt. Curabitur urna diam, venenatis ac condimentum quis, tempus ac nibh. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis posuere" > lipsumbyte

        for ((i=0;i<size;i++))
        do cat lipsumbyte >> lipsum
        done
    }
}

function copycheck {
    echo "Checking Copy..."

    if cmp -n $(du -b lipsum | awk '{ print $1 }') lipsum /dev/mmcblk0; then
        echo "OK."
    else
        echo "Bad Copy"
        exit 1
    fi
}

#Check if root
if [ $(id -u) != "0" ]; then
    echo "Root please." 1>&2
    exit 1
fi

dev=$1
if [[ $dev == "" ]]; then
    echo "No device specified"
    exit 1
fi

size=$2

echo "Use device $1? [y/n]"
if sure; then
    exit 1
fi
if [ -e lipsum ]; then
    echo "File lipsum exists, overwrite? (file will be used as test otherwise) [y/n]"
    if ! sure; then
        lipsum
    fi
else
    lipsum
fi

TIMEFORMAT='Time taken for dd and sync: %E seconds'
time {
    echo "Copying file with dd..."
    dd bs=1M if=lipsum of=$dev
    sync
}

copycheck

TIMEFORMAT='Time taken to clear SD card: %E seconds'
time {
    echo "Clearing card with dd..."
    dd bs=1M count=11 if=/dev/zero of=$dev
    sync
}

TIMEFORMAT='Time taken for sdupdate and sync: %E seconds'
time {
    echo "Copying file with sdupdate..."
    ./sdupdate -b 4M lipsum $dev
    sync
}

copycheck

echo "Benchmark completed successfully. Cleaning up Files..."
rm -f lipsumbyte
exit 0
