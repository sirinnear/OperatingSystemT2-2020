#!/bin/bash
FILE=./page_fault_counter
rm answers.txt
if [[ -f "$FILE" ]]; then
    echo "$FILE exist"
    
    for file in focus.trace sort.trace scan.trace
    do
        echo "file: $file" >> answers.txt
        for pagesize in 256 1024
        do
            echo "pagesize: $pagesize" >> answers.txt
            for algo in lru fifo rand
            do
                echo "algo: $algo" >> answers.txt
                for i in {3..20}
                do
                    echo "nframes: $i"
                    ./page_fault_counter $file $pagesize $i $algo >> answers.txt
                done
            done
        done
    done
fi
