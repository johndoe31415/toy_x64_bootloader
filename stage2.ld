ENTRY(stage2_main);
SECTIONS {
	. = 0x40000000;
	.text : {
		*(.ivt);
		*(.early);

		_text = .;
		*(.text);
		*(.text*);
		_text_end = .;
	}

	.data : {
		_data = .;
		*(.bss);
		*(.bss*);
		*(.data);
		*(.rodata*);
		*(COMMON)
		_data_end = .;
	}
	
	/DISCARD/ : {
		*(.note*);
		*(.iplt*);
		*(.igot*);
		*(.rel*);
		*(.comment);
		*(.eh_frame);
	}
}
