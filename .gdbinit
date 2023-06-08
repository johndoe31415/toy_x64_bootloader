define sidisas
si
disas $rip, +8
end

define dumpmem
dump binary memory memory.bin 0 0x10000
end
