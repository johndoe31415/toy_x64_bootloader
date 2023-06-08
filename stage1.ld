ENTRY(main);
SECTIONS {
	. = 0x8000;
	.text : {
		*(.ivt);
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
