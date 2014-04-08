# Bitgen generates mcs_0_bd.bmm if the BMM_fl is set in the NCD file.
# NGDBuild sets the BMM_fl flag with the option "-bm ipcore_dir/mcs_0.bmm"

mcs_dir=/home/kmtaylor/Xilinx/Projects/transceiver_ise
mcs_name=mcs_0
mcs_bmm_bd=$mcs_name\_bd.bmm

bitfile=toplevel.bit

elf_file=tx_framer.elf

make

data2mem -p xc5vlx50tff1136-1 -bm $mcs_dir/ipcore_dir/$mcs_bmm_bd \
		 -bd $elf_file tag $mcs_name -bt "$mcs_dir/$bitfile" \
		 -o b "toplevel_out.bit"

djtgcfg -i 0 -d Genesys prog -f toplevel_out.bit
